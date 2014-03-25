/* T A B L E I N S . C */

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
#include "fkp.h"
#include "debug.h"
#include "table.h"
#include "error.h"
#include "inter.h"

#ifdef WIN
#include "field.h" 
#include "rareflag.h"
#endif


extern char             rgchEop[];
extern char             rgchTable[];

extern struct PAP       vpapFetch;
extern struct SEP       vsepFetch;
extern struct CHP       vchpFetch;
extern struct TAP       vtapFetch;
extern CP				vmpitccp[];
extern struct CA        caPara;
extern struct CA        caPage;
extern struct CA        caSect;
extern struct CA        caTap;
extern int              vccpFetch;
extern char HUGE        *vhpchFetch;
extern struct PREF      vpref;
extern struct ITR	vitr;
#ifdef MAC
extern struct LLC	vllc;
#endif

extern struct DOD       **mpdochdod[];

extern struct SEL       selCur;
extern struct FLI       vfli;

extern struct FKPD      vfkpdPap;
extern struct FKPD      vfkpdChp;
extern struct FKPDT     vfkpdText;

extern struct WWD			**hwwdCur;
extern struct MERR		vmerr;
extern int				vdocTemp;

#ifdef WIN
extern struct RF	vrf;
#endif /* WIN */



/* C  I N S E R T  E M P T Y  T A B L E */
/* Adds cRows of cColumns each at position cpFirst and sets the
	selection to the added rows.  Returns the number of rows inserted */
/* %%Function:CInsertEmptyTable %%Owner:rosiep */
CInsertEmptyTable(doc, cpFirst, cRows, cColumns, ptap, fCleanPap, fShowPercent)
int doc;
CP cpFirst;
int cRows, cColumns;
struct TAP *ptap;
int fCleanPap, fShowPercent;
{
	int iRow = 0, iCol;
	struct CA caDel, caT;
	CP cpCur = cpFirst;
	CP cpTemp;
	struct PAP papT;
	struct CHP chpT;
	struct TAP tapT;
	char papx[cchPapxMax+1];
	int cchPapx;

	if (DocCreateTemp(doc) == docNil)
		return 0;

	if (CpMacDocEdit(vdocTemp) > cp0)
		if (!FDelete(PcaSetWholeDoc(&caT, vdocTemp)))
			return 0;

	tapT = *ptap;
	FetchCpAndPara(doc, cpFirst, fcmProps);
	chpT = vchpFetch;
	CleanTableChp(doc, &chpT);
	papT = vpapFetch;
	if (fCleanPap)
		CleanTablePap(&papT, doc, fTrue, clrNone);
	cchPapx = CbGenTablePapxFromPap(doc, &papT, &papx);

	tapT.itcMac = cColumns;
	papT.ptap = &tapT;
	if (doc == selCur.doc)
		TurnOffSel(&selCur);

	if (!FInsertNCellMarks(vdocTemp, cp0, cColumns, &chpT, &papx, cchPapx, papT.stc))
		return iRow;

	cpTemp = ccpEop * cColumns;
	if (!FInsertRowMark(PcaSet(&caT, vdocTemp, cpTemp, cpTemp),
			&chpT, &papT))
		return iRow;
	cpTemp += ccpEop;
	PcaSet(&caT, vdocTemp, cp0, cpTemp);

	for (iRow = 0 ; iRow < cRows ; iRow++)
		{
		if (fShowPercent && !(iRow & 3))
			SetLlcPercentCp(cp0, (long)cRows, (long)iRow);
		if (!FReplaceCps(PcaPoint(&caDel, doc, cpCur), &caT))
			return iRow;
		cpCur += cpTemp;
		}

	if (fShowPercent)
		{
		SetLlcPercentCp(cp0, (long)iRow, (long)iRow);
		UnprotectLlc();
		}

	DirtyOutline(doc);
	return cRows;

}


/* C  I N S E R T  N E W  R O W S */
/* Adds cRows of cColumns each at position cpFirst. 
	Returns the number of rows inserted.
	The paragraph properties are taken from the row at cpFirst */
/* %%Function:CInsertNewRows %%Owner:davidmck %%Reviewed:6/27/89 */
CInsertNewRows(doc, cpFirst, cRows, cColumns, ptap, fShowPercent)
int doc;
CP cpFirst;
int cRows, cColumns;
struct TAP *ptap;
int fShowPercent;
{
	int iRow, iCol;
	struct CA caDel, caT;
	CP dcpTemp;
	struct PAP papT;
	struct CHP chpT;
	struct TAP tapT;
	/* vmpitccp is itcMax+1 large.  Must be this
		* big to include ttp position */
	CP mpitccp[itcMax+1];
	char papx[cchPapxMax+1];
	int cchPapx;
	struct DOD *pdodTemp, *pdod;

	if (DocCreateTemp(doc) == docNil)
		return 0;

	if (!FDelete(PcaSetWholeDoc(&caT, vdocTemp)))
		return 0;

	tapT = *ptap;
	Assert(FInTableDocCp(doc, cpFirst));

	if (doc == selCur.doc)
		TurnOffSel(&selCur);

	(pdodTemp = PdodDoc(vdocTemp))->fMotherStsh = fTrue;
	pdodTemp->doc = doc;
#ifdef MAC
	pdod = PdodDoc(doc);
	if (pdod->fShort || pdod->fMotherStsh)
		pdodTemp->doc = pdod->doc;
#endif

	CpFirstTap(doc, cpFirst);
	bltb(vmpitccp, mpitccp, sizeof(CP) * (itcMax + 1));

	for (iCol = 0; iCol <= cColumns; iCol++)
		{
		FetchCpAndPara(doc, mpitccp[iCol], fcmProps);
		chpT = vchpFetch;
		CleanTableChp(doc, &chpT);
		papT = vpapFetch;
		if (iCol < cColumns)
			{
			cchPapx = CbGenTablePapxFromPap(doc, &papT, &papx);
			if (!FInsertCellMark(PcaSetDcp(&caT, vdocTemp, iCol * ccpEop, cp0), &chpT, &papx, cchPapx, papT.stc))
				return 0;
			}
		}

	dcpTemp = ccpEop * cColumns;

	tapT.itcMac = cColumns;
	papT.ptap = &tapT;

	if (!FInsertRowMark(PcaPoint(&caT, vdocTemp, dcpTemp),
			&chpT, &papT))
		return 0;
	dcpTemp += ccpEop;
	PcaSet(&caT, vdocTemp, cp0, dcpTemp);

	for (iRow = 0 ; iRow < cRows ; iRow++)
		{
#ifdef MAC
		if (fShowPercent)
			SetLlcPercentCp(cp0, (long)cRows, (long)iRow);
#endif
		if (!FReplaceCps(PcaPoint(&caDel, doc, cpFirst), &caT))
			return iRow;
		cpFirst += dcpTemp;
		}

#ifdef MAC
	if (fShowPercent)
		{
		SetLlcPercentCp(cp0, (long)iRow, (long)iRow);
		UnprotectLlc();
		}
#endif

	return cRows;

}


