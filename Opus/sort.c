/*****************************************************************************
**
**     COPYRIGHT (C) 1986, 1987 MICROSOFT
**
******************************************************************************
**
** Module: sort.c --- Sort Functions.
**
** Functions included:
**
** REVISIONS
**
** Date        Who         Remarks
**  7/08/88     dsb         dumped gunk to far mem with PLFs and HQs.  Added
**                          table sort specific stuff.
** 11/30/87     yxy         Gee.  It's been a year....  Removed much of
**                          PC Word 3 code.  Probably the only thing
**                          you recognize are function names....
** 11/20/86     yxy         Ported from PC Word 3, and revised
**                          for efficiency.
**
*****************************************************************************/

#define NOMINMAX
#define NOSCROLL
#define NOSHOWWINDOW
#define NOREGION
#define NOVIRTUALKEYCODES
#define NOWH
#define NORASTEROPS
#define NOGDI
#define NOMETAFILE
#define NOBITMAP
#define NOWNDCLASS
#define NOBRUSH
#define NOWINOFFSETS
#define NOWINMESSAGES
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
#define NOMB
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

#include "word.h"
DEBUGASSERTSZ        /* WIN - bogus macro for assert string */
#include "heap.h"
#include "inter.h"
#include "disp.h"
#include "props.h"
#include "border.h"
#include "ch.h"
#include "sel.h"
#include "doc.h"
#include "prm.h"
#include "format.h"
#include "debug.h"
#include "error.h"
#include "message.h"
#include "prompt.h"
#include "ourmath.h"
#include "sort.h"
#include "field.h"
#include "fltexp.h"
#include "opuscmd.h"

#include "dlbenum.h"

#include "table.h"
#include "idd.h"

#include "sdmdefs.h"
#include "sdmver.h"
#include "sdm.h"
#include "sdmtmpl.h"
#include "sdmparse.h"

#include "sort.hs"
#include "sort.sdm"


#ifdef PROTOTYPE
#include "sort.cpt"
#endif /* PROTOTYPE */

/* External variables */

extern CP           vcpFetch;
extern int          vccpFetch;
extern CHAR         rgchEOP[];
extern HWND         vhDlg;
extern ENV          *penvMathError;
extern CHAR HUGE    *vhpchFetch;
extern struct CHP   vchpFetch;
extern struct PAP   vpapFetch;
extern struct TAP   vtapFetch;
extern struct CA    caPara;
extern struct CA    caTable;
extern struct CA    caTap;
extern struct TCC   vtcc;
extern struct SEL   selCur;
extern struct MERR  vmerr;
extern struct UAB   vuab;
extern struct WWD   **hwwdCur;
extern struct FLI   vfli;
extern struct PREF  vpref;
extern struct ITR   vitr;


/* Function declarations */
SC  ScCompareNumAscIsod();
SC  ScCompareNumDesIsod();
SC  ScCmpAlphaCIAscIsod();
SC  ScCmpAlphaCIDesIsod();
SC  ScCmpAlphaAscIsod();
SC  ScCmpAlphaDesIsod();
SC  ScCmpDttmAscIsod();
SC  ScCmpDttmDesIsod();

NATIVE	BOOL FFetchSortKey(); /* DECLARATION ONLY */
NATIVE	BOOL FRefetchPfvb(); /* DECLARATION ONLY */
NATIVE	Merge(); /* DECLARATION ONLY */
#ifdef BOGUS
NATIVE	BOOL FCpNL_EOL(); /* DECLARATION ONLY */
#endif /* BOGUS */

BOOL    FParseKeyAlphaNum();
BOOL    FParseKeyNumeric();
BOOL    FParseKeyDate();

CP    CpSRNormalCpPsod();
CP    CpSRBlockCpPsod();
CP    CpSROutlineCpPsod();
CP    CpLimBlock();

#ifdef DEBUG
struct SELS selsSave;
#endif

/* Global variables */

/* Note: If the initialization of rgpfnSc[] is changed,
		macros related to (including itself) PfnScFromSot()
		must be changed in sort.h */
csconst PFN  rgpfnSc[irgpfnScMax] = 
{
	ScCmpAlphaAscIsod,
			ScCmpAlphaDesIsod,
			ScCmpAlphaCIAscIsod,
			ScCmpAlphaCIDesIsod,
			ScCompareNumAscIsod,
			ScCompareNumDesIsod,
			ScCmpDttmAscIsod,
			ScCmpDttmDesIsod
};


csconst CHAR mpsktipfnScBase[] = 
{
	0, dirgpfnScAlpha, dirgpfnScAlpha + dirgpfnScNum};


csconst PFN  rgpfnPK[irgpfnScMax] =
{
	FParseKeyAlphaNum,
			FParseKeyNumeric,
			FParseKeyDate
};


csconst PFN  rgpfnSR[ipfnSRMax] =
{
	CpSRNormalCpPsod,
			CpSRBlockCpPsod,
			CpSROutlineCpPsod
};


struct SOT sot
	= {
	fFalse,        /* sort order: descending or ascending.    */
	sktAlphaNum,   /* sort key type                           */
	iSortSepComma, /* sort separator: comma or tab            */
	1,             /* key field number                        */
	fFalse,        /* what to swap: column only or whole rec. */
	fFalse,        /* fTrue iff case sensitive.               */
	fFalse,        /* fTrue iff table sort.                   */
	fFalse,        /* fTrue iff sel in one table cell         */
	NULL,
		{
		cpNil, cpNil, docNil		}
};


struct PLF **hplfSod;      /* heap far plex of SODs            */
int isodMac;       /* total number of records in the selection */
int cFldSkip;
CHAR chSep;

NATIVE CHAR HUGE *HpInPlf(); /* DECLARATION ONLY */
struct PLF **HplfInit();


/* Utilities Sort
	Sorting works both in a normal and block selection.

-- Sort record is defined as follows:
		Normal selection:
		a paragraph
		Block selection:
		a display line or a block line
		(depending on the column only setting.)
		Outline:
		a run of paragraphes of levels lower than
		the one of the first paragraph in the run.

-- Sort key is a text in a given record separated by
	a specified number of key separators (comma or tab).
	(Only first cchMaxSKFetch - 1 (65) characters of the sort key text is
	considered.)

	Alpha numeric:
		text itself.
	Numeric:
		a floating point number resulting
		from applying "Utilities Calculate"
		to the text.
	Date:
		a dttm obtained by parsing the text
		as date/time.

-- Counting of separators to obtain the text of a sort
	key in a block selection starts from the beginning
	of a block line, not a sort record.  Also, it is
	limited to a block line.

-- For sort key separation purposes, only the first
	paragraph in a sort record is considered in the out-
	line mode.

-- Sort range is cut short, if we encounter a paragraph
	of a level higher than the one of the first record in
	the outline mode.

-- Column only sort in the outline mode is prohibited.
*/
/*  %%Function:  CmdSort  %%Owner:  bobz       */

