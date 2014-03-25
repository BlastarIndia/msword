/*      Compare.c
			Author: Dave Bourne
			Date:   10-21-85
			version: 3.00

		Routines that can be called from the outside world:

		CompareDocs(docCur, docOld)

			Code to compare two documents and show revision marks.
			The current version compares the documents at the
			paragraph level and will show revision marks only when
			there is a paragraph in the new document not in the
			old document.  Revision marks will not be shown when
			the new document is missing a paragraph in the old
			document nor when paragraphs in the new document are
			out of sequence with those in the old.

			The only data structure of note is the Compare Table
			Entry, or "Cmte", which contains a paragraph hash
			code, or "parhc" and a pointer to the next compare
			table entry.

			This code does its own dynamic memory allocation
			from a block of memory with a "finger" called
			"hcmteMem".  The variables "icmteMax" and "icmteMac"
			are used to keep track of how many Cmte's can be
			stored and how many Cmte's are currently stored in the
			block of memory, respectively.  The block of memory
			is grown when it runs out of space.
*/

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "props.h"
#include "border.h"
#include "sel.h"
#include "prm.h"
#include "doc.h"
#include "debug.h"
#include "disp.h"
#include "dlbenum.h"
#include "error.h"
#define REVMARKING
#include "compare.h"
#include "status.h"
#include "cmd.h"
#include "screen.h"
#include "prompt.h"
#include "message.h"
#include "ch.h"

#include "idd.h"
#include "sdmdefs.h"
#include "sdmver.h"
#include "sdm.h"
#include "sdmtmpl.h"
#include "doslib.h"
#include "core.h"

#include "revmark.hs"
#include "revmark.sdm"
#include "cmpfile.hs"
#include "cmpfile.sdm"

extern BOOL		vfRecording;
extern HWND		vhwndCBT;
extern BOOL		fElActive;
extern BOOL             vfOvertypeMode;
extern HWND             vhwndStatLine;
extern struct SEL       selCur;
extern struct CA        caTap;
extern struct CA        caPara;
extern struct CA        caTable;
extern struct TCC       vtcc;
extern struct CHP       vchpFetch;
extern struct PAP       vpapFetch;
extern struct TAP       vtapFetch;
extern CP               vcpFetch;
extern CHAR HUGE        *vhpchFetch;
extern int              vccpFetch;
extern int              wwCur;
extern int              docMac;
extern int              vfDeactByOtherApp;
extern struct DOD       **mpdochdod[];
extern CHAR             szApp[];
extern CHAR	    	szDoc[];
extern CHAR		stDOSPath[];
extern BOOL		vfFileCacheDirty;
extern struct MERR      vmerr;
extern struct SCI	vsci;

static uns      **hcmteMem;
static int      icmteMac;
static int      icmteMax;
static BOOL	fWholeDoc;

CMD CmdChangeRevMarking();

/* %%Function:CmdCompare %%Owner:davidbo */
CMD CmdCompare(pcmb)
CMB * pcmb;
{
	int docCur, docOld, fDirty, nIncr, dof;
	struct PPR **hppr;
	CP cp, cpMacReport;
	struct CA ca;
	char stT[ichMaxFile];
	char stFile[ichMaxFile];
	int  rgicmte[rgicmteTblSize];

	docCur = DocMother(selCur.doc);

	if (!FSetUndoB1(imiCompare, uccFormat, PcaSetWholeDoc(&ca,docCur)))
		return cmdCancelled;

	CloseEveryFn (fFalse);	/* because user is likely to 
	change floppies when running this command */

	GetDocSt(docCur, stFile, gdsoRelative | gdsoNoUntitled);
	if (!FGetCmpStFile(pcmb, stFile, ichMaxFile))
		return cmdCancelled;

	if (pcmb->fCheck)
		{
		if (!FTermFile(pcmb, Iag(CABCMPFILE, hszFile), 
				tmcNull, fFalse, fFalse, nfoNormal))
			return cmdError;
		}

	if (!pcmb->fAction)
		return cmdOK;

	UpdateWw(wwCur, fFalse /* fAbortOk */);

	FNormalizeStFile(stFile, stT, nfoNormal);
	GetDocSt(docCur, stFile, gdsoFullPath);
	dof = FEqNcSt(stT, stFile) ? (dofNoMemory | dofNoWindow) : (dofNoWindow);
	if ((docOld = DocOpenStDof(stT, dof, NULL)) == docNil)
		return cmdError;

	StartLongOp ();

	hcmteMem = hcmteMemNull;
	icmteMac = icmteMax = 0;
	InitRgicmte(rgicmte);
	cpMacReport = CpMacDoc(docOld) + CpMacDoc(docCur);
	nIncr = NIncrFromL(cpMacReport);
	cp = cp0;
	if ((hppr = HpprStartProgressReport(mstCompare, &cp, nIncr, fTrue)) == hNil)
		goto LRet;
	if (FQueryAbortCheck())
		goto LAbort;
	fDirty = fFalse;
	
	/* REVIEW davidbo (sah):If FHashDoc fails should cmd = cmdOutOfMemory? */

	if (FHashDoc(docOld, fFalse, rgicmte, &fDirty, hppr, &cp, cpMacReport) &&
			FHashDoc(docCur, fTrue, rgicmte, &fDirty, hppr, &cp, cpMacReport) &&
			fDirty)
		{
		InvalCp(PcaSetWholeDoc(&ca, docCur));
		TrashWw(wwCur);
		DirtyDoc(docCur);
		SetUndoAfter(NULL);
		}
LAbort:
	StopProgressReport(hppr, pdcRestore);
LRet:
	FreeH(hcmteMem);
	DisposeDoc (docOld);
	KillExtraFns();
	EndLongOp (fFalse /* fAll */);

	return cmdOK;
}