/* F  I N S  T A B L E  C  C E L L S */
/* Insert cCells starting at cpFirst */
/* If pchp == 0, cache to get chp */
/* %%Function:FInsTableCCells %%Owner:davidmck %%Reviewed:6/27/89 */
FInsTableCCells(doc, cpFirst, cCells, pchp, ppap)
CP cpFirst;
int doc, cCells;
struct CHP *pchp;
struct PAP *ppap;
{
	struct PAP papT;
	struct CHP chpT;
	char papx[cchPapxMax+1];
	int cchPapx;

	if (pchp == 0)
		{
		FetchCpAndPara(doc, cpFirst, fcmProps);
		chpT = vchpFetch;
		CleanTableChp(doc, &chpT);
		pchp = &chpT;
		}
	if (ppap == 0)
		{
		CachePara(doc, cpFirst);
		papT = vpapFetch;
		ppap = &papT;
		}

	ppap->fInTable = fTrue;
	ppap->fTtp = fFalse;

	cchPapx = CbGenTablePapxFromPap(doc, ppap, &papx);
	return FInsertNCellMarks(doc, cpFirst, cCells, pchp, &papx, cchPapx, ppap->stc);

}


/* T B L  F O R M A T  S C A N  S E L */
/* Scans sel and returns info as to the default format and
	the number of rows and columns */
/* %%Function:TblFormatScanSel %%Owner:rosiep */
TblFormatScanSel(psel, prgCorw)
struct SEL *psel;
struct CORW *prgCorw;   /* An array of rows and columns for each format */
{
	int fCommaLine = fFalse, fCommaPrev = fTrue;
	int cParas = 0, fTab = fFalse, fField = fFalse;
	int cRow = 0, cColTb = 0, cColCm = 0;
	CP cpCur = psel->cpFirst, cpLimPara;
	int ich, ichMac;
	int doc = psel->doc;
	int cColTbMac = 0, cColCmMac = 0;
	char ch;
	char HUGE *hpch;

	SetWords(prgCorw, 0, 8);

	if (psel->cpLim >= CpMacDoc(doc))
		{
#ifdef MAC		
		CP cpT;
#endif

		TurnOffSel(psel);
#ifdef MAC
/* MACREVIEW davidmck (rp): This change fixes your bug #4529 (also requires
	a change in tablemac.c - see our corresponding code in dlgtable.c).  You
	may want to take it for MacWord 4.x */
		cpT = CpMacDoc(doc) - ccpEop;
		if (cpT > psel->cpFirst)
			psel->cpLim = cpT;
#else
		psel->cpLim = CpMacDocEdit(doc);
		if (psel->cpFirst == psel->cpLim)
			{
			psel->sk == skIns;
			/* doesn't matter; we ignore return value anyway */
			return tblfmtPara;
			}
#endif
		}

	/* Deal with hidden paragraphs at the beginning of the selection */
	CachePara(doc, cpCur);
	cpLimPara = caPara.cpLim;
	FetchCpAndPara(doc, cpCur, fcmProps);
#ifdef WIN
	if (vchpFetch.fVanish && !PwwdWw(psel->ww)->grpfvisi.fSeeHidden &&
			!PwwdWw(psel->ww)->grpfvisi.fvisiShowAll)
#else
		if (!vpref.fSeeHidden && vchpFetch.fVanish)
#endif
			{
			CP cpFirst;

			cpFirst = cpCur + vccpFetch;

			while (fTrue)
				{
				if (cpFirst >= CpMacDoc(doc))
					{
					cpCur = cpFirst;
					break;
					}
				if (cpFirst >= cpLimPara)
					cpCur = cpLimPara;
				CachePara(doc, cpFirst);
				cpLimPara = caPara.cpLim;
				FetchCp(doc, cpFirst, fcmProps);
				if (!vchpFetch.fVanish)
					break;
				cpFirst += vccpFetch;
				}
			if (cpCur >= psel->cpLim)
				{
				Win(psel->cpLim = psel->cpFirst);
				return (WinMac(tblfmtPara, -1));
				}
			if (cpCur != psel->cpFirst)
				{
				TurnOffSel(psel);
				psel->cpFirst = cpCur;
				}
			}

	StartLongOp();
	if (cpCur >= psel->cpLim)
		fCommaPrev = fFalse;