CMD CmdSort(pcmb)
CMB * pcmb;
{
	CP cpTableFirst, cpTableLim;
	SE  se, se0;
	CP  cpFirstIns = cpNil;
	int ww;

	/* Substructure in SOT and CABSORT must match in size and meaning. */
	/* Also, refer to sort.h.                      */
	Assert(sizeof(BOOL) == sizeof(int));

	if (selCur.fBlock && !FDelCkBlockPsel(&selCur, rpkText,
			NULL, fFalse /* fReportErr */, fFalse /* fSimple */))
		{
		ErrorEid(eidInvalidSelection, "CmdSort");
		return cmdError;
		}

	/* Note: for ins pt, we select the whole doc or table before the
		dialog comes up so the user sees what is being sorted. We save the
		cpFirst and restore the sel if we cancel
	*/

	/* determine if doing special table sort */
	sot.fTableSort = sot.fInOneCell = fFalse;
	if (FInTableDocCp(selCur.doc, selCur.cpFirst))
		{
		if (selCur.fIns)
			{
			cpFirstIns = selCur.cpFirst; /* to restore sel if we cancel */
			CacheTable(selCur.doc, selCur.cpFirst);
			SelectRow(&selCur, caTable.cpFirst, caTable.cpLim);
			selCur.sty = styWholeTable;
			}
		cpTableFirst = CpTableFirst(selCur.doc, selCur.cpFirst);
		cpTableLim = CpTableLim(selCur.doc, selCur.cpFirst);
		CpFirstTap(selCur.doc, selCur.cpFirst);
		CacheTc(wwNil, selCur.doc, selCur.cpFirst, fFalse, fFalse);
		/* We do table sort iff the current selection is 
			within a single table AND spans more than one cell. If
			within a single cell,fInOneCell is true    */

		if (selCur.cpFirst >= cpTableFirst && selCur.cpLim <= cpTableLim)
			if (selCur.cpLim > vtcc.cpLim)
				sot.fTableSort = fTrue;
			else
				sot.fInOneCell = fTrue;
		}
	else  if (selCur.fIns)  /* select whole doc */
		{
		cpFirstIns = selCur.cpFirst; /* to restore sel if we cancel */
		Select(&selCur, cp0, CpMacDoc(selCur.doc));
		}
		 /* for outline, be sure whole paragraphs are selected */

	else if ((*hwwdCur)->fOutline)
		{
		cpFirstIns = selCur.cpFirst; /* to restore sel if we cancel */
		ExpandOutlineCa(&selCur.ca, fFalse);
		Select(&selCur, selCur.ca.cpFirst, selCur.ca.cpLim);
		}

	sot.ca = selCur.ca;
	sot.uFieldNum = 1;
	if (sot.fTableSort)
		{
		CpFirstTap(selCur.doc, selCur.cpFirst);
		sot.ca.cpFirst = caTap.cpFirst;
		CpFirstTap(selCur.doc, selCur.cpLim - ccpEop);
		sot.ca.cpLim = caTap.cpLim;
		}
	else  if (selCur.fBlock)
		{
		sot.ca.cpLim = CpLimBlock();
		}

	if (FCmdFillCab())
		{
		/* Copy the initial setting. */
		Assert(sot.fColumnOnly == 0 || sot.fColumnOnly == 1);
		Assert(hwwdCur != hNil);
		/* only column only if block or column sel and not outline */
		if (sot.fTableSort)
			/* turn col only off if all cols in rows selected */
			/* NOTE: only true if row mark selected as well */
			sot.fColumnOnly &= !FWholeRowsPsel(&selCur);
		else
			sot.fColumnOnly &= (selCur.fBlock && !(*hwwdCur)->fOutline);
		bltbyte(&sot, &((CABSORT *) *pcmb->hcab)->iSortOrder, cbSOTInit);
		}

	if (FCmdDialog())
		{
		char dlt [sizeof (dltSort)];

		BltDlt(dltSort, dlt);

		switch (TmcOurDoDlg(dlt, pcmb))
			{
#ifdef DEBUG
		default:
			Assert(fFalse);
			return cmdError;
#endif

		case tmcError:
			/* if we changed table ins pt to a full table sel */
			if (cpFirstIns != cpNil)
				SelectIns(&selCur, cpFirstIns);
			return cmdNoMemory;

		case tmcCancel:
			/* if we changed table ins pt to a full table sel */
			if (cpFirstIns != cpNil)
				SelectIns(&selCur, cpFirstIns);
			return cmdCancelled;

		case tmcOK:
			break;
			}
		}

	if (FCmdAction())
		{

#ifdef DEBUG
		blt(&selCur, &selsSave, cwSELS);
#endif


		/* Put the sort setting into sot. */
		bltbyte(&((CABSORT *) *pcmb->hcab)->iSortOrder, &sot, cbSOTInit);

		if ((int)sot.uFieldNum < 1) /* ninch or 0 reset to 1 */
			sot.uFieldNum = 1;

		/* only column only if block or column sel and not outline */
		if (sot.fTableSort)
			/* turn col only off if all cols in rows selected */
			/* NOTE: only true if row mark selected as well */
			sot.fColumnOnly &= !FWholeRowsPsel(&selCur);
		else
			sot.fColumnOnly &= (selCur.fBlock && !(*hwwdCur)->fOutline);

		if (sot.fTableSort)
			/* fieldnum has origin at sel start not table start.
				itcFirst may be 0x8000 if an explicit table select
				was not done, so ignore itcfirst in that case.
				Base on 0, not 1 for a table. 
			*/
			{
			sot.uFieldNum--;  /* 0 based */
			if (selCur.itcFirst > 0)
				sot.uFieldNum += selCur.itcFirst;
			}
		sot.pfnSc = PfnScFromSot(sot);
		SpecialSelModeEnd();

		StartLongOp();
		SetPromptMst(mstSort, pdcAbortCheck);
		InitAbortCheck();
		se = SeBuildSortRec();
		if (se == SENone || se == SERecIgnored)
			{
			se0 = se;
			se = SeMergeSort();
			/* warn only if nothing real went wrong */
			if (se == SENone && se0 == SERecIgnored)
				se = SERecIgnored;
			}
		TerminateAbortCheck(pdcRestoreImmed);

		if (se == SENone || se == SERecIgnored) /* otherwise we didn't do the sort */
			{
			/* if any window of this doc is in outline view, 
			hplcpad should be dirtied */
			for (ww = WwDisp(selCur.doc, wwNil, fFalse); ww != wwNil; 
				ww = WwDisp(selCur.doc, ww, fFalse))
				{
				if (PwwdWw(ww)->fOutline)
					{
					PdodDoc(selCur.doc)->fOutlineDirty = fTrue;
					break;
					}
				}
			}

		Scribble(ispLayout1, ' ');
		EndLongOp(fFalse/*fAll*/);
		switch (se)
			{
		default:
			ErrorEid(se, " FDoSort");
			return cmdError;

		case SERecIgnored:  /* just a warning */
			ErrorEid(se, " FDoSort");
			return cmdOK;

		case SENoMem:
			ErrorNoMemory (se);
			return cmdError;

		case SENone:
			return cmdOK;

		case SECancelled:
			return cmdCancelled;
			}
		}

	return cmdOK;
}


/*  %%Function:  FDlgSort  %%Owner:  bobz       */


BOOL FDlgSort(dlm, tmc, wNew, wOld, wParam)
DLM dlm;
TMC tmc;
WORD wNew, wOld, wParam;
{
	switch (dlm)
		{
	case dlmInit:
		if (sot.fTableSort)
			EnableTmc(tmcSortSep, fFalse);

		EnableTmc(tmcSortCol, 
				sot.fTableSort ?
				/* turn col only off if all cols in rows selected */
				/* NOTE: only true if row mark selected as well */
		!FWholeRowsPsel(&selCur) :
				(selCur.fBlock && !(*hwwdCur)->fOutline));
		Assert(HcabDlgCur());
		EnableTmc(tmcSortCase, 
				(((CABSORT *) *(HcabDlgCur()))->skt == sktAlphaNum));
		break;

	case dlmClick:
		if (tmc == tmcSortKeyType)
			{
			EnableTmc(tmcSortCase, 
					(ValGetTmc(tmcSortKeyType) == sktAlphaNum));
			}
		break;

	case dlmTerm:
		if (tmc != tmcOK)
			break;
		if (ValGetTmc(tmcSortFieldNum) == uNinch)
			{
			ErrorEid(eidNOTNUM, " FDlgSort");
			SetTmcTxs(tmcSortFieldNum, TxsAll());
			return (fFalse);
			}
		break;
		}

	return fTrue;
}