/*
*      Rol is used in generating paragraph hash codes.  It implements
*      a "long" left rotate.
*/

/* %%Function:Rol %%Owner:davidbo */
long Rol(parhc,amount)
ParHc parhc;
short amount;
{
	return((parhc << amount) +
			((parhc >> (SizeParHc - amount)) & ((1 << amount) - 1)));
}


/*
*      IcmteAlloc allocates space from the "global" block of memory
*      indirectly pointed to by hcmteMem.  The first time it is
*      called, it creates a block big enough for "ccmte" entries.
*      Whenever the block runs out of space, it is grown to accomodate
*      another "ccmte" entries.
*      Since the hash table functions treat this global block as a
*      big array of Cmte's, IcmteAlloc returns an index into this
*      array.  The hash table functions use these indices as pointers
*      when building linked lists.
*/
/* %%Function:IcmteAlloc %%Owner:davidbo */
IcmteAlloc()
{
	int     cw,cwMem;

	if (icmteMac < icmteMax)
		return(icmteMac++);
	else
		{
		cw = cbcmte * ccmte / sizeof(int);
		cwMem = icmteMax * cbcmte / sizeof(int);
		if (hcmteMem == hcmteMemNull)
			{
			if (FNoHeap(hcmteMem = HAllocateCw(cw)))
				return(icmteNull);
			icmteMax = ccmte;
			return(icmteMac++);
			}
		else  if (!FChngSizeHCw(hcmteMem,cwMem + cw,fFalse))
			return(icmteNull);
		else
			{
			icmteMax += ccmte;
			return(icmteMac++);
			}
		}
}


/*
*      Initrgicmte initializes the hash table to point to...
*      well...
*      ahhh...
*      nothing.
*/
/* %%Function:InitRgicmte %%Owner:davidbo */
InitRgicmte(rgicmte)
int rgicmte[];
{
	long iicmte;

	for (iicmte = 0; iicmte < rgicmteTblSize; iicmte++)
		rgicmte[iicmte] = icmteNull;
}


/*
*      Given a paragraph hash code, IcmteFind returns either a
*      pointer to that hash codes's entry in the hash table or
*      it returns "icmteNull" if the hash code isn't in the
*      table.
*/
/* %%Function:IcmteFind %%Owner:davidbo */
IcmteFind(parhc,rgicmte)
ParHc parhc;
int rgicmte[];
{
	register int    icmte;
	int             iicmte;
	struct Cmte     *rgcmte;

	iicmte = (parhc & AbsMask) % rgicmteTblSize;
	rgcmte = (struct Cmte *) *hcmteMem;
	for (icmte = rgicmte[iicmte]; icmte != icmteNull; )
		{
		if (rgcmte[icmte].parhc == parhc)
			return(icmte);
		icmte = rgcmte[icmte].icmteNext;
		}
	return(icmteNull);
}


/*
*      IcmteCreate generates a new Cmte and places it in the
*      hash table.
*/
/* %%Function:IcmteCreate %%Owner:davidbo */
IcmteCreate(parhc)
ParHc parhc;
{
	struct Cmte *pcmte,*rgcmte;
	int icmte;

	if ((icmte = IcmteAlloc()) == icmteNull)
		return(icmteNull);
	rgcmte = (struct Cmte *) *hcmteMem;
	pcmte = &rgcmte[icmte];
	pcmte->parhc = parhc;
	pcmte->icmteNext = icmteNull;
	return(icmte);
}