	while (cpCur < psel->cpLim)
		{
		cRow++;
		cColTb = 1;
		cColCm = 1;
		fCommaLine = fFalse;
		CachePara(doc, cpCur);
		cpLimPara = caPara.cpLim;

#ifdef WIN
			{
			struct CA caT;
			CP cpLim;

		/* Disallow insertion of a partial field into a cell */
			AssureLegalSel(PcaSet(&caT, psel->doc, cpCur,
					cpLim = CpMin(psel->cpLim, cpLimPara)));
			if (caT.cpFirst != cpCur || caT.cpLim != cpLim)
				{
				ErrorEid(eidIllegalTextInTable,"TblFormatScanSel");
				EndLongOp(fFalse);
				return -1;
				}

		/* If there are any fields (at this point we know they
			are complete fields) in any line, we must treat
			this selection as tblfmtPara because the rules
			for how to divide it up would be too complex. */

			caT.cpFirst--;  /* to fool IfldWithinCa which adds 1 */
			if (vrf.fForcePara || IfldWithinCa(&caT) != ifldNil)
				{
				vrf.fForcePara = fTrue;
				goto LNoCommaTab;
				}
			}
#endif


		/* Only look at those characters in the selection */
		if (cpLimPara > psel->cpLim)
			cpLimPara = psel->cpLim;

		CachePara(doc, cpCur);
		FetchCp(doc, cpCur, fcmChars | fcmProps);

		while (cpCur < cpLimPara)
			{
			if (cpCur + vccpFetch > cpLimPara)
				vccpFetch = cpLimPara - cpCur;
			if (vchpFetch.fSpec && vrf.fForcePara)
				{
				goto LFetchNext;
				}
			/* Look at each of the characters in the paragraph */
			for (ich = 0, hpch = vhpchFetch;
					ich < vccpFetch && !vrf.fForcePara; ich++, hpch++)
				{
				if (vchpFetch.fSpec)
					{
					if (*hpch == chFieldBegin)
						{
						vrf.fForcePara = fTrue;
						break;
						}
					continue;
					}
				if ((ch = *hpch) == chTab)
					{
					fTab = fTrue;
					cColTb++;
					if (cColTb <= 0) /* wrap-around */
						vrf.fForcePara = fTrue;
					}
				else  if (ch == vitr.chList)
					{
					fCommaLine = fTrue;
					cColCm++;
					if (cColCm <= 0) /* wrap-around */
						vrf.fForcePara = fTrue;
					}
				else  if (ch == chSect || ch == chColumnBreak)
					{
					if (ich == 0 && cpCur == caPara.cpFirst)
						continue;
					vccpFetch = ich + 1;
					cpLimPara = cpCur + ich + 1;
					break;
					}
				else  if (ch == chCRJ)
					{
					if (ich + 1 < vccpFetch || cpCur + vccpFetch < cpLimPara)
						{
						fCommaPrev &= fCommaLine;

						/* Update the number of columns in the array */
						if (cColCmMac < cColCm)
							cColCmMac = cColCm;

						if (cColTbMac < cColTb)
							cColTbMac = cColTb;

						cRow++;
						cColTb = 1;
						cColCm = 1;
						fCommaLine = fFalse;
						}
					}
				}
LFetchNext:
			cpCur += vccpFetch;
			if (cpCur < cpLimPara)
				{
				CachePara(doc, cpCur);
				FetchCp(doc, cpCur, fcmChars | fcmProps);
				}
			}

		fCommaPrev &= fCommaLine;

		/* Update the number of columns in the array */
		if (cColCmMac < cColCm)
			cColCmMac = cColCm;


		if (cColTbMac < cColTb)
			cColTbMac = cColTb;

LNoCommaTab:
		cParas++;
		cpCur = cpLimPara;

		if (vrf.fForcePara)
			fCommaPrev = fTab = fFalse;
		}


	prgCorw->cColumns = 1;
	prgCorw->cRows = cParas;
	(prgCorw+1)->cRows = cRow;
	(prgCorw+1)->cColumns = cColTbMac;
	(prgCorw+2)->cRows = cRow;
	(prgCorw+2)->cColumns = cColCmMac;

	EndLongOp(fFalse);
	/* Done, now just return the appropriate format */
	if (fTab)
		return(tblfmtTab);
	else  if (fCommaPrev)
		return(tblfmtComma);
	else
		return(tblfmtPara);

}


/* E O P  B E F O R E  P C A */
/* Makes sure there is an Eop at the beginning of the insertion point */
/* %%Function:FEopBeforePca %%Owner:rosiep */
BOOL FEopBeforePca(pca)
struct CA *pca;
{
	struct CHP chpT;
	struct PAP papT;

	/* Check for EOP at begining of insertion */
	CachePara(pca->doc, pca->cpFirst);

	if (caPara.cpFirst != pca->cpFirst)
		{
		/* Need an EOP inserted here */
		FetchCp(pca->doc, pca->cpFirst, fcmProps);
		chpT = vchpFetch;
		CleanTableChp(pca->doc, &chpT);
		papT = vpapFetch;
		if (!FInsertRgch(pca->doc, pca->cpFirst,
				rgchEop, (int)ccpEop, &chpT, &papT))
			return fFalse;
		pca->cpFirst += ccpEop;
		pca->cpLim += ccpEop;
		}

	return fTrue;
}


/* E X T E N D  P C A  P A R A S */
/* Extends the selection to include whole paragraphs */
/* %%Function:ExtendPcaParas %%Owner:rosiep */
ExtendPcaParas(pca)
struct CA *pca;
{

	CachePara(pca->doc, pca->cpFirst);
	if (caPara.cpFirst != pca->cpFirst)
		pca->cpFirst = caPara.cpFirst;

	CachePara(pca->doc, pca->cpLim);
	if (caPara.cpFirst != pca->cpLim)
		pca->cpLim = caPara.cpLim;

}


/* F  I N S E R T  R O W  M A R K */
/* Put an end of row mark at pca */
/* The TAP is stored in ppap->ptap */
/* %%Function:FInsertRowMark %%Owner:rosiep */
FInsertRowMark(pca, pchp, ppap)
struct CA *pca;
struct CHP *pchp;
struct PAP *ppap;
{
	struct PAP papRowEnd;
	int cchPapx;
	FC fc;
	struct PAP papStd;
	char papx[cchPapxMax+1];
	struct CA caT;
	struct TAP tapStd;
	char prlStc[2];

	papRowEnd = *ppap;
	papRowEnd.fTtp = fTrue;
	papRowEnd.fInTable = fTrue;

	/* set insertion point properties */
	if (!FNewChpIns(pca->doc, pca->cpFirst, pchp, stcNil))
		return fFalse;

	/* Now write the characters to the scratch file */
	fc = FcAppendRgchToFn(fnScratch, rgchTable, (int)ccpEop);
	if (vmerr.fMemFail || vmerr.fDiskEmerg)
		return fFalse;

	/* Inserting EOL--must be last character of rgch */
	MapStc(*mpdochdod[pca->doc], papRowEnd.stc, 0, &papStd);
	cchPapx = CbGenPapxFromPap(&papx, &papRowEnd, &papStd, fFalse);
	SetWords(&tapStd, 0, cwTAP);
	cchPapx = CbAppendTapPropsToPapx(&papx, cchPapx, papRowEnd.ptap, &tapStd, cchPapxMax - cchPapx);

	if (!FAddRun(fnScratch,
			vfkpdText.fcLim,
			&papx,
			cchPapx,
			&vfkpdPap,
			fTrue /* para run */,
			fTrue /* alloc at fcMac */,
			fTrue /* plcbte must expand */, 
			fFalse /* not Word 3 format */))
		return fFalse;

	/* Finally, insert the piece into the document */
	return FReplace(pca, fnScratch, fc, (FC) ccpEop);
}