/*  %%Function:  SeBuildSortRec  %%Owner:  bobz       */


SE SeBuildSortRec()
{
	/* Build an array of SOD's.  This array is a mapping from a record */
	/* to its sorting information.  The sorting info. includes the     */
	/* beginning and end cp's of a key and a record and a heap string  */
	/* for the key.                            */

	struct SOD HUGE *hpsod;
	CP            cpFirstRecord, cpLimSort, cpT;
	BOOL          (*pfnPK)();
	CP            (*pfnSR)();
	CHAR          *pSRInfo;
	int           isod;
	SE            seEmerg;
	struct CA     caKey;
	struct SOD    sod;
	struct BKS    bks;
	OSRIB         osrib;
	int           cCellsReq = -1;
	CHAR          szKey[cchMaxSKFetch];
#ifdef DEBUG
	CP            cpSave;
#endif

	Scribble(ispLayout1, 'B');

	seEmerg = SENone;

	/* First create a heap array of SOD, 10 initially */
	if (FNoHeap(hplfSod = HplfInit(cbSOD, cSODInit)))
		{
		return SENoMem;
		}

	/* Initialize some global variables */
	isodMac = 0;    /* total number of records seen */

	/* Initialize invariant. */
	pfnPK = rgpfnPK[sot.skt];
	pfnSR = sot.fTableSort ? rgpfnSR[ipfnSRNormal] :
			PfnSrFromSelPwwd(selCur, (*hwwdCur));
	chSep = (sot.iSep == iSepComma) ? vitr.chList : chTab;
	cFldSkip = sot.fTableSort ? 0 : sot.uFieldNum - 1;

	/* Initialize static fetch info. */
	if (sot.fTableSort)
		goto LNothingSpecial;

	if ((*hwwdCur)->fOutline)
		{
		if (!FUpdateHplcpad(sot.ca.doc))
			{
			seEmerg = SENoMem;
			goto LEmerg;
			}
		osrib.lvl2Cur = osrib.lvl2Top = -1;
		pSRInfo = &osrib;
		}
	else  if (selCur.fBlock)
		{
		InitBlockStat(&bks);
		pSRInfo = &bks;
		}
	else
		{
LNothingSpecial:
		pSRInfo = &cCellsReq;  /* only used by column/table sort */
		}

	cpFirstRecord = sot.ca.cpFirst;
	cpLimSort = sot.ca.cpLim;
	caKey.doc = sot.ca.doc;

	if (cpFirstRecord >= cpLimSort)
		return SENoRec;   /* no records to sort */

	do
		{
		if (FQueryAbortCheck())
			{
			seEmerg = SECancelled;
			goto LEmerg;
			}
		Debug(cpSave = cpFirstRecord);
		cpFirstRecord = (*pfnSR)(cpFirstRecord, &sod, &caKey, pSRInfo);

		/* bz changed concept so that empty records will be sorted as
			empty strings - to the top, so that multi-field records
			with not enough fields will sort up. Of course this means
			that all empty paras will sort to the top, but hey, this
			is what they demanded.
		
			In case of column only, there is no text to move, so
			we are forced to ignore the record.
		*/
		if (sod.cpFirstRecord >= sod.cpLimRecord && sot.fColumnOnly)
			{
			/* Empty record. */
			seEmerg = SERecIgnored;
			continue;
			}


		Assert(cpSave < cpFirstRecord);
		if (!FLegalSel(sot.ca.doc, sod.cpFirstRecord, sod.cpLimRecord))
			{
			seEmerg = SEFldCh;
			goto LEmerg;
			}

		AssertDo(FFetchSortKey(szKey, cchMaxSKFetch, &caKey, chSep, cFldSkip));

		AssertDo((*pfnPK)((sot.skt == sktAlphaNum) ? (CHAR *) &caKey : szKey, &sod));

		if ((isod = IAppendToHplf(hplfSod, (CHAR *) &sod)) == iNil)
			{
			seEmerg = SENoMem;
			goto LEmerg;
			}
		isodMac = isod + 1;
		if (isodMac >= 0x4000)	/* Sort routine can only handle < 16384 recs */
			{
			seEmerg = SETooManyRec;
			goto LEmerg;
			}
		}
	while (cpFirstRecord < cpLimSort);

	if (isodMac < 2)   /* don't bother with 0 or 1 record */
		{
		seEmerg = SENoRec;
LEmerg:
		/* No valid records, no sort. Should rarely happen */
		Assert(hplfSod != hNil);
		FreeHplf(hplfSod);
		return seEmerg;
		}

	hpsod = HpInPlf(hplfSod, 0);
	sot.ca.cpFirst = hpsod->cpFirstRecord;
	hpsod = HpInPlf(hplfSod, isodMac - 1);
	sot.ca.cpLim = hpsod->cpLimRecord;
	Scribble(ispLayout1, ' ');
	return seEmerg;
}


/*  %%Function:  CpSRNormalCpPsod  %%Owner:  bobz       */


CP CpSRNormalCpPsod(cpFirst, psod, pca, pcCellsReq)
CP cpFirst;
struct SOD *psod;
struct CA *pca;
int *pcCellsReq;
{
	CP cpRet, cpFirstParaTable;
	struct CA caTc;

	if (sot.fTableSort)
		{
		int itcKey = sot.uFieldNum;  /* already adjusted wrt table */
		CpFirstTap(sot.ca.doc, cpFirst);
		if (sot.fColumnOnly)
			{
			if (FInvalTCSort(itcKey, pcCellsReq))
				{
				/* make key empty, record empty too */
				/* if col is empty, we have no choice but to ignore
					the record
				*/
				psod->cpFirstRecord = psod->cpLimRecord = caTap.cpLim; /* arbitrary */
				pca->cpFirst = pca->cpLim = caTap.cpLim;
				}
			else
				{
				/* selection is sod; first para in desired cell is key */
				AssertDo(PcaColumnItc(pca, &caTap, itcKey));
				CachePara(sot.ca.doc, pca->cpFirst);
				pca->cpLim = caPara.cpLim; /* first para in cell is key */

				PcaColumnItc(&caTc, &caTap, selCur.itcFirst);
				psod->cpFirstRecord = caTc.cpFirst;
				PcaColumnItc(&caTc, &caTap, selCur.itcLim-1);
				psod->cpLimRecord = caTc.cpLim;
				}
			cpRet = caTap.cpLim;
			}
		else
			{
			/* if current row doesn't contain enough cells, or just end row
				selected, make key empty */
			if (itcKey >= vtapFetch.itcMac || selCur.itcFirst == vtapFetch.itcMac)
				{
				pca->cpFirst = pca->cpLim = caTap.cpLim;
				goto LRecRow;
				}
			/* else, entire row is the sod and the desired cell is the key */
			else
				{
				PcaColumnItc(pca, &caTap, itcKey);
				CachePara(sot.ca.doc, pca->cpFirst);
				pca->cpLim = caPara.cpLim; /* first para in cell is key */
LRecRow:
				psod->cpFirstRecord = caTap.cpFirst;
				psod->cpLimRecord = caTap.cpLim;
				}
			cpRet = psod->cpLimRecord;
			}
		}
	else
		{
		CachePara(sot.ca.doc, cpFirst);
		if ( !sot.fInOneCell && FInTableDocCp(sot.ca.doc, cpFirst))
			{
			CacheTable(sot.ca.doc, cpFirst);
			CachePara(sot.ca.doc, caTable.cpFirst);
			pca->cpFirst = caTable.cpFirst;
			pca->cpLim = caPara.cpLim;
			psod->cpFirstRecord = caTable.cpFirst;
			psod->cpLimRecord = caTable.cpLim;
			}
		else
			{
			pca->cpFirst = psod->cpFirstRecord = caPara.cpFirst;
			pca->cpLim = psod->cpLimRecord = caPara.cpLim;
			}
		cpRet = psod->cpLimRecord;
		}

	return cpRet;
}