/*
*      FInsertCmte takes a paragraph hash code and adds it
*      to the hash table if it is a new paragraph.
*/
/* %%Function:FInsertCmte %%Owner:davidbo */
FInsertCmte(parhc,rgicmte)
ParHc parhc;
int rgicmte[];
{
	int icmte,iicmte;
	struct Cmte *pcmte,*rgcmte;

	iicmte = (parhc & AbsMask) % rgicmteTblSize;
	if (IcmteFind(parhc,rgicmte) == icmteNull)
		{
		if ((icmte = IcmteCreate(parhc)) == icmteNull)
			return(fFalse);
		rgcmte = (struct Cmte *) *hcmteMem;
		pcmte = &rgcmte[icmte];
		pcmte->icmteNext = rgicmte[iicmte];
		rgicmte[iicmte] = icmte;
		}
	return(fTrue);
}


/*
*      FHashDoc runs through an entire document one paragraph at a time,
*      generating a hash code for each paragraph, and calling
*      FInsertCmte with each hash code.
*/
/* %%Function:FHashDoc %%Owner:davidbo */
FHashDoc(doc, fDocNew, rgicmte, pfDirty, hppr, pcpOffset, cpMacReport)
int doc, fDocNew;
int rgicmte[];
int *pfDirty;
struct PPR **hppr;
CP *pcpOffset, cpMacReport;
{
	int             iPara;
	CHAR HUGE       *hpch;
	long            parhc;
	CP              cpFirst,cpLim,cpMacDoc,cpMacDocE,ccp;
	struct DOD      *pdod;
	char            grpprl[2];
	struct CA       caT;

	cpMacDoc = CpMacDoc(doc);
	cpMacDocE = CpMacDocEdit(doc);
	CachePara(doc,cp0);
	*pfDirty = fFalse;
	iPara = 0;
	for (;;)
		{
		parhc = 0;
		cpFirst = caPara.cpFirst;
		cpLim = CpMin(caPara.cpLim,cpMacDoc);

		if (!(iPara % 4))
			{
			if (FQueryAbortCheck())
				/* if old doc, return fFalse so we don't hash the new one */
				return (fDocNew);
			ProgressReportPercent(hppr, cp0, cpMacReport,*pcpOffset+cpLim,NULL);
			}
		iPara++;
		FetchCp(doc,cpFirst,fcmChars);
		for (;;)
			{
			ccp = CpMin((CP) vccpFetch,cpLim - vcpFetch);
			for (hpch = vhpchFetch; ccp-- > cp0; hpch++)
				parhc = Rol(parhc,5) + *hpch;
			if (vcpFetch + vccpFetch >= cpLim)
				break;
			FetchCp(docNil,cpNil,fcmChars);
			}

		if (fDocNew)
			{
			if (IcmteFind(parhc,rgicmte) == icmteNull)
				{
				grpprl[0] = sprmCFRMark;
				grpprl[1] = fTrue;
				caT = caPara;
				if (caPara.cpLim == cpMacDoc)
					caT.cpLim = cpMacDocE;
				if (DcpCa(&caT) != cp0)
					ApplyGrpprlCa(grpprl, sizeof(grpprl), &caT);
				*pfDirty = fTrue;
				}
			}
		else  if (!FInsertCmte(parhc,rgicmte))
			return(fFalse);
		if (cpLim >= cpMacDocE)
			break;
		CachePara(doc,cpLim);
		}
	*pcpOffset = cpMacDoc;
	return(fTrue);
}


typedef struct _cerm
	{
	BOOL fDoneAction;
} CERM;

/* %%Function:CmdRevMarking %%Owner:davidbo */
CMD CmdRevMarking(pcmb)
CMB * pcmb;
{
	CABREVMARKING *pcab;
	CERM cerm;

	if (pcmb->fDefaults)
		{
		struct DOD * pdod;

		pdod = PdodMother(selCur.doc);
		pcab = (CABREVMARKING *) *pcmb->hcab;
		pcab->fRevMarking = pdod->dop.fRevMarking;
		pcab->iRMBar = pdod->dop.irmBar;
		pcab->iRMProps = pdod->dop.irmProps;
		cerm.fDoneAction = fFalse;
		}

	if (pcmb->fDialog)
		{
		char dlt [sizeof (dltRevMarking)];

		BltDlt(dltRevMarking, dlt);
		pcmb->pv = &cerm;
		switch (TmcOurDoDlg(dlt, pcmb))
			{
#ifdef DEBUG
		default:
			Assert(fFalse);
			return cmdError;
#endif
		case tmcError:
			return cmdNoMemory;

		case tmcCancel:
			return cerm.fDoneAction ? cmdOK : cmdCancelled;

		case tmcOK:
			break;
			}
		}

	if (pcmb->fAction)
		{
		int rrm = rrmNil;

		switch (pcmb->tmc)
			{
		case tmcRMSearch:
			SearchRM();
			if (fElActive)
				break; /* we want to call CmdChangeRevMarking... */
			return cmdOK;

		case tmcRMRemove:
			rrm = rrmRemove;
			break;

		case tmcRMUndo:
			rrm = rrmUndo;
			break;
			}

		pcab = (CABREVMARKING *) *pcmb->hcab;
		return CmdChangeRevMarking(pcab->fRevMarking, pcab->iRMBar, 
				pcab->iRMProps, rrm, 
				pcmb->tmc == tmcOK || fElActive);
		}

	return cmdOK;
}