/* F  I N S E R T  C E L L  M A R K */
/* Inserts an end-of-cell mark in place of pca */
/* %%Function:FInsertCellMark %%Owner:rosiep */
FInsertCellMark(pca, pchp, ppapx, cchPapx, stcBase)
struct CA *pca;
struct CHP *pchp;
char *ppapx;
int cchPapx, stcBase;
{
	FC fc;
	char prlStc[2];

	CachePara(pca->doc, pca->cpFirst);
	if (vpapFetch.stc != stcBase)
		{
		struct CA caT;

		prlStc[0] = sprmPStc;
		prlStc[1] = (char) stcBase;
		ApplyGrpprlCa(&prlStc, 2, PcaSetDcp(&caT, pca->doc, pca->cpFirst, ccpEop));
		caPara.doc = docNil;
		}

	/* set insertion point properties */
	if (!FNewChpIns(pca->doc, pca->cpFirst, pchp, stcNil))
		return fFalse;

	/* Now write the characters to the scratch file */
	fc = FcAppendRgchToFn(fnScratch, rgchTable, (int)ccpEop);
	if (vmerr.fMemFail || vmerr.fDiskEmerg)
		return fFalse;

	if (!FAddRun(fnScratch,
			vfkpdText.fcLim,
			ppapx,
			cchPapx,
			&vfkpdPap,
			fTrue /* para run */,
			fTrue /* alloc at fcMac */,
			fTrue /* plcbte must expand */, 
			fFalse /* not Word 3 format */))
		return fFalse;

	caTap.doc = docNil;
	/* Finally, insert the piece into the document */
	return FReplace(pca, fnScratch, fc, (FC) ccpEop);
}


/* F  I N S E R T  N  C E L L  M A R K S */
/* Inserts several end-of-cell marks */
/* %%Function:FInsertNCellMarks %%Owner:rosiep */
FInsertNCellMarks(doc, cpFirst, cMarks, pchp, ppapx, cchPapx, stcBase)
int doc, cMarks, cchPapx, stcBase;
CP cpFirst;
struct CHP *pchp;
char *ppapx;
{
	FC fc, fcT;
	struct CA caT;
	int iMark;

	char prlStc[2];

	CachePara(doc, cpFirst);
	if (vpapFetch.stc != stcBase)
		{
		struct CA caT;

		prlStc[0] = sprmPStc;
		prlStc[1] = (char) stcBase;
		ApplyGrpprlCa(&prlStc, 2, PcaSetDcp(&caT, doc, cpFirst, ccpEop));
		caPara.doc = docNil;
		}

	/* set insertion point properties */
	if (!FNewChpIns(doc, cpFirst, pchp, stcNil))
		return fFalse;

	/* We choose not to save space in the previously islanded
		text page so following FcAppend... calls write text
		contiguously */

	if ((cbSector - vfkpdText.bFreeFirst) < cMarks * cchEop)
		vfkpdText.pn = pnNil;

	for (iMark = 0 ; iMark < cMarks ; iMark++)
		{
		/* Now write the characters to the scratch file */
		fcT = FcAppendRgchToFn(fnScratch, rgchTable, (int)ccpEop);
		if (vmerr.fMemFail || vmerr.fDiskEmerg)
			return fFalse;
		if (iMark == 0)
			fc = fcT;

		if (!FAddRun(fnScratch,
				vfkpdText.fcLim,
				ppapx,
				cchPapx,
				&vfkpdPap,
				fTrue /* para run */,
				fTrue /* alloc at fcMac */,
				fTrue /* plcbte must expand */, 
				fFalse /* not Word 3 format */))
			return fFalse;
		}

	/* Do this for smart adjustments */
	caTap.doc = docNil;
	/* Finally, insert the piece into the document */
	return FReplace(PcaPoint(&caT, doc, cpFirst), fnScratch, fc, 
			(FC) (ccpEop * cMarks));
}


/* F  F O R M A T  T A B L E  P C A */
/* Format a selection as a table following the requested formatting
* instructions */
/* %%Function:FFormatTablePca %%Owner:rosiep */
FFormatTablePca(pca, pcaUndo, tblfmt, cRows, cColumns, ptap)
struct CA *pca, *pcaUndo;
int tblfmt, cRows, cColumns;
struct TAP *ptap;
{
	CP cpCur, ccpDel, ccpInsTotal, cpOffset;
	CP cpLim, cpLimPara;
	int iRow = 0, iColumn = 0;
	int itbd;
	struct CA caT, caT2;
	char chLook;
	char HUGE *hpch;
	int ich, doc = pca->doc, ichMac;
	int docTemp;
	struct PAP papT;
	struct CHP chpT;
	struct TAP tapT;
	int brcTop, brcBottom, brcLeft, brcRight;
	int itc;
	int fDecTab;
	struct TBD tbd;
	int dxaColumnWidth;
	char ch;
	char papx[cchPapxMax+1];
	int cchPapx;
	int fComplete = fFalse;
	int brcMask = brcLine1 | brcLine2 | brcSpace2;
	BOOL fPageBreak;
	BOOL fCRJ = fTrue;
	struct DOD *pdodTemp, *pdod;


	if ((docTemp = DocCreate(fnNil)) == docNil)
		return fFalse;

	StartLongOp();
	tapT = *ptap;
	dxaColumnWidth = tapT.rgdxaCenter[1] - tapT.rgdxaCenter[0];
	tapT.itcMac = cColumns;
	cpOffset = pcaUndo->cpLim - pca->cpLim;

	/* Copy all of the selection to our temporary document */
	caT = *pca;
	/* So we get the right styles */
	(pdodTemp = PdodDoc(docTemp))->fMotherStsh = fTrue;
	pdodTemp->doc = doc;
#ifdef MAC
	pdod = PdodDoc(doc);
	if (pdod->fShort || pdod->fMotherStsh)
		pdodTemp->doc = pdod->doc;
#endif

	SetWholeDoc(docTemp, &caT);
	CachePara(pca->doc, pca->cpLim);
	if (caPara.cpFirst != pca->cpLim)
		{
		/* Include style paragraph */
		FReplaceCps(PcaPoint(&caT, docTemp, CpMacDocEdit(docTemp)),
				PcaSetDcp(&caT2, pca->doc, caPara.cpLim - ccpEop, ccpEop));
		}

	if (vmerr.fMemFail)
		goto LDisposeTemp;

	cpCur = cp0;
	cpLim = pca->cpLim - pca->cpFirst;
	chLook = chTab;
	if (tblfmt == tblfmtComma)
		chLook = vitr.chList;