/* handle various bizarre conditions for rejecting rows in a table column sort */
/*  %%Function:  FInvalTCSort  %%Owner:  bobz       */

FInvalTCSort(itcKey, pcCellsReq)
int itcKey;
int *pcCellsReq;
{
	int cCellsReq; /* # of real cells desired by selCur */
	int itc, itcLim;

	/* key cannot be before itcFirst by definition */

	if (itcKey >= vtapFetch.itcMac  /* key past last real cell */
			|| itcKey >= selCur.itcLim)  /* key outside selection */
		return fTrue;

	cCellsReq = selCur.itcLim - selCur.itcFirst;
	if ((selCur.itcLim - 1) == vtapFetch.itcMac)
		cCellsReq--;

	Assert (cCellsReq >= 0);
	if (!cCellsReq   /* only end mark selected */
			/* more cells requested than available? */
	|| cCellsReq > vtapFetch.itcMac - selCur.itcFirst)
		return fTrue;

	/* # of requested cells must be the same as in any previous row, or
	there will be no place to put some data. The first valid cell gets
	to decide what the number is
	*/

	if (*pcCellsReq == -1)
		*pcCellsReq = cCellsReq;
	else  if (cCellsReq != *pcCellsReq)
		return fTrue;

	/* reject merged cells */
	itcLim = min(selCur.itcLim, vtapFetch.itcMac);
	for (itc = selCur.itcFirst;itc < itcLim; itc++)
		if (vtapFetch.rgtc[itc].fMerged)
			return fTrue;

	return fFalse;    /* whew. ok */

}


/*  %%Function:  CpSRBlockCpPsod  %%Owner:  bobz       */


CP CpSRBlockCpPsod(cpFirst, psod, pca, pbks)
CP cpFirst;
struct SOD *psod;
struct CA *pca;
struct BKS *pbks;
{
	BOOL fSplats;
	CP cpT, dcp, cpMin, cpMac;
	int fRet;

	dcp = 0;

	Assert(pbks != NULL);

	/* Assumes that FGetBlockLine excludes Eop, Cr and CRJ
		and expands dcp to include field end char properly. */
	fRet = FGetBlockLine(&cpFirst, &dcp, pbks);

	/* note that we store these values even if FGet... fails. */
	Assert (!fRet ? !dcp : fTrue); /* dcp == 0 if fget... failed */

	pca->cpFirst = psod->cpFirstRecord = cpFirst;
	pca->cpLim = psod->cpLimRecord = cpFirst + dcp;
	/* if !fRet, this will return the values in the sod equal, and then
		cause the loop to terminate
	*/

	if (!fRet)
		{
		return (sot.ca.cpLim);
		}

	if (!sot.fColumnOnly)
		{
		/* We now have to change cpFirstRecord and cpLimRecord
			to range over a dl. */
		CachePara(pca->doc, pca->cpFirst);
		FormatLine(selCur.ww, pca->doc, CpFormatFrom(selCur.ww, pca->doc, pca->cpFirst));
		fSplats = vfli.fSplats;
		cpMin = vfli.cpMin;
		cpMac = vfli.cpMac;
		psod->cpFirstRecord = CpFirstSty(selCur.ww, selCur.doc,
				cpMin, styChar, fFalse);
		if (fSplats)
			{
			psod->cpLimRecord = psod->cpFirstRecord;
			}
		else
			{
			psod->cpLimRecord = CpLimSty(selCur.ww, selCur.doc,
					cpMac, styChar, fTrue);

#ifdef BOGUS
			/* bz - as far as I can tell, this only comes into effect at the
				final eop, and if we do back up to cpMacDocEdit, we lose the
				para mark, and the last line is sorted with no para and gets
				confused. I am removing it until or unless I can find out why
				is was in here - hopefully for an obsolete reason. bz
			*/
			cpT = CpFirstSty(selCur.ww, selCur.doc,
					psod->cpLimRecord - 1L, styChar,
					fFalse);
			if (FCpNL_EOL(cpT))
				{
				psod->cpLimRecord = cpT;
				}
#endif /* BOGUS */

			}
		pca->cpFirst = psod->cpFirstRecord;
		pca->cpLim = psod->cpLimRecord;
		}
	return (pbks->cpFirst);
}


/*  %%Function:  CpSROutlineCpPsod  %%Owner:  bobz       */


CP CpSROutlineCpPsod(cpFirst, psod, pca, posrib)
CP cpFirst;
struct SOD *psod;
struct CA *pca;
OSRIB *posrib;
{
	int lvl2Next;
	CP cpSave, cpLimSave;

	Assert(posrib != NULL);

	cpFirst = CpSRNormalCpPsod(cpFirst, psod, pca, NULL);
	/* if beyond sort limit, set everything to limit so we bag out */
	if (psod->cpFirstRecord > sot.ca.cpLim)
		{
		psod->cpFirstRecord = pca->cpFirst = sot.ca.cpLim;
		goto LPastLim;
		}
	if (cpFirst > sot.ca.cpLim)
		{
LPastLim:
		psod->cpLimRecord = pca->cpLim = sot.ca.cpLim;
		return (sot.ca.cpLim);
		}

	cpSave = psod->cpFirstRecord;
	cpLimSave = psod->cpLimRecord;

	posrib->lvl2Cur = Lvl2FromDocCp(pca->doc, psod->cpFirstRecord);
	if (posrib->lvl2Top == -1)
		{
		/* The very first record in sel. */
		posrib->lvl2Top = posrib->lvl2Cur;
		}

	while (cpFirst < sot.ca.cpLim)
		{
		lvl2Next = Lvl2FromDocCp(pca->doc, psod->cpLimRecord);
		if (lvl2Next > posrib->lvl2Top)
			{
			posrib->lvl2Cur = lvl2Next;
			cpFirst = CpSRNormalCpPsod(cpFirst, psod, pca, NULL);
			continue;
			}
		break;
		}

	pca->cpFirst = psod->cpFirstRecord = cpSave;
	pca->cpLim = cpLimSave;
	/* If we have encountered a para at the level strictly higher than
		what we started with, stop scanning there. */
	return (posrib->lvl2Cur < posrib->lvl2Top ? sot.ca.cpLim : cpFirst);
}


/*  %%Function:  FFetchSortKey  %%Owner:  bobz       */