/* %%Function:FDlgRevMarking %%Owner:davidbo */
BOOL FDlgRevMarking(dlm, tmc, wNew, wOld, wParam)
DLM dlm;
TMC tmc;
WORD wOld, wNew, wParam;
{
	CMB cmb;

	switch (dlm)
		{
	case dlmInit:
		if (vhwndCBT == NULL)
			{
			extern HWND vhwndApp;
			struct RC rcApp, rcDlg;
			HWND hwnd;

			 /* MoveDlg not safe if dialog coming down */
			if (!FIsDlgDying())
				{
				/* position the dialog out of the way */
				GetWindowRect(vhwndApp, (LPRECT) &rcApp);
				Assert (HdlgGetCur() != hdlgNull);
				hwnd = HwndFromDlg(HdlgGetCur());
				Assert (hwnd != NULL);
				GetWindowRect(hwnd, (LPRECT) &rcDlg);
			   	MoveDlg(rcDlg.xpLeft, 
			   		rcApp.ypBottom - vsci.dypBdrCapt -
			   		rcDlg.ypBottom + rcDlg.ypTop);
			   	}
			}
		break;

	case dlmIdle:
		/* turn on selection highlight */
		/* one time only process since true returned */
		if (!vfDeactByOtherApp)
			{
			if (!selCur.fNil && selCur.fHidden)
				TurnOnSelCur();
			}
		break;

	case dlmClick:
		Assert(tmc == tmcRMSearch || 
				tmc == tmcRMRemove || 
				tmc == tmcRMUndo);

		if ((cmb.hcab = HcabFromDlg(fFalse)) == hcabNotFilled)
			return fFalse;
		if (cmb.hcab == hcabNull)
			/* sdm filter will take down dialog */
			return fTrue;

		cmb.cmm = cmmAction;
		cmb.tmc = tmc;
		CmdRevMarking(&cmb);

		if (tmc != tmcRMSearch)
			{
			RMChangeCancelToClose((CERM *) PcmbDlgCur()->pv);
			SetFocusTmc(fWholeDoc ? tmcOK : tmcRMSearch);
			}
		/* break; */
		}

	return fTrue;
}


/* %%Function:RMChangeCancelToClose %%Owner:davidbo */
RMChangeCancelToClose(pcerm)
CERM *pcerm;
{
	if (!pcerm->fDoneAction)
		{
		SetTmcText(tmcCancel, SzSharedKey("Close",Close));
		pcerm->fDoneAction = fTrue;
		}
}