	for ( ;; )
		{
		SetLlcPercentCp(cp0, cpLim, cpCur);
#ifdef MAC
		if (!(iRow & 7) && FCancel())
			goto LDisposeTemp;
#endif
LRestart:
		CachePara(docTemp, cpCur);
		if (vpapFetch.fInTable)
			{
			ErrorEid(eidInvalidSelection, "tableins");
			goto LDisposeTemp;
			}
		cpLimPara = caPara.cpLim;
		itbd = -1;
		switch (tblfmt)
			{
		case tblfmtPara:
			while (cpCur < cpLimPara)
				{
				FetchCp(docTemp, cpCur, fcmChars | fcmProps);
				fPageBreak = fFalse;
				if (cpCur + vccpFetch > cpLimPara)
					vccpFetch = cpLimPara - cpCur;
				for (ich = 0, hpch = vhpchFetch ; ich < vccpFetch ; ich++, hpch++)
					{
					/* The chSect will be a page break */
					if (*hpch == chSect || *hpch == chColumnBreak && !vchpFetch.fSpec)
						{
						if (ich == 0 && cpCur == caPara.cpFirst
								Win (|| !FLegalSel(docTemp, cpCur, cpLimPara))
								)
							{
							if (!FDelete(PcaSet(&caT, docTemp, cpCur, cpCur + ccpSect)))
								goto LDisposeTemp;
							cpLim--;
							goto LRestart;
							}
						else
							{
							cpLimPara = cpCur + ich + ccpSect;
							vccpFetch = ich + ccpSect;
							fPageBreak = fTrue;
							break;
							}
						}
					}
				cpCur += vccpFetch;
				}
			FetchCpAndPara(docTemp, CpMax(CpMin(cpLim, cpLimPara) - 1, cp0), fcmProps);
			chpT = vchpFetch;
			CleanTableChp(pca->doc, &chpT);
			papT = vpapFetch;
			CleanTablePap(&papT, docTemp, fFalse, clrNone);
			cchPapx = CbGenTablePapxFromPap(docTemp, &papT, &papx);
			ccpDel = fPageBreak ? ccpSect : ccpEop;
			/* Try and deal with 1 character EOPs */
			if (!fPageBreak)
				{
				if (FOneCharEop(docTemp, cpLimPara))
					{
					Assert(ccpEop == 2);
					ccpDel = 1;
					}
				else
					ccpDel = ccpEop;
				}
			else
				ccpDel = ccpSect;

			if (cpLimPara > cpLim)
				{
				cpLimPara = cpLim;
				ccpDel = 0;
				}
			PcaSet(&caT, docTemp,
					cpLimPara - ccpDel,
					cpLimPara);
			if (!FInsertCellMark(&caT, &chpT, &papx, cchPapx, vpapFetch.stc))
				goto LDisposeTemp;

			ccpDel -= ccpEop;
			cpLimPara -= ccpDel;
			cpLim -= ccpDel;
			iColumn++;
			if (iColumn >= cColumns)
				{
				FetchCpAndPara(docTemp, cpCur - 1, fcmProps);
				chpT = vchpFetch;
				CleanTableChp(pca->doc, &chpT);
				papT = vpapFetch;
				CleanTablePap(&papT, docTemp, fFalse, clrNone);
				papT.ptap = &tapT;
				if (!FInsertRowMark(PcaPoint(&caT, docTemp,
						cpLimPara),
						&chpT, &papT))
					goto LDisposeTemp;

				iColumn = 0;
				iRow++;
				cpLimPara += ccpEop;
				cpLim += ccpEop;
				}
			cpCur = cpLimPara;
			break;
		case tblfmtTab:
		case tblfmtComma:
			brcLeft = (vpapFetch.brcBar == 0 ? vpapFetch.brcLeft :
					vpapFetch.brcBar) & brcMask;
			brcRight = vpapFetch.brcRight & brcMask;
			brcTop = (iRow == 0 ? vpapFetch.brcTop : vpapFetch.brcBetween) & brcMask;
			brcBottom = (iRow == (cRows - 1) ? vpapFetch.brcBottom :
					vpapFetch.brcBetween) & brcMask;
			papT = vpapFetch;
			CleanTablePap(&papT, docTemp, fTrue, clrStyle);
			Assert(papT.brcp == 0 && papT.brcl == 0);
			papT.itbdMac = 0;
			cchPapx = CbGenTablePapxFromPap(docTemp, &papT, &papx);
			iColumn = 0;
			CachePara(docTemp, cpCur);
			while (cpCur < cpLimPara && cpCur < cpLim)
				{
				ccpInsTotal = 0;
				FetchCpAndPara(docTemp, cpCur, fcmChars | fcmProps);
				if (vchpFetch.fSpec)
					{
					cpCur += vccpFetch;
					continue;
					}
				fPageBreak = fFalse;
				fCRJ = fFalse;
				ichMac = vccpFetch;
				if (ichMac + cpCur > cpLim)
					ichMac = cpLim - cpCur;
				if (ichMac + cpCur > cpLimPara)
					ichMac = cpLimPara - cpCur;
				for (ich = 0, hpch = vhpchFetch ; ich < ichMac ; ich++, cpCur++, hpch++)
					{

					if ((ch = *hpch) == chCRJ)
						{
						cpLimPara = cpCur + ccpCRJ;
						cpCur = cpLimPara;
						fCRJ = fTrue;
						break;
						}
					else  if (ch == chSect || ch == chColumnBreak)
						{
						/* The chSect should be a page break */
						if (ich == 0 && cpCur == caPara.cpFirst)
							{
							if (!FDelete(PcaSet(&caT, docTemp, cpCur, cpCur + ccpSect)))
								goto LDisposeTemp;
							cpLim--;
							goto LRestart;
							}
						else
							{
							cpLimPara = cpCur + ccpSect;
							cpCur = cpLimPara;
							ichMac = ich;
							fPageBreak = fTrue;
							break;
							}
						}
					if (ch == chLook)
						{
						FetchCpAndPara(docTemp, cpCur, fcmProps);
						chpT = vchpFetch;
						CleanTableChp(pca->doc, &chpT);
						ccpDel = 1;
						/* If it is a comma, delete
							the next character if it is
							a space */
						/* Check + 2 for ichMac because we don't
							want to upset the final character
							in the line which looks like a space
							but really is a para mark */
						if (chLook == vitr.chList && ich + 1 < ichMac &&
								*(hpch+1) == chSpace)
							ccpDel++;
						fDecTab = fFalse;
						if (tblfmt == tblfmtTab)
							{
							while (itbd >= 0 && itbd < vpapFetch.itbdMac)
								{
								/* check to see if tab is decimal tab */
								bltb(&(vpapFetch.rgtbd[itbd]), &tbd, 1);
								if (tbd.jc == jcBar)
									{
									itbd++;
									continue;
									}
								if (tbd.jc == jcDecimal)
									fDecTab = fTrue;
								break;
								}
							}
						if (fDecTab && papT.itbdMac != 1)
							{
							papT.itbdMac = 1;
							*(char *)&tbd = 0;
							tbd.jc = jcDecimal;
							bltb(&tbd, &(papT.rgtbd[0]), 1);
							papT.rgdxaTab[0] = dxaColumnWidth/2;
							cchPapx = CbGenTablePapxFromPap(docTemp, &papT, &papx);
							}
						else  if (!fDecTab && papT.itbdMac != 0)
							{
							papT.itbdMac = 0;
							cchPapx = CbGenTablePapxFromPap(docTemp, &papT, &papx);
							}
						PcaSet(&caT, docTemp,
								cpCur, cpCur + ccpDel);
						/* Insert Cell mark */
						if (!FInsertCellMark(&caT, &chpT, &papx, cchPapx, papT.stc))
							goto LDisposeTemp;
						itbd++;

						if (ccpEop != ccpDel)
							{
							ccpDel -= ccpEop;
							ccpInsTotal -= ccpDel;
							}
						iColumn++;
						cpCur += ccpEop;
						break;
						}
					}
				if (iColumn >= cColumns)
					{
					papT.ptap = &tapT;
					for (itc = 0 ; itc < cColumns ; itc++)
						{
						tapT.rgtc[itc].brcTop = brcTop;
						tapT.rgtc[itc].brcBottom = brcBottom;
						}
					tapT.rgtc[0].brcLeft = brcLeft;
					tapT.rgtc[cColumns-1].brcRight = brcRight;

					if (!FInsertRowMark(PcaPoint(&caT, docTemp, cpCur) , &chpT, &papT))
						goto LDisposeTemp;
					iColumn = 0;
					iRow++;
					cpCur += ccpEop;
					ccpInsTotal += ccpEop;
					}
				cpLimPara += ccpInsTotal;
				cpLim += ccpInsTotal;
				ccpInsTotal = 0;
				}
			cpCur = CpMin(cpLimPara, cpLim);
			/* We have dealt with all the characters */
			/* Do we need to set up empty columns? */
			if (cpLim >= cpLimPara)
				{
				CP ccpT;

				if (!fPageBreak)
					{
					if (!fCRJ)
						{
						if (FOneCharEop(docTemp, cpCur))
							ccpT = 1;
						else
							ccpT = ccpEop;
						}
					else
						ccpT = ccpCRJ;
					}
				else
					ccpT = ccpSect;

				PcaSet(&caT, docTemp, CpMax(cp0, cpCur - ccpT), cpCur);
				cpCur -= ccpT;
				ccpInsTotal -= ccpT;
				}
			else
				PcaPoint(&caT, docTemp, cpCur);
			FetchCpAndPara(docTemp, caT.cpFirst, fcmProps);
			chpT = vchpFetch;
			CleanTableChp(pca->doc, &chpT);
			fDecTab = fFalse;
			if (tblfmt == tblfmtTab)
				{
				while (itbd >= 0 && itbd < vpapFetch.itbdMac)
					{
					/* check to see if tab is decimal tab */
					bltb(&(vpapFetch.rgtbd[itbd]), &tbd, 1);
					if (tbd.jc == jcBar)
						{
						itbd++;
						continue;
						}
					if (tbd.jc == jcDecimal)
						fDecTab = fTrue;
					break;
					}
				}
			if (fDecTab && papT.itbdMac != 1)
				{
				papT.itbdMac = 1;
				*(char *)&tbd = 0;
				tbd.jc = jcDecimal;
				bltb(&tbd, &(papT.rgtbd[0]), 1);
				papT.rgdxaTab[0] = dxaColumnWidth/2;
				cchPapx = CbGenTablePapxFromPap(docTemp, &papT, &papx);
				}
			else  if (!fDecTab && papT.itbdMac != 0)
				{
				papT.itbdMac = 0;
				cchPapx = CbGenTablePapxFromPap(docTemp, &papT, &papx);
				}
			for ( ; iColumn < cColumns ; iColumn++)
				{
				/* Insert Empty cells as needed */
				if (!FInsertCellMark(&caT, &chpT, &papx, cchPapx, papT.stc))
					goto LDisposeTemp;

				cpCur += ccpEop;
				ccpInsTotal += ccpEop;
				PcaPoint(&caT, docTemp, cpCur);
				}

			papT.ptap = &tapT;
			for (itc = 0 ; itc < cColumns ; itc++)
				{
				tapT.rgtc[itc].brcTop = brcTop;
				tapT.rgtc[itc].brcBottom = brcBottom;
				}
			tapT.rgtc[0].brcLeft = brcLeft;
			tapT.rgtc[cColumns-1].brcRight = brcRight;

			/* Use the ca that was created above */
			if (!FInsertRowMark(PcaPoint(&caT, docTemp, cpCur) , &chpT, &papT))
				goto LDisposeTemp;

			iRow++;
			cpCur += ccpEop;
			ccpInsTotal += ccpEop;
			cpLimPara += ccpInsTotal;
			cpLim += ccpInsTotal;
			break;
			}
		if (cpLimPara >= cpLim)  /* We're done */
			break;
		}