NATIVE BOOL FFetchSortKey(szKey, cchMax, pca, chSep, cFldSkip)
CHAR szKey[];
int cchMax;
struct CA *pca;
CHAR chSep;
int cFldSkip;
{
	int cFld, ichFetch;
	CHAR *pch, *pchLim, *pchLimNonblank;
	struct FVB fvb;
	CHAR rgch[cchMaxSKFetch];
	struct CR rgcr[ccrFebMax];


	/* bz we always return true out of this - if the key is not found
		we put out an empty string so the item will sort to the top.
		Note that *pca is not modified in this routine.
	*/

	InitFvbBufs(&fvb, rgch, ichLkAhdMax, rgcr, ccrFebMax);

	fvb.doc = pca->doc;
	fvb.cpFirst = pca->cpFirst;
	fvb.cpLim = pca->cpLim;
	Assert(pca->doc == selCur.doc);
	FetchVisibleRgch(&fvb, selCur.ww/*fvcScreen*/, fFalse, fTrue);
	ichFetch = 0;

	if (sot.fTableSort)
		goto LNoSkip;

	for (cFld = 0; cFld < cFldSkip; cFld++)
		{
		while (ichFetch < fvb.cch)
			{
LKeepScan:
			if (fvb.rgch[ichFetch++] == chSep)
				goto LDec;
			}

		if (!FRefetchPfvb(&fvb, &ichFetch))
			{
			szKey[0] = 0;
			return fTrue;  /* empty key returned if we run out */
			}
		else
			goto LKeepScan;
LDec:
		;
		}

LNoSkip:
	pch = szKey;
	pchLim = &szKey[cchMax - 1];
	pchLimNonblank = szKey;
	while (pch < pchLim)
		{
		CHAR ch;

		if (ichFetch >= fvb.cch && !FRefetchPfvb(&fvb, &ichFetch))
			break;

		if ((ch = fvb.rgch[ichFetch++]) == chSep)
			break;

		if (ch == chSpace || ch == chTab || FChNL_EOL(ch) ||
				ch == chColumnBreak || ch == chSect ||
				ch == chNonBreakSpace)
			{
			if (pchLimNonblank != szKey)
				*pch++ = ch;
			}
		else
			{
			*pch++ = ch;
			pchLimNonblank = pch;
			}
		}
	*pchLimNonblank = '\0';
	return (fTrue);
}


/*  %%Function:  FRefetchPfvb  %%Owner:  bobz       */


NATIVE BOOL FRefetchPfvb(pfvb, pich)
struct FVB *pfvb;
int *pich;
{
	if (*pich >= pfvb->cch)
		{
		if (pfvb->fOverflow) /* There's more to fetch. */
			{
			Assert(pfvb->doc == selCur.doc);
			FetchVisibleRgch(pfvb, selCur.ww/*fvcScreen*/, fFalse, fTrue);
			*pich = 0;
			}
		else
			{
			return fFalse;
			}
		}
	return fTrue;
}


/*  %%Function:  SeMergeSort  %%Owner:  bobz       */

SeMergeSort()
{
	/* Perform a two-way merge sort */
	/* Note that, because ints are used, the maximum number of records to
	   sort must be < 16384.  Otherwise the "cRecord < isodMac" test will
	   be incorrect when cRecord is doubled to 0x8000 (-32768 in an int).
	*/

	int indx, iBegin, cRecord, seRet;
	int HUGE *hpint1;
	int HUGE *hpint2;
	HQ hqrgIndex1, hqrgIndex2, hqT;
	struct CA caT;

	Assert(isodMac < 0x4000);

	Scribble(ispLayout1, 'S');

	/* Create 2 heap arrays of indices */
	hqrgIndex1 = hqrgIndex2 = hqNil;
	if ((hqrgIndex1 = HqAllocLcb((long)isodMac * sizeof(int))) == hqNil || 
			(hqrgIndex2 = HqAllocLcb((long)isodMac * sizeof(int))) == hqNil)
		{
		seRet = SENoMem;
		goto LRet;
		}

	/* Initialize the first array of indices */
	hpint1 = (int HUGE *) HpOfHq(hqrgIndex1);
		{{ /* NATIVE - SeMergeSort */
		for (indx = 0; indx < isodMac; indx++)
			{
			*(hpint1 + indx) = indx;
			}

		for (cRecord = 1; cRecord < isodMac; cRecord *= 2)
			{
			if (FQueryAbortCheck())
				{{ /* !NATIVE - SeMergeSort */
				seRet = SECancelled;
				goto LRet;
				}}

			hpint1 = (int HUGE *) HpOfHq(hqrgIndex1);
			hpint2 = (int HUGE *) HpOfHq(hqrgIndex2);
			for (iBegin = 0; iBegin < isodMac - 2*cRecord + 1; iBegin+=(2*cRecord))
				{
				Merge(hpint1, iBegin, iBegin+cRecord-1, iBegin+2*cRecord-1, hpint2);
				}

			if (FQueryAbortCheck())
				{{ /* !NATIVE - SeMergeSort */
				seRet = SECancelled;
				goto LRet;
				}}

			hpint1 = (int HUGE *) HpOfHq(hqrgIndex1);
			hpint2 = (int HUGE *) HpOfHq(hqrgIndex2);
			if (iBegin + cRecord < isodMac)
				Merge(hpint1, iBegin, iBegin + cRecord - 1, isodMac-1, hpint2);
			else
				bltbh(hpint1 + iBegin, hpint2+iBegin, (isodMac-iBegin)*sizeof(int));

		/* Interchange the roles of hq1 and hq2 */
			hqT = hqrgIndex1;
			hqrgIndex1 = hqrgIndex2;
			hqrgIndex2 = hqT;
			}
		}}

	/* At this point we have an array of ordered indices of SOD's */
	FreeHq(hqrgIndex2);
	hqrgIndex2 = hqNil;
	Assert(!vfNoInval);
	caT = selCur.ca;
	if (selCur.fBlock)
		caT.cpLim = CpLimBlock();
	InvalCp1Sub(&caT);
	DisableInval();
	seRet = SeReorder(hqrgIndex1);
	EnableInval();
	/* reinvalidate in case of shrink/grow (possible in col sort maybe bz ) */
	/* have to invalidate whole doc because we had DisableInval on and
		dependencies we missed might have caused other parts of doc to
		become invalid */
	/* NOTE: we MUST do the whole doc; FQueryAbortCheck can cause a
	   call to UpdateWw which can redraw the OLD text and erase the
	   edl.fDirty bits, thus leaving the screen incorrect */
	InvalCp1Sub(PcaSetWholeDoc(&caT, selCur.doc));

LRet:
	Scribble(ispLayout1, ' ');
	if (hqrgIndex1 != hqNil)
		FreeHq(hqrgIndex1);
	if (hqrgIndex2 != hqNil)
		FreeHq(hqrgIndex2);
	Assert(hplfSod);
	FreeHplf(hplfSod);
	return seRet;
}


/*  %%Function:  Merge  %%Owner:  bobz       */


NATIVE Merge(hpintSrc, iFirst, iLast1, iLast2, hpintDest)
int HUGE *hpintSrc;
int HUGE *hpintDest;
int iFirst, iLast1, iLast2;
{
	/* The indices at pSource from iFirst to iLast1 and from iLast1+1  */
	/* to iLast2 are ordered.  The task of this routine is to sort     */
	/* these two group of indices and put the result into pDest.       */

	int iFirst1 = iFirst;
	int iFirst2 = iLast1 + 1;
	int ich = iFirst;

	while (iFirst1 <= iLast1 && iFirst2 <= iLast2)
		{
		int indx1 = *(hpintSrc + iFirst1);
		int indx2 = *(hpintSrc + iFirst2);

		if (FScLessEqual((*sot.pfnSc)(indx1, indx2)))
			{
			*(hpintDest + ich) = indx1;
			ich++;
			iFirst1++;
			}
		else
			{
			*(hpintDest + ich) = indx2;
			ich++;
			iFirst2++;
			}
		}

	if (iFirst1 > iLast1)
		bltbh(hpintSrc+iFirst2, hpintDest+ich, (iLast2-iFirst2+1)*sizeof(int));
	else
		bltbh(hpintSrc+iFirst1, hpintDest+ich, (iLast1-iFirst1+1)*sizeof(int));
}