/* %%Function:CmdChangeRevMarking %%Owner:davidbo */
CMD CmdChangeRevMarking(fRevMarking, irmBar, irmProps, rrm, fChangeDop)
int fRevMarking, irmBar, irmProps, rrm, fChangeDop;
{
	int doc, fSubDocs, fTrash;
	CMD cmd;
	struct CA ca;

	fWholeDoc = fFalse;
	/* check validity of passed parameters */
	if (!(irmBar >= irmBarNone && irmBar <= irmBarOutside &&
			irmProps >= irmPropsNone && irmProps <= irmPropsDblUnder &&
			(rrm == rrmNil || rrm == rrmUndo || rrm == rrmRemove)))
		{
		return cmdError;
		}

	fTrash = fFalse;
	if (fChangeDop)
		{
		struct DOD *pdod = PdodMother(selCur.doc);
		/* only change dop (and trash wws) when things change */
		fTrash = pdod->dop.irmProps != irmProps || pdod->dop.irmBar != irmBar;
		if (fTrash || pdod->dop.fRevMarking != fRevMarking)
			DirtyDoc(DocMother(selCur.doc));
		pdod->dop.fRevMarking = fRevMarking;
		pdod->dop.irmBar = irmBar;
		pdod->dop.irmProps = irmProps;
		}
	if (fRevMarking)
		vfOvertypeMode = fFalse;
	if (vhwndStatLine != NULL)
		UpdateStatusLine(usoToggles);
	cmd = cmdOK;
	if (rrm != rrmNil)
		{
		fSubDocs = selCur.fIns;
		if (fSubDocs)
			{
			fWholeDoc = fTrue;
			doc = DocMother(selCur.doc);
			/* don't use PcaSetWholeDoc because it doesn't include eop */
			PcaSet(&ca, doc, cp0, CpMacDoc(doc));
			if (!fElActive &&
					IdMessageBoxMstRgwMb(rrm == rrmRemove ? mstAcceptAllRM : 
					mstUndoAllRM, NULL, MB_YESNOQUESTION) != IDYES)
				{
				return cmdCancelled;
				}
			}
		else
			{
			ca = selCur.ca;
			}

		TurnOffSel(&selCur);

		if (!FSetUndoB1(ucmRevMark, uccPaste, &ca))
			return cmdError;
		InvalCp(&ca);
		StartLongOp();
		if (!FRemoveRevMarking(&ca, rrm, fSubDocs))
			{
			cmd = cmdError;
			EndLongOp(fFalse);
			goto LErr;
			}
		EndLongOp(fFalse);
		if (fSubDocs)
			SetUndoNil();
		else
			SetUndoAfter(&ca);
LErr:
		/* don't use PcaSetWholeDoc because it doesn't include eop */
		if (fWholeDoc)
			PcaSet(&ca, selCur.doc, cp0, CpMacDoc(selCur.doc));
		if (FInTableDocCp(ca.doc, ca.cpFirst) &&
				(ca.cpLim > cp0 && FInTableDocCp(ca.doc, ca.cpLim-1)))
			{
			CacheTable(ca.doc, ca.cpFirst);
			if (ca.cpFirst >= caTable.cpFirst && ca.cpLim <= caTable.cpLim)
				SelectRow(&selCur, ca.cpFirst, ca.cpLim);
			else
				goto LSelect;
			}
		else
			{
LSelect:
			Select( &selCur, ca.cpFirst, ca.cpLim );
			}

		TurnOnSelCur();
		if (!fTrash)
			TrashWwsForDoc(selCur.doc);
		}

	if (fTrash)
		{
		int ww, docMother, docHdrDisp;
		struct DOD *pdodMother;

		Assert(fChangeDop);
		Assert(rrm == rrmNil || fElActive);
		TrashWwsForDoc(docMother = DocMother(selCur.doc));
		pdodMother = PdodDoc(docMother);
		if (pdodMother->docFtn != docNil)
			TrashWwsForDoc(pdodMother->docFtn);
		if (pdodMother->docAtn != docNil)
			TrashWwsForDoc(pdodMother->docAtn);
		/*
		*  hdr disp chain looks like:
		*    docMother->docHdr->docHdrDisp->docHdrDisp->...->docNil
		*/
		if ((docHdrDisp = pdodMother->docHdr) != docNil)
			{
			while ((docHdrDisp = PdodDoc(docHdrDisp)->docHdr) != docNil)
				{
				TrashWwsForDoc(docHdrDisp);
				}
			}
		}

	return cmd;
}