	cpCur = cpLim;
	if (iRow < cRows && tblfmt == tblfmtPara)
		{
		/* Add extra rows and columns if needed */
		FetchCpAndPara(docTemp, cpCur - 1, fcmProps);
		chpT = vchpFetch;
		CleanTableChp(pca->doc, &chpT);
		papT = vpapFetch;
		CleanTablePap(&papT, docTemp, fTrue, tblfmt == tblfmtPara ? clrNone : clrStyle);
		cchPapx = CbGenTablePapxFromPap(docTemp, &papT, &papx);
		for ( ; iRow < cRows ; iRow++)
			{
			for ( ; iColumn < cColumns ; ++iColumn)
				{
				/* Insert Section mark */
				if (!FInsertCellMark(PcaPoint(&caT, docTemp, cpCur),
						&chpT, &papx, cchPapx, papT.stc))
					goto LDisposeTemp;

				cpCur += ccpEop;
				}
			papT.ptap = &tapT;
			if (!FInsertRowMark(PcaPoint(&caT, docTemp,
					cpCur),
					&chpT, &papT))
				goto LDisposeTemp;

			cpCur += ccpEop;
			iColumn = 0;
			if (tblfmt != tblfmtPara)
				break;
			}
		}

	SetLlcPercentCp(cp0, 1L, 1L);