/*  %%Function:  SeReorder  %%Owner:  bobz       */

SeReorder(hqrgIndex)
HQ hqrgIndex;
{
	CP cpFirstSel;
	CP cpLimSel;
	CP dcpSel;
	int doc = sot.ca.doc;
	int isod, isodSwap, seRet;
	int docTmp = docNil;
	CP cpT;
	int HUGE *hpint;
	struct SOD HUGE *hpsod;
	struct CHP chpT;
	struct PAP papT;
	struct CA caSrc, caDest, caT;
	BOOL fEndOneCell = fFalse;

	/* We insert an extra EOP just before the EOP of the last paragraph
		range with the same PAP and CHP, so that we will not ReplaceCp the
		very last EOP in a doc.  After all the swapping is done, we will
		remove the first EOP at the end of a sorting range.  Apply its
		PAP and CHP to the last EOP in the range. */
	/* In order to swap the records or keys around, we need to create  */
	/* a docTemp to hold the new order of the records, then we can     */
	/* replace selCur.doc with the content of docTemp.         */

	/* assurelegalsel can expand the ca we are setting and undoing to
		include entire fields.
			Since the keys are already calculated, and the docTemp cp's
			are adjusted by cpFirstSel, the proper cp's should be sorted
	*/
	AssureLegalSel( &sot.ca ); /* may change ca if partial fields */
	cpFirstSel = sot.ca.cpFirst;
	cpLimSel = sot.ca.cpLim;
	Assert (cpLimSel >= cpFirstSel);

	dcpSel = (cpLimSel - cpFirstSel);

	if (dcpSel < ccpEop)
		return (SENoRec);  /* nothing to sort */

	Scribble(ispLayout1, 'R');

	seRet = SENone;
	docTmp = DocCreate(fnNil);
	if (docTmp == docNil)
		{
		seRet = SENoMem;
		goto LRet;
		}

	/* DocCreate always sets fFormatted. Reset it to be the same
		as the source so sorting unformatted text will not set
		fFormatted, and save will still be text.
	*/
	PdodDoc(docTmp)->fFormatted = PdodDoc(doc)->fFormatted;

	/* Now copy the selection to docTemp */
	SetWholeDoc(docTmp, &sot.ca);
	if (vmerr.fMemFail)
		{
		seRet = SENoMem;
		goto LRet;
		}

	if (!sot.fTableSort)
		{
		CachePara(doc, cpT = cpLimSel - ccpEop);
		FetchCp(doc,  cpT, fcmProps);
		bltbyte(&vchpFetch, &chpT, sizeof(struct CHP));
		bltbyte(&vpapFetch, &papT, sizeof(struct PAP));
		if (FInTableDocCp(doc, cpT))
			{
			CleanTableChp(doc, &chpT);
			CleanTablePap(&papT, doc, fTrue /* fClearBorders */, clrNone);
			}
		if (sot.fInOneCell)  /* in a table, within one cell */
			{
			/* this ia a doozy. If we are in a cell and the selection includes
				the table mark, we must avoid moving the table mark. Since text
				from within doc gets replaced into doctemp, then we must not
				include the table mark in the source. However, we do want a
				para mark where the table mark was so the text won't merge into
				the next para. Soooo....
			
				We insert an eop before the table mark in the original doc.
				We reduce the size of the selection moved into docTemp
				so the final replace will remove the extra para mark.
				The extra para mark we put in below handles the merging paras
				problem.
			*/
			if (ChFetch(doc, cpT, fcmChars) == chReturn && 
					ChFetch(doc, cpT+1, fcmChars) == chTable)
				/* don't move table mark if moving within a cell - eop added
				above will handle the para without an eop problem */
				{
				dcpSel -= ccpEop;
				if (!FInsertRgch (doc, cpT, rgchEOP, cchEop, &chpT, &papT))
					{
					seRet = SENoMem;
					goto LRet;
					}
				}
			}

		if (!FInsertRgch (docTmp, (CP) dcpSel, rgchEOP, cchEop, &chpT, &papT))
			{
			seRet = SENoMem;
			goto LRet;
			}

		}

	/* Now reorder */
	TurnOffSel(&selCur);

		{{ /* NATIVE - SeReorder */
		for (isod = (isodMac - 1); isod >= 0; isod--)
			{
			if (FQueryAbortCheck())
				{{ /* !NATIVE - SeReorder */
				seRet = SECancelled;
				goto LRet;
				}}

			if (vmerr.fMemFail)
				{{ /* !NATIVE - SeReorder */
				seRet = SENoMem;
				goto LRet;
				}}

		/* don't swap if para's already in the right spot! */
			hpint = (int HUGE *) HpOfHq(hqrgIndex);
			isodSwap = *(hpint + isod);
			if (isod == isodSwap)
				continue;

		/* Swap the records */
			hpsod = HpInPlf(hplfSod, isod);
			PcaSet( &caDest, docTmp, hpsod->cpFirstRecord - cpFirstSel, 
					hpsod->cpLimRecord - cpFirstSel );
			hpsod = HpInPlf(hplfSod, isodSwap);
			PcaSet( &caSrc, doc, hpsod->cpFirstRecord, hpsod->cpLimRecord );

			FReplaceCps( &caDest, &caSrc );
			}
		}}

	if (vmerr.fMemFail)
		{
		seRet = SENoMem;
		goto LRet;
		}

	/* Save the selection in selCur.doc for Undo before
		replacing it with docTemp */
	/* Since the replacement text length may not be equal to the
		original text length, (due to extra clrf inserted at eod)
		use uccPaste, not uccFormat. */

	if (!FSetUndoB1(imiSort, uccPaste, PcaSetDcp( &caT,
			sot.ca.doc, cpFirstSel, dcpSel )))
		{
		seRet = SECancelled;
		goto LRet;
		}

	/* at eop, we may sort away the ending para mark, as in when
		a table row goes to the end of doc, so if the docTmp stuff
		does not end in CRLF and we are replacing at eop, increase
		the size of dcpSel to include the extra para mark will inserted
		earlier - this means the doc gets bigger... The props of the end para
		mark will be those of the para that was originally at the end.
	*/

	if (!sot.fTableSort && sot.ca.cpLim > CpMacDocEdit(doc))
		{
		FetchCpAndPara(docTmp, dcpSel-ccpEop, fcmChars);
#ifdef BZ
		CommSzNum(SzShared("vhpchFetch[0] at sort end: "), (int)vhpchFetch[0]);
		CommSzNum(SzShared("vhpchFetch[1] at sort end: "), (int)vhpchFetch[1]);
#endif
		if (vhpchFetch[0] != chReturn || vhpchFetch[1] != chEop)
			dcpSel+= ccpEop;
		}
	else if (sot.fInOneCell)
		{
		/* time to party! remember that funny business above about cells?
		   you can have the end of the sorted text be a section mark,
		   and if so we backed up too much */
		/* FUTURE: if section breaks were ccpEop long, this would be unnecessary */
		CacheSect(docTmp, dcpSel - 1);
		/* this means there's a section break after what we're about to
		   insert that is NOT the end of doc section break */
		if (caSect.cpLim < CpMacDoc(docTmp) && caSect.cpLim > dcpSel)
			dcpSel = caSect.cpLim;
		}
	/* if this fails then whole operation was a nop */
	if (!FReplaceCps(&sot.ca, PcaSet(&caSrc, docTmp, cp0, dcpSel)))
		{
		seRet = SENoMem;
		goto LRet;
		}

	/* Hilite the sorted selection */
	/* For Column Only sort (the original selection was block sel),
		we leave an insertion point at the cpFirst, to avoid an unpolished
		look of the selection afterwards.
		
		We select the column for a table sort, and make an non-table block
		sel an ins pt, but other sels should be intact, and can just be
		turned on.
	
	*/

	if (sot.fTableSort)
		{
		if (sot.fColumnOnly)
			{
			/* the whole selection is moved even in column sort */
			/* restore col sort to selection */
			Assert (selCur.sk = skColumn);
			CacheTable(selCur.doc, selCur.cpFirst);
			CpFirstTap(selCur.doc, selCur.cpFirst);
			SelectColumn(&selCur, caTable.cpFirst, caTable.cpLim,
					selCur.itcFirst, selCur.itcLim);
			}
		else
			{
			SelectRow(&selCur, cpFirstSel, cpFirstSel + dcpSel);
			}
		}
	else if (sot.fColumnOnly)
		SelectIns(&selCur, cpFirstSel);
	else 
		Select(&selCur, cpFirstSel, cpFirstSel + dcpSel);


	SetUndoAfter(PcaSet(&caSrc, doc, cpFirstSel, cpFirstSel + dcpSel));

LRet:
	/* Kill docTemp */
	DisposeDoc(docTmp);
	Scribble(ispLayout1, ' ');
	return seRet;
}