/*
*  Removes revision marks from *pca.  If fSubDocs, removes
*  revision marks from subdocs as well.
*  If rrm is rrmRemove, struckthru text
*  is deleted and new text loses its newness.  If rrm is rrmUndo, new
*  text is deleted and struckthru text becomes normal text.  If rrm
*  is rrmDelete, new text is deleted and struckthru text remains
*  unchanged.
*  The passed ca is altered to the size of whatever remains.
*
*  Note, rev marking condoms (remember, they are here to protect you
*  and the documents you edit): the final eop is protected against being
*  marked new or struck through.  FDeleteableEop also protects against
*  letting a new or struck through eop become the final eop.  Also,
*  FDeleteCkBlockPsel keeps us from deleting things we shouldn't be
*  deleting.
*/
/* %%Function:FRemoveRevMarking %%Owner:davidbo */
FRemoveRevMarking(pca, rrm, fSubDocs)
struct CA *pca;
int rrm, fSubDocs;
{
	struct DOD *pdod;
	char grpprl[4];
	int doc, fTreatEop;
	CP cpLim, cpT, dcp;
	struct CA caRun, caT;
	struct SEL sel;

	sel = selCur;
	sel.fIns = fFalse;
	sel.fBlock = fFalse;
	sel.fTable = fFalse;
	fTreatEop = fFalse;
	if ((cpT = CpMacDocEdit(pca->doc)) < pca->cpLim)
		{
		fTreatEop = fTrue;
		cpLim = pca->cpLim;
		pca->cpLim = cpT;
		}
	for (caRun = *pca; caRun.cpFirst < pca->cpLim; caRun.cpFirst = caRun.cpLim)
		{
		FetchCpAndParaCa(&caRun, fcmProps);
		caRun.cpLim = CpMin( pca->cpLim, vcpFetch + vccpFetch );

		switch (rrm)
			{
		case rrmStripHidden:
			if (vchpFetch.fVanish)
				goto LDelete;
			break;

		case rrmDelete:
			if (vchpFetch.fRMark)
				goto LDelete;
			else  if (!vchpFetch.fStrike)
				{
				grpprl[0] = sprmCFStrikeRM;
				grpprl[1] = fTrue;
				goto LApplySprm;
				}
			break;

		case rrmUndo:
			if (vchpFetch.fRMark)
				goto LDelete;
			else  if (vchpFetch.fStrike)
				{
				grpprl[0] = sprmCFStrikeRM;
				grpprl[1] = fFalse;
				goto LApplySprm;
				}
			break;

		case rrmRemove:
			if (vchpFetch.fStrike)
				{
LDelete:
				/*  not even this function is allowed to delete
					unmatched field characters! */
				AssureLegalSel(&caRun);     /* invalidates v*Fetch */
				CachePara(caRun.doc, caRun.cpFirst); /* revalidate v*Fetch */
				if (FInTableVPapFetch(caRun.doc, caRun.cpFirst))
					{
					RemoveRevTable(&caRun);
					if (DcpCa(&caRun) == 0)
						continue;
					}
				sel.ca = caRun;
				/* okay to pass uninitialized ca since we're using rpkNil */
				if (!FDelCkBlockPsel(&sel, rpkNil, &caT, fFalse /*fReportErr*/,
						fFalse /* fSimple */))
					continue;
				if (!FDelete(&caRun))
					return fFalse;
				dcp = DcpCa(&caRun);
				pca->cpLim -= dcp;
				cpLim -= dcp;
				caRun.cpLim = caRun.cpFirst;
				AssureNestedPropsCorrect(&caRun, fFalse);
				}
			else  if (vchpFetch.fRMark)
				{
				grpprl[0] = sprmCFRMark;
				grpprl[1] = fFalse;
LApplySprm:
				ApplySprmRM(&caRun, grpprl, 2);
				}
			break;
			}
		}

	if (fTreatEop)
		{
		caRun.cpFirst = pca->cpLim;
		caRun.cpLim = cpLim;
		grpprl[0] = sprmCFRMark;
		grpprl[1] = fFalse;
		grpprl[2] = sprmCFStrikeRM;
		grpprl[3] = fFalse;
		ApplySprmRM(&caRun, grpprl, 4);
		pca->cpLim = cpLim;
		}

	if (!fSubDocs)
		return fTrue;

	/* Remove revision marking from all subdocs */
	for (doc = docMinNormal; doc < docMac; doc++)
		{
		if (mpdochdod [doc] != hNil && (pdod = PdodDoc(doc))->doc == pca->doc)
			{
			/* don't use PcaSetWholeDoc because it doesn't include eop */
			if (!FRemoveRevMarking(PcaSet(&caT, doc, cp0, CpMacDoc(doc)), rrm, fFalse))
				return fFalse;
			}
		}
	return fTrue;
}


/* %%Function:ApplySprmRM %%Owner:davidbo */
ApplySprmRM(pca, grpprl, cbgrpprl)
struct CA *pca;
char *grpprl;
int cbgrpprl;
{
	struct CA caT, caInval;

	ApplyGrpprlCa(grpprl, cbgrpprl, pca);
	caT = *pca;
	ExpandCaSprm(&caT, &caInval, grpprl);
	InvalCp(&caInval);
	InvalText(&caT, fFalse /* fEdit */);
}


/* %%Function:RemoveRevTable %%Owner:davidbo */
RemoveRevTable(pcaRun)
struct CA *pcaRun;
{
	char rgsprm[3];
	struct CA caCell, caTapLocal, caT;
	CP cpFirstPara;