	if (!FReplaceCps(pca, PcaSet(&caT, docTemp, cp0, cpCur)))
		goto LDisposeTemp;
	cpCur += pca->cpFirst;
	pcaUndo->cpLim = cpCur + cpOffset;

	if (selCur.doc == pca->doc)
		SelectRow(&selCur, pca->cpFirst, cpCur);

	fComplete = fTrue;

LDisposeTemp:
	UnprotectLlc();
	DisposeDoc(docTemp);
	EndLongOp(fFalse);
	if (selCur.doc == pca->doc)
		DirtyOutline(selCur.doc);
	return fComplete;

}


/* F  T A B L E  T O  T E X T  S E L  C U R */
/* Converts the current selection from a table into straight
	text based on the tblfmt */
/* %%Function:FTableToTextSelCur %%Owner:rosiep */
FTableToTextSelCur(tblfmt)
int tblfmt;
{
	CP cpCur, ccpDel, cpEnd;
	int doc = selCur.doc;
	char *prgch;
	int cch;
	struct CA caT, caSel;
	struct PAP *ppap = 0, papT, papRow;
	struct CHP chpT;
	char rgchTab = chTab;
	char rgchComma[2];
	int docTemp;
	int fComplete = fFalse;
	int iParas = 0;
	struct DOD *pdodTemp, *pdod;
	CHAR ch;

	if ((docTemp = DocCreate(fnNil)) == docNil)
		return fFalse;
	StartLongOp();
	TurnOffSel(&selCur);
	(pdodTemp = PdodDoc(docTemp))->fMotherStsh = fTrue;
	pdodTemp->doc = doc;
#ifdef MAC
	pdod = PdodDoc(doc);
	if (pdod->fShort || pdod->fMotherStsh)
		pdodTemp->doc = pdod->doc;
#endif

	/* Determine character and size that will be inserted */
	switch (tblfmt)
		{
	case tblfmtPara:
		prgch = &rgchEop;
		cch = cchEop;
		papRow = vpapFetch;
		papRow.fInTable = fFalse;
		ppap = &papRow;
		break;
	case tblfmtComma:
		rgchComma[0] = vitr.chList;
		rgchComma[1] = ' ';
		prgch = &rgchComma;
		cch = 2;
		break;
	case tblfmtTab:
		prgch = &rgchTab;
		cch = 1;
		break;
		}

	/* Working backwards is easier than working forwards */
	cpEnd = selCur.cpLim - selCur.cpFirst;
	caSel = selCur.ca;
	cpCur = selCur.cpLim - selCur.cpFirst - 1;
	SetWholeDoc(docTemp, &caSel);
	SetLlcPercentCp(cp0, cpEnd, cp0);
	if (vmerr.fMemFail)
		goto LDisposeDoc;

	while (cpCur >= cp0)
		{
		CachePara(docTemp, cpCur);
#ifdef MAC
		if (!(iParas & 7) && FCancel())
			goto LDisposeDoc;
#endif
		if (!(iParas & 3))
			SetLlcPercentCp(cp0, cpEnd, cpEnd-cpCur);
		cpCur = caPara.cpFirst - 1;
		Assert(caPara.cpLim >= 1);

		/* Yes, that is a -1! */
		if ((ch = ChFetch(docTemp, caPara.cpLim-1, fcmChars)) != chTable)
			{
			ApplyTableProps(PcaSet(&caT, caPara.doc,
					caPara.cpLim - (ch == chSect ? ccpSect : ccpEop),
					caPara.cpLim), fFalse);
			if (vmerr.fMemFail || vmerr.fDiskEmerg)
				goto LDisposeDoc;
			InvalCp1(&caT);
			continue;
			}

		if (vpapFetch.fTtp)
			{
			/* End of row.  Insert an EOP */
			/* This has to be done because the ttp has no properties */
			FetchCpAndPara(docTemp, caPara.cpFirst - 1, fcmProps);
			chpT = vchpFetch;
			papT = vpapFetch;
			CachePara(docTemp, caPara.cpLim);
			if (!FDelete(PcaSet(&caT, docTemp, caPara.cpLim - (ccpDel = (tblfmt == tblfmtPara ? 1 : 2 ) * ccpEop),
					caPara.cpLim)))
				goto LDisposeDoc;
			papT.fTtp = fFalse;
			papT.fInTable = fFalse;
			if (tblfmt != tblfmtPara)
				{
				if (!FInsertRgch(docTemp, caT.cpFirst, &rgchEop,
						cchEop, &chpT, &papT))
					goto LDisposeDoc;
				cpEnd -= ccpDel - ccpEop;
				/* Skip back over the next section break since
					we have deleted two of them */
				CachePara(docTemp, caT.cpFirst);
				cpCur = caPara.cpFirst - 1;
				}
			else
				{
				cpEnd -= ccpDel;
				cpCur = caPara.cpFirst - 1;
				}
			}
		else
			{
			/* End of cell.  Insert appropriate mark */
			FetchCp(docTemp, caPara.cpLim - ccpEop, fcmProps);
			chpT = vchpFetch;
			if (ppap != 0)
				{
				papRow = vpapFetch;
				papRow.fInTable = fFalse;
				}
			if (!FDelete(PcaSet(&caT, docTemp, caPara.cpLim-ccpEop, caPara.cpLim))
					|| !FInsertRgch(docTemp, caT.cpFirst,
					prgch, cch, &chpT, ppap))
				goto LDisposeDoc;
			cpEnd -= ccpEop - cch;
			}
		}

	if (FReplaceCps(&caSel, PcaSet(&caT, docTemp, cp0, cpEnd)))
		Select(&selCur, caSel.cpFirst, caSel.cpFirst + cpEnd);
	fComplete = fTrue;
	SetLlcPercentCp(cp0, 1L, 1L);

LDisposeDoc:
	UnprotectLlc();
	DisposeDoc(docTemp);
	EndLongOp(fFalse);
	DirtyOutline(selCur.doc);
	return fComplete;

}