/* Key comparison functions --- Since these functions are called
				QUITE often, we will have a function
	for each case to minimize the overhead in it. */

/*  %%Function:  ScCompareNumAscIsod  %%Owner:  bobz       */

SC ScCompareNumAscIsod(isod1, isod2)
int isod1, isod2;
{{ /* NATIVE - ScCompareNumAscIsod */
	SC         sc;
	NUM        numT1, numT2;
	struct SOD HUGE *hpsod1;
	struct SOD HUGE *hpsod2;
	ENV        *penvSave, env;

	Assert(wapxGT > 0 && wapxEQ == 0 && wapxLT < 0);
	if (SetJmp(&env) != 0)
		{
		PopMPJmp(penvSave);
		return scEqual;
		}
	else
		{
		hpsod1 = HpInPlf(hplfSod, isod1);
		hpsod2 = HpInPlf(hplfSod, isod2);
		numT1 = hpsod1->numKey;
		numT2 = hpsod2->numKey;
		PushMPJmpEnv(env, penvSave);
		sc = WApproxCmpPnum(&numT1, &numT2);
		PopMPJmp(penvSave);
		return sc;
		}
}}


/*  %%Function:  ScCompareNumDesIsod  %%Owner:  bobz       */

SC ScCompareNumDesIsod(isod1, isod2)
int isod1, isod2;
{{ /* NATIVE - ScCompareNumDesIsod */
	SC         sc;
	NUM        numT1, numT2;
	struct SOD HUGE *hpsod1;
	struct SOD HUGE *hpsod2;
	ENV        *penvSave, env;

	Assert(wapxGT > 0 && wapxEQ == 0 && wapxLT < 0);
	if (SetJmp(&env) != 0)
		{
		PopMPJmp(penvSave);
		return scEqual;
		}
	else
		{
		hpsod1 = HpInPlf(hplfSod, isod1);
		hpsod2 = HpInPlf(hplfSod, isod2);
		numT1 = hpsod1->numKey;
		numT2 = hpsod2->numKey;
		PushMPJmpEnv(env, penvSave);
		sc = WApproxCmpPnum(&numT2, &numT1);
		PopMPJmp(penvSave);
		return sc;
		}
}}


/*  %%Function:  ScCmpAlphaCIAscIsod  %%Owner:  bobz       */

SC ScCmpAlphaCIAscIsod(isod1, isod2)
int isod1, isod2;
{
	return(WFetchCompKeys(isod1, isod2, fFalse));
}


/*  %%Function:  ScCmpAlphaCIDesIsod  %%Owner:  bobz       */

SC ScCmpAlphaCIDesIsod(isod1, isod2)
int isod1, isod2;
{
	return(-WFetchCompKeys(isod1, isod2, fFalse));
}


/*  %%Function:  ScCmpAlphaAscIsod  %%Owner:  bobz       */

SC ScCmpAlphaAscIsod(isod1, isod2)
int isod1, isod2;
{
	return(WFetchCompKeys(isod1, isod2, fTrue));
}


/*  %%Function:  ScCmpAlphaDesIsod  %%Owner:  bobz       */

SC ScCmpAlphaDesIsod(isod1, isod2)
int isod1, isod2;
{
	return(-WFetchCompKeys(isod1, isod2, fTrue));
}


/*  %%Function:  WFetchCompKeys  %%Owner:  bobz       */

WFetchCompKeys(isod1, isod2, fCase)
int isod1, isod2, fCase;
{{ /* NATIVE - WFetchCompKeys */
	struct SOD HUGE *hpsod;
	struct CA caKey;
	CHAR szKey1[cchMaxSKFetch];
	CHAR szKey2[cchMaxSKFetch];

	caKey.doc = selCur.doc;
	hpsod = HpInPlf(hplfSod, isod1);
	caKey.cpFirst = hpsod->cpFirstKey;
	caKey.cpLim = hpsod->cpLimKey;
	FFetchSortKey(szKey1, cchMaxSKFetch, &caKey, chSep, cFldSkip);

	hpsod = HpInPlf(hplfSod, isod2);
	caKey.cpFirst = hpsod->cpFirstKey;
	caKey.cpLim = hpsod->cpLimKey;
	FFetchSortKey(szKey2, cchMaxSKFetch, &caKey, chSep, cFldSkip);

	return (WCompSzSrt(szKey1, szKey2, fCase));
}}


/* *********************** WARNING ******************************* */
/* The following 2 functions are strictly machine and compiler     */
/* dependent.  It exploits the fact that lDttm in dttm is      */
/* numerically greater than another one if the date and time it    */
/* represents is chronologically later than the one of the other.  */
/* ******************* End of Warning **************************** */


/*  %%Function:  ScCmpDttmAscIsod  %%Owner:  bobz       */

SC ScCmpDttmAscIsod(isod1, isod2)
int isod1, isod2;
{{ /* NATIVE - ScCmpDttmAscIsod */
	long dl;
	struct DTTM dttm1, dttm2;
	struct SOD HUGE *hpsod1 = HpInPlf(hplfSod, isod1);
	struct SOD HUGE *hpsod2 = HpInPlf(hplfSod, isod2);

	dttm1.lDttm = hpsod1->dttmKey.lDttm;
	dttm2.lDttm = hpsod2->dttmKey.lDttm;

	/* Mask off day of the week. */
	dttm1.wdy = dttm2.wdy = 0;
	dl = dttm1.lDttm - dttm2.lDttm;
	return (DlToSc(dl));
}}


/*  %%Function:  ScCmpDttmDesIsod  %%Owner:  bobz       */

SC ScCmpDttmDesIsod(isod1, isod2)
int isod1, isod2;
{{ /* NATIVE - ScCmpDttmDesIsod */
	long dl;
	struct DTTM dttm1, dttm2;
	struct SOD HUGE *hpsod1 = HpInPlf(hplfSod, isod1);
	struct SOD HUGE *hpsod2 = HpInPlf(hplfSod, isod2);

	dttm1.lDttm = hpsod1->dttmKey.lDttm;
	dttm2.lDttm = hpsod2->dttmKey.lDttm;

	/* Mask off day of the week. */
	dttm1.wdy = dttm2.wdy = 0;
	dl = dttm2.lDttm - dttm1.lDttm;
	return (DlToSc(dl));
}}


/*  %%Function:  FParseKeyAlphaNum  %%Owner:  bobz       */