	CpFirstTap(pcaRun->doc, pcaRun->cpFirst);
/* revalidate since CpFirst... does cacheparas */
	CachePara(pcaRun->doc, pcaRun->cpFirst);
	cpFirstPara = caPara.cpFirst;
	if (vpapFetch.fTtp)
		{
		/* if looking at final chTable, skip it!  Special case below is the 
			only legal way to nuke final chTable. */
		*pcaRun = caTap;
		pcaRun->cpFirst = pcaRun->cpLim;
		}
	else
		{
		AssertDo(PcaCell(&caCell, pcaRun->doc, pcaRun->cpFirst));
		if (pcaRun->cpLim == caCell.cpLim)
			{
			if (vtapFetch.rgtc[vtcc.itc].fFirstMerged || vtapFetch.rgtc[vtcc.itc].fMerged)
				{
				/* Cannot delete these */
				caT = *pcaRun;
				caT.cpFirst = caT.cpLim - ccpEop;
				ApplyRevMarking(&caT, fFalse, fFalse);
				pcaRun->cpLim -= ccpEop;
				}
			else
				{
				/* special case: if table has only a single cell, nuke the table */
				if (vtapFetch.itcMac == 1)
					{
					pcaRun->cpLim += ccpEop;
					return;
					}
				caTapLocal = caTap;
				InvalCp1(&caTapLocal);
				rgsprm[0] = sprmTDelete;
				rgsprm[1] = (char)vtcc.itc;
				rgsprm[2] = (char)vtcc.itc+1;
				ApplyGrpprlCa(rgsprm, 3, &caTapLocal);
				/* if we're nuking cell mark, can't leave paras with table
					property behind */
				if (cpFirstPara > caCell.cpFirst)
					ApplyTableProps(PcaSet(&caT, caCell.doc, caCell.cpFirst,
							cpFirstPara), fFalse);
				}
			}
		}
}


/* %%Function:ApplyRevMarking %%Owner:davidbo */
ApplyRevMarking(pca, fRMark, fStrike)
struct CA *pca;
int fRMark, fStrike;
{
	char grpprl[4];

	grpprl[0] = sprmCFRMark;
	grpprl[1] = fRMark;
	grpprl[2] = sprmCFStrikeRM;
	grpprl[3] = fStrike;
	ApplyGrpprlCa(grpprl, sizeof(grpprl), pca);
}


/* %%Function:SearchRM %%Owner:davidbo */
SearchRM()
{
	int fHaveStart, fRM, ccp, docFetch, doc, idMsgBox;
	int fStartInTable, fLimM1InTable, fSearchWrap;
	CP cp, cpMac, cpFirstRM, cpLimRM;
	CP ccpSkip;

	doc = docFetch = selCur.doc;
	cp = selCur.ca.cpLim;
	ccp = 1; /* set to something "not 0" */
	cpMac = CpMacDocEdit(doc);

	fHaveStart = fSearchWrap = fStartInTable = fLimM1InTable = fFalse;
	StartLongOp();
	idMsgBox = 0;
	for (;; cp += ccp)
		{
		if (ccp == 0 || cp >= cpMac)
			{
			if (fHaveStart)
				{
				ccpSkip = 0;
				cp = cpMac;
				goto LCheckForTable;
				}
			else
				{
				if (!fSearchWrap && (idMsgBox = IdMessageBoxMstRgwMb(mstSearchWrapFwd, NULL, MB_YESNOQUESTION)) == IDYES)
					{
					fSearchWrap = fTrue;
					cpMac = selCur.ca.cpLim;
					cp = cp0;
					docFetch = selCur.doc; /* ...back to non-sequential fetch */
					}
				else
					break;
				}
			}

		FetchCpPccpVisible(docFetch, cp, &ccp, selCur.ww, fFalse);
		ccpSkip = vcpFetch - cp;
		cp = vcpFetch;
		docFetch = docNil; /* set up for sequential fetch */
		fRM = vchpFetch.fStrike | vchpFetch.fRMark;
		if (fHaveStart)
			{
			if (!fRM)
				{
LCheckForTable:
				cpLimRM = cp - ccpSkip; /* backup from hidden text */
				Assert(cpLimRM > cpFirstRM);
				fLimM1InTable = FInTableDocCp(doc, cp-1);
				break;
				}
			}
		else  if (fRM)
			{
			fHaveStart = fTrue;
			fStartInTable = FInTableDocCp(doc, cp);
			cpFirstRM = cp;
			}
		}
	EndLongOp (fFalse /* fAll */);

	if (fHaveStart)
		{
		/* expand to the beginning or end of a table row */
		if (fStartInTable)
			{
			cpFirstRM = CpFirstTap(doc, cpFirstRM);
			CacheTable(doc, cpFirstRM);
			}
		if (fLimM1InTable)
			{
			CpFirstTap(doc, cpLimRM - 1);
			cpLimRM = caTap.cpLim;
			}
		AssureCpAboveDyp( cpFirstRM, vsci.dysMacAveLine, fFalse );
		/* if both cps in the same table, do a Table Selection */
		if (fStartInTable && fLimM1InTable &&
				cpFirstRM >= caTable.cpFirst && cpLimRM <= caTable.cpLim)
			SelectRow(&selCur, cpFirstRM, cpLimRM);
		else
			Select(&selCur, cpFirstRM, cpLimRM);
		}
	else  if (idMsgBox != IDNO)
		ErrorEid(eidRevNotFound, "SearchRM");
}