/* F  C H E C K  P C A  E N D  D O C */
/* Sees if pca points to the end of the document, in which
	case we need to back up one, and insert an extra EOP */
/* %%Function:FCheckPcaEndDoc %%Owner:rosiep */
FCheckPcaEndDoc(pca)
struct CA *pca;
{
	struct PAP papT;
	struct CHP chpT;
	CP cpIns, cpT;

	if (pca->cpLim < CpMacDoc(pca->doc))
		{
		cpT = CpMax(pca->cpFirst, pca->cpLim - ccpEop);
		Assert(cpT >= cp0);
#ifdef DISABLE_DRM
		CachePara(pca->doc, cpT);
		if (pca->cpLim == caPara.cpLim || pca->cpLim == caPara.cpLim - ccpEop)
#endif
			return fTrue;
		}

#ifdef DISABLE_DRM
	if (pca->cpLim < CpMacDoc(pca->doc))
		{
		/* Not at end of paragraph */
		cpIns = pca->cpLim;
		FetchCpAndPara(pca->doc, cpIns, fcmProps);
		chpT = vchpFetch;
		CleanTableChp(pca->doc, &chpT);
		papT = vpapFetch;
		}
	else
#endif
			{
			cpIns = pca->cpLim;
			MapStc(PdodDoc(pca->doc), stcNormal, &chpT, &papT);
			}

	papT.fSideBySide = fFalse;
	if (!FInsertRgch(pca->doc, cpIns, rgchEop,
			cchEop, &chpT, &papT))
		return fFalse;

	pca->cpLim += cchEop;
	return fTrue;

}


/* D C P  I N S E R T  N E W  R O W */
/* Inserts a new table row at cp cpFirst.  Assumes the character
	before cpFirst is an end-of-row mark.

	If fUseNext then use the next paragraph style from cpFirst
	Else takes the styles from the previous row.
	Returns the count of characters inserted, or -1 if fails

	If pcaSrc is non-null, get properties from it, otherwise get
	them from the dest.  This should be non-null in the case
	where cpFirst passed in is cp0.
*/
/* %%Function:DcpInsertNewRow %%Owner:rosiep */
DcpInsertNewRow(doc, cpFirst, fUseNext, pcaSrc)
int doc, fUseNext;
CP cpFirst;
struct CA *pcaSrc;
{
	struct CA caT, caDel;
	int ctc;
	CP cpCur = cpFirst, cpTemp;
	struct CHP chpT;
	struct PAP papT;
	struct TAP tapT;
	struct DOD *pdod;
	int stcp, stc;
	struct PLESTCP *pplestcp;
	int itc;
	CP mpitccp[itcMax+1];		/* Local copy of vpitccp */
	char papx[cchPapxMax+1];
	int cchPapx;
	int docFetch;

#ifdef DEBUG
	{
		int docAssert;
		CP cpFirstAssert;

		if (pcaSrc == NULL)
			{
			/* Assert that character before cpFirst is End-Of-Row */
			docAssert = doc;
			cpFirstAssert = cpFirst;
			}
		else
			{
			/* Assert that pcaSrc->cpLim is End-Of-Row */
			docAssert = pcaSrc->doc;
			cpFirstAssert = pcaSrc->cpLim;
			}

		Assert(cpFirstAssert >= ccpEop);
		Assert(FInTableDocCp(docAssert, cpFirstAssert - 1));
		CpFirstTap(docAssert, cpFirstAssert-1);
		Assert(caTap.cpLim == cpFirstAssert);

	}
#endif

	if (DocCreateTemp(doc) == docNil)
		return cpNil;

	if (CpMacDocEdit(vdocTemp) > cp0)
		if (!FDelete(PcaSetWholeDoc(&caT, vdocTemp)))
			return cpNil;

	if (pcaSrc == NULL)
		{
		docFetch = doc;
		Assert(cpFirst >= ccpEop);
		CpFirstTap(docFetch, cpFirst - ccpEop);
		}
	else
		{
		docFetch = pcaSrc->doc;
		CpFirstTap(docFetch, pcaSrc->cpLim - ccpEop);
		}
	tapT = vtapFetch;
	ctc = tapT.itcMac;
	bltb(&vmpitccp, &mpitccp, sizeof(CP) * (itcMax + 1));

	for (itc = 0; itc < ctc; itc++)
		{
		FetchCpAndPara(docFetch, mpitccp[itc], fcmProps);
		chpT = vchpFetch;
		CleanTableChp(docFetch, &chpT);
		papT = vpapFetch;
		cchPapx = CbGenTablePapxFromPap(docFetch, &papT, &papx);
		if (!FInsertCellMark(PcaSetDcp(&caT, vdocTemp, itc * ccpEop, cp0), &chpT, &papx, cchPapx, papT.stc))
			return cpNil;
		}

	cpTemp = ctc * ccpEop;
	FetchCpAndPara(docFetch, mpitccp[ctc], fcmProps);
	chpT = vchpFetch;
	CleanTableChp(docFetch, &chpT);
	papT = vpapFetch;
	papT.ptap = &tapT;

	if (!FInsertRowMark(PcaPoint(&caT, vdocTemp, cpTemp), &chpT, &papT))
		return cpNil;
	cpTemp += ccpEop;

	if (!FReplaceCps(PcaPoint(&caDel, doc, cpCur), 
			PcaSet(&caT, vdocTemp, cp0, cpTemp)))
		return cpNil;

	return(cpTemp);

}


/* F  O N E  C H A R  E O P */
/* Returns true if doc, cp is the end of a one character eop */
/* %%Function:FOneCharEop %%Owner:rosiep */
FOneCharEop(doc, cp)
int doc;
CP	cp;
{

#ifdef DEBUG
	CachePara(doc, cp - 1);
	Assert(cp == caPara.cpLim);
#endif

	Assert(ccpEop == 2);
	if (cp < ccpEop)
		return fTrue;

	if (ChFetch(doc, cp - ccpEop, fcmChars) != rgchEop[0])
		{
		Assert(ChFetch(doc, cp-1, fcmChars) == chSect ||
			ChFetch(doc, cp-1, fcmChars) == rgchEop[1]);
		return fTrue;
		}

	Assert(ChFetch(doc, cp-1, fcmChars) == rgchEop[1]);
	return fFalse;

}