BOOL FParseKeyAlphaNum(pca, psod)
struct CA *pca;
struct SOD *psod;
{
	psod->cpFirstKey = pca->cpFirst;
	psod->cpLimKey = pca->cpLim;
	return fTrue;
}


/*  %%Function:  FParseKeyNumeric  %%Owner:  bobz       */

BOOL FParseKeyNumeric(sz, psod)
CHAR *sz;
struct SOD *psod;
{
	PES pes;
	int f;

	if (!*sz)
		{
LErrRet:
		numNegBig;  /* return large negative # as key */
		StiNum(&(psod->numKey));

#ifdef BZ
		CommSzPnum(SzShared("Inval Numeric arg: "), &(psod->numKey));
#endif /* BZ */

		return fTrue;
		}

	/* parse a value */
	InitPpes(&pes);

	if (FCalculateNumFromSz(sz, &(psod->numKey), &pes))
		{
#ifdef BZ
		CommSzPnum(SzShared("Numeric arg: "), &(psod->numKey));
#endif /* BZ */
		return fTrue;
		}
	else
		goto LErrRet;
}


/*  %%Function:  FParseKeyDate  %%Owner:  bobz       */


BOOL FParseKeyDate(sz, psod)
CHAR *sz;
struct SOD *psod;
{
	if (*sz)
		{
		if (FDttmFromSzPl(sz, &(psod->dttmKey), (long *) NULL))
			return fTrue;
		else
			goto LErrRet;
		}
	else
		{
LErrRet:
		psod->dttmKey.lDttm = 0L;
		return fTrue;
		}
}


#ifdef BOGUS /* See the other "BOGUS" comment for an explanation */
/*  %%Function:  FCpNL_EOL  %%Owner:  bobz       */

NATIVE BOOL FCpNL_EOL(cp) /* WINIGNORE - ifdef BOGUS */
CP cp;
{
	CHAR ch;

	/* Return true if a char at cp is a newline or eol. */
	FetchCp(sot.ca.doc, cp, fcmChars);
	ch = *vhpchFetch;
	return FChNL_EOL(ch);
}


#endif /* BOGUS */

/* The following routines used to reside in memrare.c */

extern struct MERR  vmerr;

/* H P  I N  P L F */
/* Get huge pointer to ith element in plf */
/*  %%Function:  HpInPlf  %%Owner:  bobz       */

NATIVE CHAR HUGE *HpInPlf(hplf, i)
struct PLF **hplf;
int i;
{
	int isb;
	int iInSb;
	SB sb;
	struct PLF *pplf = *hplf;

	AssertH(hplf);
	Assert(i < pplf->iMac);
	Assert(pplf->iMac <= pplf->iMax);
	Assert(pplf->iMax == pplf->isbMac * pplf->cFooChunk);

	isb = i / pplf->cFooChunk;
	iInSb = i % pplf->cFooChunk;

	sb = *(SB *)PInPl(hplf, isb);

	return HpOfSbIb(sb, (iInSb * pplf->cbFoo));
}


/* H P L F  I N I T */
/*  %%Function:  HplfInit  %%Owner:  bobz       */

struct PLF **HplfInit(cb, iMax)
int cb, iMax;
{
	extern int cbMemChunk;
	struct PLF **hplf, *pplf;
	int isb;
	int isbMac;
	int cFooChunk;

	cFooChunk = cbLmemHeap / cb;
	isbMac = (iMax + cFooChunk - 1) / cFooChunk;

	if ((hplf = HplInit2(sizeof(SB), cbPLFBase, isbMac, fFalse)) == hNil)
		return hNil;

	pplf = *hplf;
	pplf->iMac = 0;
	pplf->iMax = 0;
	pplf->cbFoo = cb;
	pplf->cFooChunk = cFooChunk;

	if (!FChngSizeHplf(hplf, iMax))
		{
		FreeHplf(hplf);
		return hNil;
		}

	Assert((*hplf)->iMax >= iMax);

	(*hplf)->iMac = 0;

	return hplf;
}




/* F R E E  H P L F */
/*  %%Function:  FreeHplf  %%Owner:  bobz       */

FreeHplf(hplf)
struct PLF **hplf;
{
	SB *psb, *psbMac;

	psb = PInPl(hplf, 0);
	psbMac = psb + (*hplf)->isbMac;

#ifdef BZ
	CommSzNum(SzShared("FreeHplf isbMac: "), (*hplf)->isbMac);
#endif

	while (psb < psbMac)
		{
#ifdef BZ
		CommSzNum(SzShared("FreeHplf arg to FreeEmmSb: "), *psb);
#endif
		FreeEmmSb(*psb++);
		}

	FreeH(hplf);
}



/* I  A P P E N D  T O  H P L F */
/*  %%Function:  IAppendToHplf  %%Owner:  bobz       */

IAppendToHplf(hplf, pch)
struct PLF **hplf;
CHAR *pch;
{
	int iMacNew = (*hplf)->iMac + 1;
	AssertH(hplf);
	if (!FChngSizeHplf(hplf, iMacNew))
		return iNil;
	PutPlf(hplf, iMacNew-1, pch);
	return iMacNew-1;
}


/* F  C H N G  S I Z E  H P L F */
/*  %%Function:  FChngSizeHplf  %%Owner:  bobz       */

FChngSizeHplf(hplf, iMacNew)
struct PLF **hplf;
int iMacNew;
{
	struct PLF *pplf = *hplf;
	int iMinSb = (pplf->isbMac - 1) * pplf->cFooChunk;
	int isbMacNew = (iMacNew + pplf->cFooChunk - 1) / pplf->cFooChunk;

	if (iMacNew < iMinSb)
		/* reduce number of sbs */
		{
		SB *psb, *psbMacNew;
		psb = PInPl(hplf, pplf->isbMac-1);
		psbMacNew = PInPl(hplf, isbMacNew);

		while (psb >= psbMacNew)
			FreeEmmSb(*psb--);

		pplf->isbMac = isbMacNew;
		pplf->iMax = isbMacNew * pplf->cFooChunk;
		}

	else  if (iMacNew > pplf->iMax)
		/* must allocate more sbs */
		{
		int isb;
		int cFooChunk = pplf->cFooChunk;
		int cbChunk = cFooChunk * pplf->cbFoo;

		for (isb = pplf->isbMac; isb < isbMacNew; isb++)
			{
			SB sb = SbAllocEmmCb(cbChunk);
			if (sb == sbNil)
				goto LFail;
			if (!FInsertInPl(hplf, isb, &sb))
				{
				FreeEmmSb(sb);
LFail:
				return fFalse;
				}
			(*hplf)->iMax += cFooChunk;
			}
		}

	(*hplf)->iMac = iMacNew;

	Assert(iMacNew <= (*hplf)->iMax);
	Assert(iMacNew >= ((*hplf)->isbMac-1) * (*hplf)->cFooChunk);
	Assert((*hplf)->iMax == (*hplf)->isbMac * (*hplf)->cFooChunk);

	return fTrue;
}


#ifdef NOTUSED

/* G E T  P L F */
/*  %%Function:  GetPlf  %%Owner:  bobz       */

GetPlf(hplf, i, pch)
struct PLF **hplf;
int i;
CHAR *pch;
{
	bltbh(HpInPlf(hplf, i), (CHAR HUGE *)pch, (*hplf)->cbFoo);
}


#endif /* NOTUSED */

/* P U T  P L F */
/*  %%Function:  PutPlf  %%Owner:  bobz       */

PutPlf(hplf, i, pch)
struct PLF **hplf;
int i;
CHAR *pch;
{
	bltbh((CHAR HUGE *)pch, HpInPlf(hplf, i), (*hplf)->cbFoo);
}