/* F  G E T  C M P  S T  F I L E  */
/*  Bring up a dialog to get a filename.  Return true if OK pressed.
*/
/* %%Function:FGetCmpStFile %%Owner:davidbo */
FGetCmpStFile (pcmb, stCmpFile, cchMax)
CMB * pcmb;
CHAR *stCmpFile;
int cchMax;
{
	CABCMPFILE *pcab;

	CHAR stDOSPathOld[ichMaxFile + 1];
	CHAR szAll[6];
	CHAR szUser [ichMaxBufDlg];
	CHAR *pch;

	if (stCmpFile[0] != 0)  /* already relative and no Untitled */
		{
		StToSz(stCmpFile, szUser);
		pcmb->pv = szUser;
		}

	BuildSzAll(szDoc, szAll);

	/* To check if the path was changed by the file name dialog. */
	CopySt(stDOSPath, stDOSPathOld);

	if (pcmb->fDefaults)
		{
		pcab = (CABCMPFILE *) *pcmb->hcab;
		pcab->iDirectory = uNinchList;
		if (!FSetCabSz(pcmb->hcab, szAll, Iag(CABCMPFILE, hszFile)))
			return fFalse;
		pcmb->tmc = tmcOK;
		}

	if (pcmb->fDialog)
		{
		CHAR dlt [sizeof (dltCmpFile)];

		BltDlt(dltCmpFile, dlt);
		TmcOurDoDlg(dlt, pcmb);
		}

	/* Path has been changed by the dialog, update window titles. */
	if (FNeNcSt(stDOSPath, stDOSPathOld))
		{
		vfFileCacheDirty = fTrue;
		UpdateTitles(); /* changes window menu, vhsttbWnd and captions */

		if (vfRecording)
			RecordChdir(stDOSPath);
		}

	if (pcmb->tmc == tmcOK)
		{
		GetCabSt(pcmb->hcab, stCmpFile, cchMax, Iag(CABCMPFILE, hszFile));

		if (vfRecording)
			FRecordCab(pcmb->hcab, IDDCmpFile, pcmb->tmc, pcmb->bcm);
		}

	if (pcmb->tmc == tmcError)
		{
		SetErrorMat(matMem);
		return fFalse;
		}

	return (pcmb->tmc == tmcOK);
}


/* ****
*
	Function: FDlgCmpFile
*  Author:
*  Copyright: Microsoft 1986
*  Date: 3/10/86
*
*  Description: Dialog function 
*
** ***/

/* %%Function:FDlgCmpFile %%Owner:davidbo */
BOOL FDlgCmpFile(dlm, tmc, wNew, wOld, wParam)
DLM dlm;
TMC tmc;
WORD wOld, wNew, wParam;
{
	int iT;
	CMB * pcmb;

	switch (dlm)
		{
	case dlmClick:
		/* non-atomic combos get no dlmChange on list box click.
			if we get any non-nil entry, be sure OK is enabled
		*/
		if  (tmc == tmcCmpList || tmc == tmcCmpDir)
			{
			if (wNew != uNinchList)
				EnableTmc(tmcOK, fTrue);
			}
		break;

	case dlmChange:
			{
		/* disable Ok if no filename string */

			if  (tmc == (tmcCmpFile & ~ftmcGrouped))
				{
				GrayButtonOnBlank(tmcCmpFile, tmcOK);
				break;
				}

			break;
			}

	case dlmInit:
		if (PcmbDlgCur()->pv != NULL)
			SetTmcText(tmcCmpFile, PcmbDlgCur()->pv);
		break;

	case dlmIdle:
		if (wNew /* cIdle */ == 0)
			return fTrue;  /* call FSdmDoIdle and keep idling */
		else  if (wNew /* cIdle */ == 3)
			FAbortNewestCmg (cmgLoad1, fTrue, fTrue);
		else  if (wNew /* cIdle */ == 5)
			FAbortNewestCmg (cmgLoad2, fTrue, fTrue);
		else  if (wNew > 5)
			return fTrue; /* stop idling */
		return fFalse; /* keep idling */


	case dlmDirFailed:
		ErrorEid(eidBadFileDlg," FDlgCmpFile");
		SetTmcTxs (tmcCmpFile, TxsAll());
		return fFalse;    /* could not fill directory; stay in dlg */

	case dlmTerm:
		UpdateStDOSPath();

		if (tmc != tmcOK)
			break;

		return FTermFile(PcmbDlgCur(), Iag(CABCMPFILE, hszFile),
				tmcCmpFile, fFalse /* fLink */,
				fFalse /* fDocCur */, nfoNormal);
		}

	return fTrue;
}


