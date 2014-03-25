/* E D I T . C */

#ifdef PCJ
/* #define SHOWCPMAC */
#endif /* PCJ */

#define SCREENDEFS
#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "disp.h"
#include "doc.h"
#include "props.h"
#include "border.h"
#include "sel.h"
#include "ch.h"
#include "scc.h"
#include "format.h"
#include "prm.h"
#include "ruler.h"
#include "layout.h"
#include "file.h"
#include "debug.h"
#include "cmd.h"
#include "error.h"
#include "message.h"

#ifdef WIN
#include "dde.h"
#include "idle.h"
#include "field.h"
#include "status.h"
#define REVMARKING
#include "compare.h"
#define AUTOSAVE
#include "autosave.h"
#define FINDNEXT
#include "search.h"
#endif /* WIN */


/* E X T E R N A L S */

extern struct CA        caHdt;
extern int              vdocHdrEdit;
extern struct CA        caSect;
extern struct PAP       vpapFetch;
extern struct DOD       **mpdochdod[];
extern struct SAB       vsab;
extern struct SEP       vsepFetch;
extern int              wwMac;
extern struct MERR      vmerr;
extern struct RULSS	vrulss;
extern int              vdocFetch;
extern struct DBS       vdbs;
extern struct ESPRM     dnsprm[];

#ifdef WIN
extern ASD              asd;    /* autosave descriptor. */
extern int              vdocScratch;
extern struct DDES      vddes;
extern BOOL             vfDdeIdle;
extern IDF		vidf;
extern FNI	        fni;
extern struct FB        vfbSearch;
#endif /* WIN */

#ifdef WIN
BOOL vfNoInval=0; /* can be > 1 */
#else
#define vfNoInval fFalse
#endif /* WIN */

struct PLC              *HplcCreateEdc();


/* TRANSACTION PROCEDURES: XFoo(fPlan, pxbc, pxs).

Transactions are atomic operation which may be executed without failure or not
at all. They are implemented by "bifuricating" the procedure into a fPlan
phase and an execution (!fPlan) phase. In the former, a list of "intentions"
of using resources is created. At the end of the first phase, the list is
examined by FDoTns procedure which will check if it can reserve all of
the resources which will be needed and return fTrue iff successful.


If fPlan, only idempotent changes may be made, and a failure may be posted on
the intentions list at any time. Obviously planning operations
can not depend on any changes which are delayed until the !fPlan phase.
Useful quantities may be remembered in xs (xaction state) for the !fPlan phase.

If !fPlan, changes may be made but they must not fail.  Any allocations that
were accounted for during the fPlan phase can be asserted to succeed.  No
allocations that were not accounted for should be attempted.

Examples of idempotent operations:
	Creation of empty subdocs
	stretching of plc's (FStretchPlc)
	invalidations: however it is inefficient to execute these twice!
	allocating prc's (since they will be deleted)
Non-idempotent operations:
	FOpenPlc (even deletions)
	PutCpPlc
	...

Always NOTE: code in XFoo procedures will be executed twice!
*/


BOOL vfInCommit = fFalse;

#define BeginCommit()   AssertDo(!vfInCommit++)
#define EndCommit()     AssertDo(!--vfInCommit)

/* Xaction declarations */

#define tntNil        0
#define tntHplc       1
#define tntHsttb      2
#define tntAbort      3
#define tntCloseHdrWw 4
#define tntDelBkmk    5

/* inTentioN */
struct TN
	{
	int	tnt; /* intention type */
	void  **h; /* affected handle */
	int	c;   /* number of elements or cb */
};
#define itnMax 45 /* GUESS (bump up if we ever overflow) */

/* Xaction Bifurication Control */
struct XBC
	{
	int     ixsMax;
	int     ixsMac;
	char   *rgxs;   /* set of transaction states to handle recursive calls */
	int     itnMac;
	struct TN rgtn[itnMax]; /* list of transactions */
};

/* Xaction State for Adding to Hplc's (XAddToHplcEdit) */
struct XSA
	{
	int iFirst;
	int	c;
};

/* Xaction State for ApplyGrpprl (XApplyGrpprlDocCp) */
struct XSP
	{
	int prm;
/* We don't want fCalledXDeleteHdrText to be DEBUG only, because this would
	require DEBUG and non-DEBUG offsets in structs.inc (BradV, 7/26/89). */
/* used to guarantee XDeleteHdrText doesn't do anything if it is called !fPlan
	when it was not called fPlan. */
	BOOL    fCalledXDeleteHdrText;
};

/* Xaction State for Footnote/Annotation (XAddReferencedText) */
struct XSF
	{
	int	ifooFirst;
	int	ifooLim;
};

/* Xaction State for fieLds (XDeleteFields) */
struct XSL
	{
	int ifld;
	int cfld;
};

/* Xaction State for Bookmarks (XCopyBkmks) */
struct XSB
	{
	int ibklLim;
	int ibkfLim;
	int ibkf;
};

/* Xaction State for Replace */
struct XSR
	{
	/* arguments to function */
	/* arrange dfc, fc, fn, pcaDel in this way to shorten .asm init code. */
	FC      dfc;
	FC      fc;
	int     fn;
	struct CA *pcaDel;
	struct CA *pcaIns;

	/* state saved */
	BOOL    fNotEmptyPcaDel; /* there is text to be deleted */
	int     ipcdInsFirst; /* ipcdFirst of piece in source doc to be inserted */
	int     ipcdInsLast; /* ipcdLast of piece in source doc to be inserted */
	int     cpcd; /* number of pieces being deleted */
	BOOL    fReplHidnSect; /* recursive call to copy final sect mark required */
	BOOL    fNotDelPassCpMac; /* not deleting past end of dest doc */

	/* transaction states for subroutine calls */
	struct XSF xsfFtnDel;  /* footnotes */
	struct XSF xsfFtnCopy;
	struct XSA xsaPgdDel;  /* hplcpgd */
	struct XSA xsaPgdCopy;
	struct XSA xsaSedCopy; /* hplcsed */
	struct XSA xsaSeaDel;  /* hplcsea */
	struct XSA xsaSeaCopy;
	struct XSA xsaPadDel;  /* hplcpad */
	struct XSA xsaPadCopy;
	struct XSA xsaPheDel;  /* hplcphe */
	struct XSA xsaPheCopy;
	struct XSP xsp;        /* XApplyGrpprlDocCp */
#ifdef WIN
	struct XSF xsfAtnDel;  /* annotations */
	struct XSF xsfAtnCopy;
	struct XSL xslDelete;  /* fields */
	struct XSB xsbCopy;    /* bookmarks */
#endif
#ifdef DEBUG
/* used to guarantee the same xs between fPlan and !fPlan */
	int     nxs;
	int     w;
#endif /* DEBUG */
};
#define cbXSR   sizeof(struct XSR)

#ifdef DEBUG
#define CheckPxs(fPlan,pxs,nxsT,wT) (fPlan ? (pxs->nxs=nxsT, pxs->w=wT) : \
										Assert(pxs->nxs==nxsT && pxs->w==wT))
#else
#define CheckPxs(fPlan,pxs,nxsT,wT)
#endif /* DEBUG */

#define nxsHidnSect     1
#define nxsAddRefText   2
#define nxsDelRefText   3
#define nxsDelHdrText   4
#define nxsAddHdrText   5



#ifdef DEBUG /* in editn.asm if WIN */
/* P X S  I N I T */
/* initialize xbc for an Xaction and return the first pxs */
/*  %%Function:PxsInit %%Owner:peterj %%reviewed: 6/28/89  */
HANDNATIVE char *C_PxsInit(pxs, ixsMax, pxbc)
char *pxs;
int ixsMax;
struct XBC *pxbc;
{
	pxbc->ixsMax = ixsMax;
	pxbc->ixsMac = 1;
	pxbc->itnMac = 0;
	pxbc->rgxs = pxs;
	SetBytes(pxs, 0, ixsMax*cbXSR);
	return pxs;
}


#endif /* DEBUG */


#ifdef DEBUG /* in editn.asm if WIN */
/* P O S T  T N */
/* add an intention to the intentions list in xbc */
/*  %%Function:PostTn %%Owner:peterj %%reviewed: 6/28/89 */
HANDNATIVE C_PostTn(pxbc, tnt, h, c)
struct XBC *pxbc;
int tnt;
void **h; /* meaning of h and c depend on tnt */
int c;
{
	struct TN *ptn = &pxbc->rgtn[pxbc->itnMac++];
#ifdef DEBUG
	Assert(pxbc->itnMac <= itnMax);
	switch (tnt)
		{
	case tntAbort:
	case tntNil:
		break;
	case tntHplc:
	case tntHsttb:
		AssertH(h);
		break;
	case tntCloseHdrWw:
		Assert((int)h >= wwDocMin && (int)h < wwMac);
		Assert(PwwdWw((int)h)->wk == wkHdr);
		break;
	case tntDelBkmk:
		Assert(!PdodDoc((int)h/*doc*/)->fShort);
		Assert(c == 0);
		break;
	default:
		Assert(fFalse);
		}
#endif /* DEBUG */
	ptn->tnt = tnt;
	ptn->h = h;
	ptn->c = c;
}


#endif /* DEBUG */

/* P X S  A L L O C */
/* obtain an additional pxs for use in a recursive call */
/*  %%Function:PxsAlloc %%Owner:peterj %%reviewed: 6/28/89  */
char *PxsAlloc(pxbc)
struct XBC *pxbc;
{
	Assert(pxbc->ixsMac < pxbc->ixsMax);
	return pxbc->rgxs + (cbXSR * pxbc->ixsMac++);
}


#ifdef DEBUG /* in editn.asm if WIN */
/* F  D O  T N S */
/* "execute" the intentions stored in rgtn.  return true if they all succeed */
/*  %%Function:FDoTns %%Owner:peterj %%reviewed: 6/28/89 */
HANDNATIVE C_FDoTns(pxbc)
struct XBC *pxbc;
{
	int c, cMax;
	struct TN *ptn, *ptnT, *ptnMac;
	void **h;
	int tnt;
	BOOL fReturn = fFalse;

	for (ptn = pxbc->rgtn, ptnMac = ptn + pxbc->itnMac; ptn < ptnMac; ptn++)
		switch (tnt = ptn->tnt)
			{
		default:
			Assert(fFalse);
			/* fall through */
		case tntNil:
		case tntCloseHdrWw:
		case tntDelBkmk:
			continue;
		case tntAbort:
			goto LReturn;
		case tntHplc:
		case tntHsttb:
/* coalesce the several intentions to allocate into a single
maximal intention.  (i.e., determine the largest that this heap 
block will get during the course of the operation by accumlating
all of the intentions that relate to this heap block and taking
the max that that value obtains). */
			c = cMax = ptn->c;
			h = ptn->h;
			for (ptnT = ptn + 1; ptnT < ptnMac; ptnT++)
				if (ptnT->tnt == tnt && ptnT->h == h)
					{
					if ((c += ptnT->c) > cMax)
						cMax = c;
					/* to avoid double counting */
					ptnT->tnt = tntNil;
					}
/* do allocation, if necessary. fail process if allocation cannot be made */
			if (cMax > 0)
				if (tnt == tntHplc && !FStretchPlc(h, cMax))
					goto LReturn;
				else  if (tnt == tntHsttb && !FStretchSttbCb(h, cMax))
					goto LReturn;
			break;
			}

	fReturn = fTrue;

/* reset state in preperation for !fPlan phase */
	pxbc->ixsMac = 1; /* reset to first xs */

LReturn:
/* failure or success - don't need to handle this prm special anymore */
	vmerr.prmDontReclaim = prmNil;

	return fReturn;
}


#endif /* DEBUG */

#ifdef DEBUG /* in editn.asm if WIN */
/* C L O S E  T N S */
/* Close up unneeded space in each hplc or hsttb used by pxbc. */
/*  %%Function:CloseTns %%Owner:peterj %%reviewed: 6/28/89 */
HANDNATIVE C_CloseTns(pxbc)
struct XBC *pxbc;
{
	struct TN *ptn, *ptnMac;

	Assert(!vfInCommit);
	for (ptn = pxbc->rgtn, ptnMac = ptn + pxbc->itnMac; ptn < ptnMac; ptn++)
		switch (ptn->tnt)
			{
#ifdef DEBUG
		case tntAbort:
		default:
			Assert(fFalse);
		case tntNil:
			break;
#endif /* DEBUG */
		case tntHplc:
			CloseUpPlc(ptn->h);
			break;
		case tntHsttb:
			CloseUpSttb(ptn->h);
			break;
		case tntCloseHdrWw:
			DisposeWwHdr(ptn->h);
			break;
		case tntDelBkmk:
				/* if all bookmarks were deleted, nuke structs */
				{
				struct DOD *pdod = PdodDoc((int)ptn->h/*doc*/);
				FreezeHp();
				Assert(!pdod->fShort);
				if (pdod->hplcbkf != hNil && IMacPlc(pdod->hplcbkf) == 0)
					CloseBkmkStructs(pdod);
				MeltHp();
				break;
				}
			}
}


#endif /* DEBUG */

/* C L O S E  B K M K  S T R U C T S */
/*  %%Function:CloseBkmkStructs %%Owner:peterj %%reviewed: 6/28/89 */
EXPORT CloseBkmkStructs(pdod)
struct DOD *pdod;
{

	FreePhsttb(&pdod->hsttbBkmk);
	FreePhplc(&pdod->hplcbkf);
	FreePhplc(&pdod->hplcbkl);
}


/* F D E L E T E */
/* delete everything in *pca */
/*  %%Function:FDelete %%Owner:peterj %%reviewed: 6/28/89  */
FDelete(pca)
struct CA *pca;
{
	return DcpCa(pca) == 0 || FReplace(pca, fnNil, fc0, fc0);
}


#ifdef DEBUG /* in editn.asm if WIN */
/* F  R E P L A C E */
/* Replace *pca by fc through (fc+dfc-1) in fn.  returns fFalse if not
successful (in which case document is not changed). */
/*  %%Function:FReplace %%Owner:peterj %%reviewed: 6/28/89  */
HANDNATIVE BOOL C_FReplace(pca, fn, fc, dfc)
struct CA *pca;
int fn;
FC fc, dfc;
{
	struct XBC xbc;
	struct XSR *pxsr;
/* this should be the maximum number of calls to XReplace or XReplaceCps that
can result from a call to FReplace (if it is too low, the assert in PxsAlloc
should go off) */
#define ixsrReplaceMax 4
	struct XSR rgxsr[ixsrReplaceMax];

	if (vrulss.caRulerSprm.doc != docNil)
		FlushRulerSprms();

	pxsr = (struct XSR *)PxsInit(rgxsr, ixsrReplaceMax, &xbc);
	pxsr->pcaDel = pca;
	pxsr->fn = fn;
	pxsr->fc = fc;
	pxsr->dfc = dfc;
	XReplace(fTrue, &xbc, pxsr);
	if (!FDoTns(&xbc))
		{
		SetErrorMat(matReplace);
		return fFalse;
		}
	BeginCommit();
	XReplace(fFalse, &xbc, pxsr);
	EndCommit();
	CloseTns(&xbc);

	return fTrue;
}


#endif /* DEBUG */

#ifdef DEBUG /* in editn.asm if WIN */
/* X  R E P L A C E */
/* Perform a bifuricated replacement.  arguments are in pxsr */
/*  %%Function:XReplace %%Owner:peterj %%reviewed: 6/28/89 */
HANDNATIVE C_XReplace(fPlan, pxbc, pxsr)
BOOL fPlan;
struct XBC *pxbc;
struct XSR *pxsr;
{
	struct CA *pca = pxsr->pcaDel;

	Assert(pca->cpFirst >= cp0 && pca->cpLim <= CpMac1Doc(pca->doc));
#ifdef WIN
	/*  check that deleted portion may be deleted WRT fields */
	AssertSz(!vdbs.fCkFldDel || 
			FCkFldForDelete(pca->doc, pca->cpFirst, pca->cpLim), 
			"Attempt to delete unmatched field char! " );
#endif

	if (DcpCa(pca) != 0)
/* delete structures for text being deleted */
		XDeleteStruct(fPlan, pxbc, pxsr);

#ifdef WIN
	if (!fPlan && !vfNoInval)
		/* may call CachePara or FetchCp, must call BEFORE changing piece tbl */
		InvalText (pca, fTrue /* fEdit */);
#endif /* WIN */

	XRepl1(fPlan, pxbc, pxsr);

/* invalidate (only needed if !fPlan) */
	if (!fPlan)
		{
		if (!vfNoInval)
			InvalCp1(pca);
		else
			/* inval the caches even if vfNoInval on */
			InvalCaFierce();

		AdjustCp(pca, pxsr->dfc);
		}
}


#endif /* DEBUG */


#ifdef DEBUG /* in editn.asm if WIN */
/* F  R E P L 1 */
/* delete pca and insert the specified piece (does not do checking,
adjustment or invalidation).  returns fFalse on failure (in which case
document is unchanged). */
/*  %%Function:FRepl1 %%Owner:peterj %%reviewed: 6/28/89 */
HANDNATIVE BOOL C_FRepl1(pca, fn, fc, dfc)
struct CA *pca;
int fn;
FC fc, dfc;
{
	struct XBC xbc;
	struct XSR *pxsr;
/* this should be the maximum number of calls to XReplace or XReplaceCps that
can result from a call to FRepl1 (if it is too low, the assert in PxsAlloc
should go off) */
#define ixsrRepl1Max 1
	struct XSR rgxsr[ixsrRepl1Max];

	pxsr = (struct XSR *)PxsInit(rgxsr, ixsrRepl1Max, &xbc);
	pxsr->pcaDel = pca;
	pxsr->fn = fn;
	pxsr->fc = fc;
	pxsr->dfc = dfc;
	XRepl1(fTrue, &xbc, pxsr);
	if (!FDoTns(&xbc))
		{
		SetErrorMat(matReplace);
		return fFalse;
		}
	BeginCommit();
	XRepl1(fFalse, &xbc, pxsr);
	EndCommit();
	CloseTns(&xbc);

	return fTrue;
}


#endif /* DEBUG */


#ifdef DEBUG /* in editn.asm if WIN */
/* I P C D  S P L I T */
/* Ensure cp is the beginning of a piece. Return index of that piece. */
/*  %%Function:IpcdSplit %%Owner:peterj %%reviewed: 6/28/89  */
HANDNATIVE int C_IpcdSplit(hplcpcd, cp)
struct PLC **hplcpcd;
CP cp;
{
	int ipcd;
	struct PCD pcd;
	CP dcp;

	vdocFetch = docNil;	/* ensure fetch cache isn't lying */
	if ((ipcd = IInPlcCheck(hplcpcd, cp)) == -1)
		return(IMacPlc(hplcpcd));
	if ((dcp = cp - CpPlc(hplcpcd, ipcd)) != cp0)
		{
		Assert(!vfInCommit);
/* Insert a new piece flush with the one at ipcd */
		if (!FOpenPlc(hplcpcd, ++ipcd, 1))
			{
			SetErrorMat( matReplace );
			return iNil;
			}
		PutCpPlc(hplcpcd, ipcd, cp);
		/* We are doing effectively:
			pcd.fn = pcdPrev.fn;
			pcd.fc = pcdPrev.fc + dcp;
			pcd.prm = pcdPrev.prm;
			pcd.fNoParaLastValid = pcdPrev.fNoParaLast;
			pcd.fNoParaLast = pcdPrev.fNoParaLast;
		*/
		GetPlc(hplcpcd, ipcd - 1, &pcd);
		pcd.fc += dcp;
		PutPlc(hplcpcd, ipcd, &pcd);
		}
	return ipcd;
}


#endif /* DEBUG */


#ifdef DEBUG /* in editn.asm if WIN */
/* X  R E P L 1 */
/* perform Bifuricated replacement.  arguments are in pxsr */
/*  %%Function:XRepl1 %%Owner:peterj %%reviewed: 6/28/89  */
HANDNATIVE C_XRepl1(fPlan, pxbc, pxsr)
BOOL fPlan;
struct XBC *pxbc;
struct XSR *pxsr;
{
	struct PLC **hplcpcd;
	int ipcdFirst, ipcdLim;
	int cpcd;
	struct PCD pcd;
	struct PCD pcdPrev;

	hplcpcd = PdodDoc(pxsr->pcaDel->doc)->hplcpcd;

	if (fPlan)
		{
		pxsr->fNotEmptyPcaDel = (pxsr->pcaDel->cpFirst != pxsr->pcaDel->cpLim);

		if ((ipcdFirst = IpcdSplit(hplcpcd, pxsr->pcaDel->cpFirst))
				== iNil)
			{
LPostAbort:
			PostTn(pxbc, tntAbort, NULL, 0);
			return;
			}

		if (!pxsr->fNotEmptyPcaDel)
			cpcd = 0;
		else
			{
			int ipcdLim;
			if ((ipcdLim = IpcdSplit(hplcpcd, pxsr->pcaDel->cpLim))
					== iNil)
				goto LPostAbort;
			cpcd = ipcdFirst - ipcdLim;
			}
		pxsr->cpcd = cpcd;
		}
	else
		{
		/* ipcd not stored because recursive calls can cause it to change */
		ipcdFirst = IpcdSplit(hplcpcd, pxsr->pcaDel->cpFirst);
		cpcd = pxsr->cpcd;
		/* make sure that the range to be deleted is already split out */
		Assert(!pxsr->fNotEmptyPcaDel ||
				IInPlc(hplcpcd, pxsr->pcaDel->cpLim) == ipcdFirst - cpcd);
		/* set so vhprc chain is checked when we run out of memory */
		vmerr.fReclaimHprcs = fTrue;
		}

	/* number of pieces to be added (negative or zero) */
	Assert(cpcd <= 0);

	if (fPlan)
		/* simplified, may be one less */
		PostTn(pxbc, tntHplc, hplcpcd, cpcd+1);

	else
		{
		if (ipcdFirst > 0)
			GetPlc(hplcpcd, ipcdFirst - 1, &pcdPrev);

		if (pxsr->dfc == fc0 ||
				(ipcdFirst > 0 && pcdPrev.fn == pxsr->fn && 
				pcdPrev.prm == prmNil && pcdPrev.fc + 
				(pxsr->pcaDel->cpFirst - CpPlc(hplcpcd,ipcdFirst-1))
				== pxsr->fc))
			/* Either pure delete or extension of previous piece */
			{
			FOpenPlc(hplcpcd, ipcdFirst, cpcd);
			if (pxsr->dfc != fc0)
				/* If extending, say we might have inserted Eop*/
				{
				Debug(pcdPrev.fNoParaLastValid = fFalse);
				pcdPrev.fNoParaLast = fFalse;
				PutPlc(hplcpcd, ipcdFirst - 1, &pcdPrev);
				}
			}
		else
			/* Insert one piece */
			{
			AssertDo(FOpenPlc(hplcpcd, ipcdFirst, cpcd + 1));
			GetPlc(hplcpcd, ipcdFirst, &pcd);
			PutCpPlc(hplcpcd, ipcdFirst, pxsr->pcaDel->cpFirst);
			pcd.fn = pxsr->fn;
			pcd.fc = pxsr->fc;
			pcd.prm = prmNil;
			Debug(pcd.fNoParaLastValid = fFalse);
			pcd.fNoParaLast = fFalse; /* Para state unknown */
			pcd.fPaphNil = fFalse;
			PutPlc(hplcpcd, ipcdFirst, &pcd);
			ipcdFirst++;
			}
		AdjustHplc(hplcpcd, pxsr->pcaDel->cpLim, pxsr->dfc-pxsr->pcaDel->cpLim + 
				pxsr->pcaDel->cpFirst, ipcdFirst);
#ifdef WIN
		InvalVisiCache();
		InvalCellCache();
#endif /* WIN */
		}
}


#endif /* DEBUG */

#ifdef DEBUG
/* X  D E L E T E  S T R U C T */
/* Delete structures from the deletion range */
/* %%Function:XDeleteStruct %%owner:peterj %%reviewed: 6/28/89 */
XDeleteStruct(fPlan, pxbc, pxsr)
BOOL fPlan;
struct XBC *pxbc;
struct XSR *pxsr;
{
	struct CA *pca = pxsr->pcaDel;

/* check for deleting para and sect boundaries; delete entries from parallel
structures */
	if (!fPlan && !vfNoInval)
		InvalParaSect(pca, pca, fTrue);
#ifdef WIN
	if (FFieldsInPca(pca))
		XDeleteFields(fPlan, pxbc, &pxsr->xslDelete, pca);

	if (!PdodDoc(pca->doc)->fShort)
		if (PdodDoc(pca->doc)->hsttbBkmk != hNil)
			XDeleteBkmks(fPlan, pxbc, pca, fFalse);
#endif
	XDelFndSedPgdPad(fPlan, pxbc, pxsr);
}


#endif /* DEBUG */


/* F  R E P L A C E  C P S  R M */
/* General replace routine -- takes revision marking into account */
/* adjusts pcaDel to what's left after the delete. */
/*  %%Function:FReplaceCpsRM %%Owner:peterj %%reviewed: 6/28/89  */
BOOL FReplaceCpsRM(pcaDel, pcaIns)
struct CA *pcaDel, *pcaIns;
{
	struct CA caT;
	char grpprl[2];

	if (vmerr.fMemFail)
		return fFalse;

#ifdef WIN
	if (PdodMother(pcaDel->doc)->dop.fRevMarking)
		{
		AssureLegalSel( pcaDel );
		if (DcpCa(pcaDel) != cp0)
			{
			if (!FRemoveRevMarking(pcaDel, rrmDelete, fFalse /* fSubDocs */))
				return fFalse;
			}
		if (DcpCa(pcaIns) != cp0)
			{
			caT = *pcaDel;
			caT.cpLim = caT.cpFirst;
			if (!FReplaceCps(&caT, pcaIns))
				return fFalse;
			caT.cpLim = caT.cpFirst + DcpCa(pcaIns);
			grpprl[0] = sprmCFRMark;
			grpprl[1] = TRUE;
			ApplyGrpprlCa(grpprl, sizeof(grpprl), &caT);
			}
		}
	else
#endif
			{
			if (!FReplaceCps(pcaDel, pcaIns))
				return fFalse;
			pcaDel->cpLim = pcaDel->cpFirst;
			}
	return !vmerr.fMemFail;
}


#ifdef DEBUG /* in editn.asm if WIN */
/* F  R E P L A C E  C P S */
/* General replace routine */
/* Replace characters from pcaDel with characters from pcaIns.  returns fFalse
on failure (in which case document is unchanged) */
/*  %%Function:FReplaceCps %%Owner:peterj %%reviewed: 6/28/89 */
HANDNATIVE BOOL C_FReplaceCps(pcaDel, pcaIns)
struct CA *pcaDel, *pcaIns;
{
	struct XBC xbc;
	struct XSR *pxsr;
/* this should be the maximum number of calls to XReplace or XReplaceCps that
can result from a call to FReplaceCps (if it is too low, the assert in PxsAlloc
should go off).  Win is greater than Mac because of annotations. */
#define ixsrReplaceCpsMax WinMac(10,8)
	struct XSR rgxsr[ixsrReplaceCpsMax];

	Assert(pcaDel->cpFirst >= cp0 && pcaDel->cpLim <= CpMac1Doc(pcaDel->doc));
	Assert(DcpCa(pcaDel) >= 0 && DcpCa(pcaIns) >= cp0);

	if (vrulss.caRulerSprm.doc != docNil)
		FlushRulerSprms();

	pxsr = (struct XSR *)PxsInit(rgxsr, ixsrReplaceCpsMax, &xbc);
	pxsr->pcaDel = pcaDel;
	pxsr->pcaIns = pcaIns;
	XReplaceCps(fTrue, &xbc, pxsr);
	if (!FDoTns(&xbc))
		{
		SetErrorMat(matReplace);
		return fFalse;
		}
	BeginCommit();
	XReplaceCps(fFalse, &xbc, pxsr);
	EndCommit();
	CloseTns(&xbc);

	return fTrue;
}


#endif /* DEBUG */


#ifdef DEBUG /* in editn.asm if WIN */
/* X  R E P L A C E  C P S */
/* perform bifuricated replacement.  arguments are in pxsr */
/*  %%Function:XReplaceCps %%Owner:peterj %%reviewed: 6/28/89 */
HANDNATIVE C_XReplaceCps(fPlan, pxbc, pxsr)
BOOL fPlan;
struct XBC *pxbc;
struct XSR *pxsr;
{
	struct PLC **hplcpcdDest, **hplcpcdSrc;
	int ipcdFirst, ipcdInsFirst, ipcdLim;
	int cpcd;
	struct CA *pcaDel = pxsr->pcaDel;
	struct CA *pcaIns = pxsr->pcaIns;
	int docDel = pcaDel->doc;
	int docIns = pcaIns->doc;
	CP dcpDel = DcpCa(pcaDel);
	CP dcpIns = DcpCa(pcaIns);
	struct DOD **hdodSrc = mpdochdod[docIns];
	struct DOD **hdodDest = mpdochdod[docDel];
	struct PCD pcd;

#ifdef DEBUG
	if (fPlan)
		/* no point doing all this twice! */
		{
		Assert(pcaDel->cpFirst >= cp0 && pcaDel->cpLim <= CpMac1Doc(docDel));
		Assert(dcpDel >= 0 && dcpIns >= cp0);
		Assert(docDel != docIns);
		/* assured by caller */
		Assert(vrulss.caRulerSprm.doc == docNil);
#ifdef WIN
		/* assure that if vdocScratch is being used, it has been "Acquired" */
		Assert ((vdocScratch == docNil || (docDel != vdocScratch &&
				docIns != vdocScratch) || fDocScratchInUse));
		/*  check that deleted portion is legal WRT fields */
		AssertSz(!vdbs.fCkFldDel || FCkFldForDelete(docDel, pcaDel->cpFirst, pcaDel->cpLim)
				&& FCkFldForDelete(pcaIns->doc, pcaIns->cpFirst, pcaIns->cpLim),
				"Attempt to delete unmatched field char!");
#endif
		}
#endif /* DEBUG */

	if (dcpIns == 0)
		/* This is just too easy . . . */
		{
		pxsr->fn = fnNil;
		pxsr->fc = fc0;
		pxsr->dfc = fc0;
		XReplace(fPlan, pxbc, pxsr);
		return;
		}

	hplcpcdDest = (*hdodDest)->hplcpcd;
	hplcpcdSrc = (*hdodSrc)->hplcpcd;

	if (dcpDel != cp0)
/* delete structures for text being deleted */
		XDeleteStruct(fPlan, pxbc, pxsr);

	if (!fPlan && !vfNoInval)
		InvalParaSect(pcaDel, pcaIns, fFalse);

/* Get the first and last pieces for insertion */
	if (fPlan)
		{
		ipcdInsFirst = pxsr->ipcdInsFirst = 
				IInPlc(hplcpcdSrc, pcaIns->cpFirst);
		pxsr->ipcdInsLast = IInPlc(hplcpcdSrc, pcaIns->cpLim - 1);
		pxsr->fNotEmptyPcaDel = (dcpDel != 0);
		}
	else
		ipcdInsFirst = pxsr->ipcdInsFirst;

/* get the limiting pieces for deletion */
	if (fPlan)
		{
		ipcdFirst = IpcdSplit(hplcpcdDest, pcaDel->cpFirst);
		ipcdLim = (pxsr->fNotEmptyPcaDel) ?
				IpcdSplit(hplcpcdDest, pcaDel->cpLim) : ipcdFirst;

/* check for failure of IpcdSplit */
		if (ipcdFirst == iNil || ipcdLim == iNil)
			{
			PostTn(pxbc, tntAbort, NULL, 0);
			return;
			}

		/* number of pieces to be added */
		pxsr->cpcd = cpcd = ipcdFirst - ipcdLim + pxsr->ipcdInsLast - ipcdInsFirst +1;
		}
	else
		{
		/* ipcd not stored because recursive calls can cause it to change */
		ipcdFirst = IpcdSplit(hplcpcdDest, pcaDel->cpFirst);
		cpcd = pxsr->cpcd;
		/* make sure that the range to be deleted is already split out */
		Assert(cpcd == ipcdFirst + pxsr->ipcdInsLast - ipcdInsFirst + 1
				- ((pxsr->fNotEmptyPcaDel) ?
				IpcdSplit(hplcpcdDest, pcaDel->cpLim) : ipcdFirst));
		}

	if (fPlan)
		PostTn(pxbc, tntHplc, hplcpcdDest, cpcd);

	else
		{
#ifdef WIN
		if (!vfNoInval)
			/* may call CachePara or FetchCp, must call BEFORE changing piece tbl */
			InvalText (pcaDel, fTrue /* fEdit */);
#endif /* WIN */

/* set so vhprc chain is checked when we run out of memory */
		vmerr.fReclaimHprcs = fTrue;

/* adjust pctb size; get pointer to the first new piece, ppcdDest, and to the
first piece we are inserting. */
		AssertDo(FOpenPlc(hplcpcdDest, ipcdFirst, cpcd));

		FreezeHp();
/* ensure rgcp in hplcpcdSrc is adjusted before we copy cps. */
		if (((struct PLC *)*hplcpcdSrc)->icpAdjust < ipcdInsFirst + 1)
			AdjustHplcCpsToLim(hplcpcdSrc, ipcdInsFirst + 1);

/* fill first new piece and split it appropriately */
		GetPlc(hplcpcdSrc, ipcdInsFirst, &pcd);
		pcd.fc += (pcaIns->cpFirst - CpPlc(hplcpcdSrc, ipcdInsFirst));
		PutPlc(hplcpcdDest, ipcdFirst, &pcd);
		PutCpPlc(hplcpcdDest, ipcdFirst, pcaDel->cpFirst);

		ipcdLim = ipcdFirst + 1;
		if ((cpcd = pxsr->ipcdInsLast - ipcdInsFirst) != 0)
			{
/* fill in rest of inserted pieces */
			ipcdLim += cpcd;
			CopyMultPlc(cpcd, hplcpcdSrc, ipcdInsFirst + 1,
					hplcpcdDest, ipcdFirst + 1,
					pcaDel->cpFirst - pcaIns->cpFirst, 0, 0);
			}

/* adjust rest of pieces in destination doc */
		AdjustHplc(hplcpcdDest, pcaDel->cpLim, /*dcpAdj*/dcpIns - dcpDel,
				ipcdLim);

/* and inform anyone else who cares */

		(*hdodDest)->fFormatted |= (pcaIns->doc == docScrap) ? 
				vsab.fFormatted : (*hdodSrc)->fFormatted;
		PdodMother(docDel)->fMayHavePic |= (docIns == docScrap) ? 
				vsab.fMayHavePic : PdodMother(docIns)->fMayHavePic;
		if (!vfNoInval)
			InvalCp1(pcaDel);
		else
			/* inval the caches even if vfNoInval on */
			InvalCaFierce();

#ifdef WIN
		/* invalidate FetchCpPccpVisible */
		InvalVisiCache();
		InvalCellCache();
#endif /* WIN */

		MeltHp();

		AdjustCp(pcaDel, dcpIns);
/* NOTE after this point pcaDel->cpLim may be untrustworthy because it may
	have been adjusted as a side effect of AdjustCp (eg. selCur.ca) */
		}


/* copy enclosed structures and subdocs */

#ifdef WIN
/*  copy any enclosed fields */
	if (FFieldsInPca(pcaIns))
		XCopyFields(fPlan, pxbc, pcaIns, pcaDel);
#endif

/* page table: if there is a table to be updated, call routine. Even if
the source table is empty, the destination will have to be invalidated. */
	if ((*hdodDest)->hplcpgd)
		{
		XAddToHplcEdit(fPlan, pxbc, &pxsr->xsaPgdCopy, pcaIns, pcaDel, 
				edcPgd);
		InvalLlc();
		}

	if (!(*hdodSrc)->fShort && !(*hdodDest)->fShort)
		{
#ifdef WIN
/*  copy any bookmarks in the source document which are fully enclosed
	in the insertion range to the destination document */
		if ((*hdodSrc)->hsttbBkmk != hNil)
			XCopyBkmks(fPlan, pxbc, &pxsr->xsbCopy, pcaIns, pcaDel);

/* copy any anotations along with their reference marks */
		if ((*hdodSrc)->docAtn != docNil)
			XAddReferencedText(fPlan, pxbc, &pxsr->xsfAtnCopy, pcaIns, pcaDel,
					edcDrpAtn);
#endif

/* copy any footnotes along with their reference marks */
		if ((*hdodSrc)->docFtn != docNil)
			XAddReferencedText(fPlan, pxbc, &pxsr->xsfFtnCopy, pcaIns, pcaDel,
					edcDrpFtn);

/* if there are any sections call AddHplcEdit to copy entries from
one hplcsed to the other */
		if ((*hdodSrc)->hplcsed)
			{
			caSect.doc = docNil;
			if ((*hdodSrc)->docHdr != docNil || (*hdodDest)->docHdr != docNil)
				{
				XAddHdrText(fPlan, pxbc, pcaIns, pcaDel);
				caSect.doc = docNil;    /* XAdd.. called CacheSect */
				}

			XAddToHplcEdit(fPlan, pxbc, &pxsr->xsaSedCopy, pcaIns, pcaDel, 
					edcSed);
			InvalLlc();
			}
		if ((*hdodSrc)->fSea && (*hdodSrc)->hplcsea)
			XAddToHplcEdit(fPlan, pxbc, &pxsr->xsaSeaCopy, pcaIns, pcaDel,
					edcSea);
/* outline table: as for page table */
		if (fPlan)
			pxsr->fNotDelPassCpMac = (pcaDel->cpFirst < (CpMacDoc(docDel)+dcpIns-dcpDel));
		if (((*hdodDest)->hplcpad || (*hdodSrc)->hplcpad) &&
				pxsr->fNotDelPassCpMac && (*hdodDest)->dk != dkGlsy)
			{
			XAddToHplcEdit(fPlan, pxbc, &pxsr->xsaPadCopy, pcaIns, pcaDel,
					edcPad);
			}

/* height table */
		if ((*hdodDest)->hplcphe)
			XAddToHplcEdit(fPlan, pxbc, &pxsr->xsaPheCopy, pcaIns, pcaDel,
					edcPhe);

/* we will replace docDest's hidden section character with the hidden section
	mark from docSrc only if we are not already copying the section mark,
	we are copying the tail of docIns to the tail of docDel, the text
	copied from docIns does not end with a section mark, and if docSrc is
	guarded (ie. == docScrap or docUndo) the hidden section character had to
	have been copied from the original document. */

		if (fPlan)
			{
			CP cpTailIns = CpTail(docIns);
			struct DOD *pdodIns;
/* Note that no cps have been adjusted yet */
			pxsr->fReplHidnSect = 
					pcaDel->cpFirst + dcpDel <= CpMacDoc(docDel) &&
					pcaIns->cpFirst < CpMacDoc(docIns) &&
					pcaIns->cpLim >= cpTailIns &&
					!FSectLimAtCp(docIns, cpTailIns) &&
					pcaDel->cpLim >= CpTail(docDel) - 
					((!PdodDoc(docDel)->fGuarded) ? ccpEop : cp0) &&
					(!(pdodIns=PdodDoc(docIns))->fGuarded ||
					(pdodIns->fSedMacEopCopied && (dcpDel > cp0 || dcpIns == CpMacDocEdit(docIns))));
			}
		if (pxsr->fReplHidnSect)
			{
			struct XSR *pxsrT = PxsAlloc(pxbc);
			struct CA caDel, caIns;
			CheckPxs(fPlan, pxsrT, nxsHidnSect, 0);
			pxsrT->pcaDel = PcaSetDcp(&caDel, docDel, CpMac1Doc(docDel)-ccpEop, 
					ccpEop);
			pxsrT->pcaIns = PcaSetDcp(&caIns, docIns, CpMac1Doc(docIns)-ccpEop,
					ccpEop);
			XReplaceCps(fPlan, pxbc, pxsrT);
			if (!fPlan && PdodDoc(docDel)->fGuarded)
				PdodDoc(docDel)->fSedMacEopCopied = fTrue;
			}
		}
}


#endif /* DEBUG */


#ifdef DEBUG
/* X  D E L  F N D  S E D  P G D  P A D */
/* Delete all footnote/annotation text corresponding to refs in pcaDel
Also delete SED's for section marks and invalidate PGD's in the page table.
*/
/*  %%Function:XDelFndSedPgdPad %%Owner:peterj %%reviewed: 6/28/89 */
HANDNATIVE C_XDelFndSedPgdPad(fPlan, pxbc, pxsr)
BOOL fPlan;
struct XBC *pxbc;
struct XSR *pxsr;
{
	struct PLC **hplc;
	struct DOD **hdod;
	struct CA caT;

	caT = *pxsr->pcaDel;
	hdod = mpdochdod[caT.doc];

/* FUTURE: why does this have to be done here?  Can it be done below with the
rest of the !fShort processing? */
	if (!(*hdod)->fShort)
		if ((hplc = (*hdod)->hplcsed) != 0)
			{
			if ((*hdod)->docHdr != docNil)
				XDeleteHdrText(fPlan, pxbc, &pxsr->xsp, pxsr->pcaDel);
			XDelInHplcEdit(fPlan, pxbc, NULL, pxsr->pcaDel, hplc, 
					edcNone);
			InvalLlc();
			}

/* protect PLCs from lookups with cp > cpMac */
	Assert(caT.cpLim <= CpMac2Doc(caT.doc));
	caT.cpLim = CpMin(caT.cpLim, CpMac2Doc(caT.doc));
	if (caT.cpLim <= caT.cpFirst)
		return;

/* these PLCs are in short and long docs */
	if ((hplc = (*hdod)->hplcpgd) != 0)
		{
		XDelInHplcEdit(fPlan, pxbc, &pxsr->xsaPgdDel, &caT, hplc, edcPgd);
		InvalLlc();
		}
	if ((hplc = (*hdod)->hplcphe) != 0)
		XDelInHplcEdit(fPlan, pxbc, &pxsr->xsaPheDel, &caT, hplc, edcPhe);

	if ((*hdod)->fShort)
		return;
/* PLCs for long docs only */

	if ((*hdod)->docFtn != docNil)
		XDelReferencedText(fPlan, pxbc, &pxsr->xsfFtnDel, &caT, edcDrpFtn);
#ifdef WIN
	if ((*hdod)->docAtn != docNil)
		XDelReferencedText(fPlan, pxbc, &pxsr->xsfAtnDel, &caT, edcDrpAtn);
#endif /* WIN */

	if ((hplc = (*hdod)->hplcpad) != 0 && caT.cpFirst < CpMacDoc(caT.doc))
		XDelInHplcEdit(fPlan, pxbc, &pxsr->xsaPadDel, &caT, hplc, edcPad);
	if ((*hdod)->fSea && (hplc = (*hdod)->hplcsea) != 0)
		XDelInHplcEdit(fPlan, pxbc, &pxsr->xsaSeaDel, &caT, hplc, edcSea);
}


#endif /* DEBUG */

/* X  A D D  T O  H P L C  E D I T */
/* inserts at Dest the section (paragraph, etc..) bounds found in the Src */
/*  %%Function:XAddToHplcEdit %%Owner:peterj %%reviewed: 6/28/89 */
EXPORT XAddToHplcEdit(fPlan, pxbc, pxsa, pcaSrc, pcaDest, edc)
BOOL fPlan;
struct XBC *pxbc;
struct XSA *pxsa;
struct CA *pcaSrc, *pcaDest;
int edc;
{
	struct PLC **hplcSrc;
	struct PLC **hplcDest;
	struct DOD **hdodDest = mpdochdod[pcaDest->doc];
	int ipgd, cIns, iFirst;
	CP cpDest = pcaDest->cpFirst;
	CP cpFirst;
	union
		{
		struct PGD pgd;
		struct PAD pad;
		struct PHE phe;
		struct SED sed;
		} foo;

/* source and destination tables */
	hplcDest = *((int *)(*hdodDest) + edc);
	hplcSrc = *((int *)PdodDoc(pcaSrc->doc) + edc);

	cpDest = CpMin(cpDest, CpMacDoc(pcaDest->doc));

	if (hplcSrc == 0 || edc == edcPgd)
		{
		Assert(hplcDest);
		if (!fPlan)
			{
/* entry at iDest will become unknown. fUnk bit will be in the same place
in all relevant structures: PGD, PAD
*/
			if (edc != edcSea)
				{
				if ((ipgd=IInPlcMult(hplcDest, cpDest)) >= IMacPlc(hplcDest))
					return;
				GetPlc(hplcDest, ipgd, &foo);
				foo.pad.fUnk = fTrue;
				PutPlcLast(hplcDest, ipgd, &foo);
				}
			if (edc == edcPad)
				(*hdodDest)->fOutlineDirty = fTrue;
			}
		return;
		}

	cpFirst = pcaSrc->cpFirst;

	if (fPlan)
		{
		CP cpLim;

		cpLim = CpMin(pcaSrc->cpLim, ((edc != edcSed) ?
				CpMacDoc(pcaSrc->doc) : CpMac1Doc(pcaSrc->doc)));

		iFirst = pxsa->iFirst = IInPlcMult(hplcSrc, cpFirst);
		pxsa->c = IInPlc2Mult(hplcSrc, cpLim, iFirst) - iFirst;
		}
	else
		iFirst = pxsa->iFirst;

	if ((cIns = pxsa->c) == 0)
		return;

	if (fPlan)
		{
/* assure we have a plc then plan for space */
		if (hplcDest == hNil)
			{
			if ((hplcDest = HplcCreateEdc(hdodDest, edc)) == hNil)
				{
				PostTn(pxbc, tntAbort, NULL, 0);
				return;
				}

			/* this is now guaranteed (all long-dod docs always have a sed) */
			Assert(edc != edcSed);
#ifdef DISABLE_PJ
			/* delete the bogus SED entry added by HplcCreateEdc */
			if (edc == edcSed)
				FOpenPlc(hplcDest, 1, -1);
#endif /* DISABLE_PJ */
			}
		PostTn(pxbc, tntHplc, hplcDest, cIns);
		}

	AssertH(hplcDest);

	if (!fPlan)
		{
		int iDest = IInPlcMult(hplcDest, cpDest);
		AssertDo(FOpenPlc(hplcDest, iDest + 1, cIns));
		CopyMultPlc(1, hplcDest, iDest, hplcDest, iDest + cIns, cp0, 0, 0);
		CopyMultPlc(cIns, hplcSrc, iFirst, hplcDest, iDest, cpDest - cpFirst,
				1, 0);

		if (edc == edcPhe)
			{
/* PHE update: entry at iDest will become invalid */
			GetPlc(hplcDest, iDest, &foo);
			foo.phe.fUnk = fTrue;
			PutPlcLast(hplcDest, iDest, &foo.phe);
/* last entry inserted will be invalid */
			if (cIns > 1)
				{
				GetPlc(hplcDest, iDest + cIns - 1, &foo);
				foo.phe.fUnk = fTrue;
				PutPlcLast(hplcDest, iDest + cIns - 1, &foo);
				}
			}
		}
}


/* X  D E L  I N  H P L C  E D I T */
/* removes bounds in the plc. cp is removed from plc iff
cpFirst < cp <= cpLim, since cp's point to beginnings of sections (pages, etc.)
Returns iFirst into plc.
*/
/*  %%Function:XDelInHplcEdit %%Owner:peterj %%reviewed: 6/28/89 */
EXPORT XDelInHplcEdit(fPlan, pxbc, pxsa, pca, hplc, edc)
BOOL fPlan;
struct XBC *pxbc;
struct XSA *pxsa;
struct CA *pca;
struct PLC **hplc;
int edc;
{
	int iFirst, cDel, iMacPlc;
	union /* NOTE: pgd and phe fUnQk must be coincident */
		{
		struct PGD pgd;
		struct PHE phe;
		} pxx;

	Debug(cDel = 0x8000);

	Assert(pca->cpFirst < pca->cpLim);
	AssertH(hplc);

	if ((iMacPlc = IMacPlc(hplc)) == 0)
		return;

	if (fPlan || pxsa == NULL)
		{
		if ((iFirst = IInPlcMult(hplc, pca->cpFirst)) < iMacPlc)
			cDel = IInPlc2Mult(hplc, pca->cpLim, iFirst) - iFirst;
		if (pxsa != NULL)
			{
			pxsa->iFirst = iFirst;
			pxsa->c = cDel;
			}
		}
	else
		{
		iFirst = pxsa->iFirst;
		cDel = pxsa->c;
		}

	if (iFirst >= iMacPlc)
		return;

	/* there is a potential path where cDel is unitialized. assert we can
		never follow that path. */
	Assert(cDel != 0x8000);

	if (cDel != 0)
		{
/* Close up plc */
/* delete cp's starting at iFirst + 1, delete entries starting at iFirst.
OpenPlc deletes them starting at the same place, so we have to save/restore
the unadjusted cp at iFirst, except at end of plc (rgcp[iMac] should remain
undisturbed) */
		if (fPlan)
			PostTn(pxbc, tntHplc, hplc, -cDel);
		else
			{
			CP cpT = CpPlc(hplc, iFirst);
			if (edc == edcPgd && iFirst > 0)
				{
/* we are invalidating the page before the page boundary that was deleted.
deleting a page boundary casts doubt on the extend and layout of the previous
page.  for example layout of previous page is blown if we remove a paragraph
with Page Break Before set to true. */
				GetPlc(hplc, iFirst - 1, &pxx);
				pxx.pgd.fUnk = fTrue;
				PutPlcLast(hplc, iFirst - 1, &pxx);
				}
			FOpenPlc(hplc, iFirst, -cDel);
			if (iFirst != IMacPlc(hplc))
				PutCpPlc(hplc, iFirst, cpT);
			}
		}
/* in the page table, invalidate the page where the deletion starts */
/* in the height table, invalidate height where the deletion starts and ends*/
	if (!fPlan && (edc == edcPgd || edc == edcPhe) && iFirst < IMacPlc(hplc))
		{
		if (edc == edcPgd)
			AdjustIpgds(pca->doc, iFirst, cDel, fTrue);
		GetPlc(hplc, iFirst, &pxx);
		pxx.pgd.fUnk = fTrue;
		PutPlcLast(hplc, iFirst, &pxx);
		}
}


/* X  A D D  R E F E R E N C E D  T E X T */
/* Add footnote/annotation text to correspond with inserted references.
Called after inserting docSrc[cpFirst:cpLim) into docDest@cpDest
At this point we know only that the source doc has docFtn/docAtn != docNil.
*/
/*  %%Function:XAddReferencedText %%Owner:peterj %%reviewed: 6/28/89 */
EXPORT XAddReferencedText(fPlan, pxbc, pxsf, pcaSrc, pcaDest, edcDrp)
BOOL fPlan;
struct XBC *pxbc;
struct XSF *pxsf;
struct CA *pcaSrc, *pcaDest;
int edcDrp;
{
	struct DOD **hdodDest;
	struct PLC **hplcRefDest, **hplcDest, **hplcRefSrc, **hplcSrc;
	int docSubDest, docSubSrc;
	int cfoo, ifooDest;
	CP cpSubSrc, cpSubDest;
	int docDest = pcaDest->doc;
	struct CA caT1, caT2;
	struct XSR *pxsr2;

		/* BLOCK - heap pointer */
		{
		struct DRP *pdrp;
		pdrp = ((int *) PdodDoc(pcaSrc->doc)) + edcDrp;
		docSubSrc = pdrp->doc;
		if ((hplcRefSrc = pdrp->hplcRef) == hNil)
			return;
		}
	hplcSrc = PdodDoc(docSubSrc)->hplcfnd;

/* compute:
	ifooFirst: first ref included in the interval [cpFirst, cpLim)
	ifooLim: last ref in the interval + 1.
*/

	if (fPlan)
		{
		pxsf->ifooFirst = IInPlcRef(hplcRefSrc, pcaSrc->cpFirst);
		Assert(pxsf->ifooFirst >= 0);
		pxsf->ifooLim = IInPlcCheck(hplcRefSrc, pcaSrc->cpLim - 1) + 1;
		}
/* we are inserting cfoo = ifooLim-ifooFirst footnotes/annotations...
*/
	if ((cfoo = pxsf->ifooLim - pxsf->ifooFirst) <= 0)
		return;

	hdodDest = mpdochdod[docDest];

	if (fPlan && DocSubEnsure(docDest, edcDrp) == docNil)
		{
LPostAbort:
		PostTn(pxbc, tntAbort, NULL, 0);
		return;
		}

		/* BLOCK - heap pointer */
		{
		struct DRP *pdrp;
		pdrp = ((int *) (*hdodDest)) + edcDrp;
		docSubDest = pdrp->doc;
		Assert(docSubDest != docNil);
		hplcRefDest = pdrp->hplcRef;
		}
	AssertH(hplcRefDest);
	hplcDest = PdodDoc(docSubDest)->hplcfnd;
	AssertH(hplcDest);

	if (fPlan)
		{
		/* caller guarantees that hdodDest is a long-dod doc, which
			guarantees that these flags are off. */
		Assert(!(*hdodDest)->fFtn && !(*hdodDest)->fAtn);
#ifdef DISABLE_PJ
		if ((*hdodDest)->fFtn)
			{ /* Inserting refs inside footnotes? No way! */
			ErrorEid(eidInvalForFN,"AddReferencedText");
			goto LPostAbort;
			}
#ifdef WIN
		if ((*hdodDest)->fAtn)
			{ /* Inserting refs inside annotations? No way! */
			ErrorEid(eidNotValidForAnn,"AddReferencedText");
			goto LPostAbort;
			}
#endif
#endif /* DISABLE_PJ */
		PostTn(pxbc, tntHplc, hplcRefDest, cfoo);
		PostTn(pxbc, tntHplc, hplcDest, cfoo);
		}
/* Find ifoo to insert new foo's */
	ifooDest = IInPlcRef(hplcRefDest, pcaDest->cpFirst);

/* Insert new referenced text in docSub of docDest from docSubSrc
from ifooFirst to ifooLim.
*/
	pxsr2 = PxsAlloc(pxbc);
	CheckPxs(fPlan, pxsr2, nxsAddRefText, edcDrp);
	pxsr2->pcaDel = PcaPoint(&caT1, docSubDest, cpSubDest = 
			CpPlc(hplcDest,ifooDest));
	pxsr2->pcaIns = PcaSet(&caT2, docSubSrc, cpSubSrc = 
			CpPlc(hplcSrc, pxsf->ifooFirst), CpPlc(hplcSrc, pxsf->ifooLim));
	XReplaceCps(fPlan, pxbc, pxsr2);

	if (!fPlan)
		{
/* Insert new foo's in docDest from docSrc */
/* open space in destination plc */
		AssertDo(FOpenPlc(hplcRefDest, ifooDest, cfoo));
		AssertDo(FOpenPlc(hplcDest, ifooDest+1, cfoo));

		CopyMultPlc(cfoo, hplcRefSrc, pxsf->ifooFirst, hplcRefDest, ifooDest,
				pcaDest->cpFirst - pcaSrc->cpFirst, 0, 0);
		CopyMultPlc(cfoo, hplcSrc, pxsf->ifooFirst, hplcDest, ifooDest,
				cpSubDest - cpSubSrc, 1, 0);
		InvalAutoReferences(docDest, ifooDest, edcDrp);
		}
}


/* X D E L  R E F E R E N C E D  T E X T */
/* Remove the text of referenced subdocs (footnote or annotation)
	that is referenced in *pca */
/*  %%Function:XDelReferencedText %%Owner:peterj %%reviewed: 6/28/89 */
EXPORT XDelReferencedText(fPlan, pxbc, pxsf, pca, edcDrp)
BOOL fPlan;
struct XBC *pxbc;
struct XSF *pxsf;
struct CA *pca;
int edcDrp;
{
	struct PLC **hplc, **hplcRef;
	int cfoo;
	int cRef;
	int docSub;
	struct DRP *pdrp;
	struct CA caT;
	struct XSR *pxsr2;

	pdrp = ((int *)PdodDoc(pca->doc)) + edcDrp;
	docSub = pdrp->doc;
/* test if footnote table exists at all */
	if ((hplcRef = pdrp->hplcRef) == hNil)  return;

	if (fPlan)
		{
		pxsf->ifooFirst = IInPlcRef(hplcRef, pca->cpFirst);
		pxsf->ifooLim = IInPlcCheck(hplcRef, pca->cpLim - 1) + 1;
		}

	/* this could be positive because of a -1 return value from IInPlcCheck */
	if ((cfoo = pxsf->ifooFirst - pxsf->ifooLim) >= 0)
		return;

#ifdef DEBUG
	if (!fPlan)
		{
		Assert(pxsf->ifooFirst == IInPlcRef(hplcRef, pca->cpFirst));
		Assert(pxsf->ifooLim == IInPlcCheck(hplcRef, pca->cpLim - 1) + 1);
		}
#endif /* DEBUG */

/* we are deleting cfoo footnotes/annotations...
cp* now refers to footnote/annotation text, not document text.*/

	hplc = PdodDoc(docSub)->hplcfnd; /* union with hplcand */
	AssertH(hplc);

	pxsr2 = PxsAlloc(pxbc);
	CheckPxs(fPlan, pxsr2, nxsDelRefText, edcDrp);
	pxsr2->pcaDel = PcaSet(&caT, docSub, CpPlc(hplc, pxsf->ifooFirst),
			CpPlc(hplc, pxsf->ifooLim));
	pxsr2->fn = fnNil;
	pxsr2->fc = fc0;
	pxsr2->dfc = fc0;
	XReplace(fPlan, pxbc, pxsr2);

	if (fPlan)
		{
		PostTn(pxbc, tntHplc, hplc, cfoo);
		PostTn(pxbc, tntHplc, hplcRef, cfoo);
		}
	else
		{
		AssertDo(FOpenPlc(hplc, pxsf->ifooFirst, cfoo));
		AssertDo(FOpenPlc(hplcRef, pxsf->ifooFirst, cfoo));
		InvalAutoReferences(pca->doc, pxsf->ifooFirst, edcDrp);
		}
}


/* X  D E L E T E  H D R  T E X T */
/* deletes the hdd entries/text referenced for the sections about to be
deleted.
*/
/*  %%Function:XDeleteHdrText %%Owner:peterj %%reviewed: 6/28/89 */
EXPORT XDeleteHdrText(fPlan, pxbc, pxsp, pca)
BOOL fPlan;
struct XBC *pxbc;
struct XSP *pxsp;
struct CA *pca;
{
	int doc = pca->doc;
	struct DOD *pdod = PdodDoc(doc);
	int docHdr = pdod->docHdr, docHdrDisp, ww;
	struct PLC **hplchdd;
	struct PLC **hplcsed;
	int ihddLim, ihddFirst;
	int chdd, ised, isedLim;
	struct CA caFirst, caT;
	CP cpLim;
#ifdef WIN
	struct CA caSectNext;
	int grpfIhdtPrev, grpfIhdtNext;
	CP cpPrev;
#endif

/* WARNING: this routine could get called during the !fPlan phase even though it
was not called during the fPlan phase because of the creation of docHdr in
XAddHdrText.  make sure that we will return without doing anything anyway (and
don't save any state) */

	if (!pdod->fMother || docHdr == docNil)
		return;

#ifdef DEBUG
	if (fPlan)
		pxsp->fCalledXDeleteHdrText = fTrue;
#endif /* DEBUG */

/* close or invalidate all header displays whose section is about to be
deleted */
	caHdt.doc = docNil;
	hplcsed = pdod->hplcsed;
	ised = IInPlc(hplcsed, pca->cpFirst);
	isedLim = IInPlc(hplcsed, pca->cpLim);

	if (ised == isedLim)
		return;

	docHdrDisp = docHdr;

/* walk the linked list of display documents displaying this header (there may
be zero, one or more).  the linked list is maintained through the docHdr entry 
of each dod (note that short DODs have a docHdr too).  the chain looks like:
	docMother->docHdr->docHdrDisp->docHdrDisp->...->docNil
when entering these loops docHdrDisp is initially docHdr.
*/

	if (fPlan)
		{
		while ((docHdrDisp = PdodDoc(docHdrDisp)->docHdr) != docNil)
			{
			pdod = PdodDoc(docHdrDisp);
			if (pdod->ised < ised)
				continue;
			if (pdod->ised < isedLim)
				{
				for (ww = WwDisp(docHdrDisp, wwNil, fTrue); ww != wwNil; 
						ww = WwDisp(docHdrDisp, ww, fTrue))
					if (PwwdWw(ww)->fHdr &&
							PdrGalley(PwwdWw(ww))->doc == docHdrDisp)
						{
						PostTn(pxbc, tntCloseHdrWw, ww, 0);
						}
				}
			}
		}

	else  /* !fPlan */		
		{
		vdocHdrEdit = docNil;	/* stave off rename */
		while ((docHdrDisp = PdodDoc(docHdrDisp)->docHdr) != docNil)
			{
			pdod = PdodDoc(docHdrDisp);
			if (pdod->ised < ised)
				continue;
			if (pdod->ised >= isedLim)
				pdod->ised -= isedLim - ised;
			else
				{
				pdod->ised = -1; /* section no longer exists */
				InvalPageView(docHdrDisp);
				}
			}
		}

#ifdef REFERENCE  /* Mac version (non-bifurcated) */
	if (!fPlan /*&& ised != isedLim*/)
		{
		docHdrDisp = docHdr;
		vdocHdrEdit = docNil;	/* stave off rename */
		while ((docHdrDisp = PdodDoc(docHdrDisp)->docHdr) != docNil)
			{
			pdod = PdodDoc(docHdrDisp);
			if (pdod->ised < ised)
				continue;
			if (pdod->ised >= isedLim)
				pdod->ised -= isedLim - ised;
			else
				{
				pdod->ised = -1; /* section no longer exists */
				InvalPageView(docHdrDisp);
				for (ww = WwDisp(docHdrDisp, wwNil, fTrue); ww != wwNil; 
						ww = WwDisp(docHdrDisp, ww, fTrue))
					if (PwwdWw(ww)->fHdr &&
							PdrGalley(PwwdWw(ww))->doc == docHdrDisp)
						{
						/* header window or split - close it */
						DisposeWwHdr(ww);
						if (mpdochdod[docHdrDisp] == hNil)
							{
							docHdrDisp = docHdr;
							break;	/* start over */
							}
						ww = wwNil; /* start over */
						}
				}
			}
		}
#endif

/* we may be applying properites to the section following the section deleted */
/* ...SedMac used because caSectNext used ultimately for applyGrpPrl */
	CacheSectSedMac(doc, pca->cpLim);
	cpLim = caSect.cpFirst;
	caSect.doc = docNil; /* to invalidate caSect because SedMac used */

#ifdef WIN
/* promote headers if deleting head of chain */
	if (isedLim + 1 < IMacPlc(hplcsed)) /* we had sections following */
		{
		grpfIhdtPrev = 0;
		grpfIhdtNext = vsepFetch.grpfIhdt;
		caSectNext = caSect;
		caSectNext.doc = doc;

		if (grpfIhdtNext == 0)
			{
/* the deleting sections may be the head of the link for sections
following.*/

			while (grpfIhdtPrev == 0)
				{
/* search backward for the head of the link within the deleting sections. */
				cpPrev = caSect.cpFirst - 1;
				if (cpPrev < pca->cpFirst)
					break;
				Assert(cpPrev >= cp0);
				CacheSect(doc, cpPrev);
				grpfIhdtPrev = vsepFetch.grpfIhdt;
				}

/* set the grpfIhdt to the remaining section so it becomes the
head of the link.  Setup cpLim to collect the ihdds that should
be deleted if necessary.
*/
			if (grpfIhdtPrev != 0)
				{
				XSetGrpfIhdtPca(fPlan, pxbc, pxsp, grpfIhdtPrev, &caSectNext);
				if (cpPrev < pca->cpFirst ||
						(cpLim = caSect.cpFirst) <= pca->cpFirst)
					{
					vdocHdrEdit = doc;
					return; /* no hdd needs to be deleted, they are being
					refered to by the new head of link */
					}
				}
			}
		}
	Assert(pca->cpFirst <= cpLim);
#endif /* WIN */

	ihddLim = IhddFromDocCp(PcaSet(&caT, doc, pca->cpFirst, cpLim), &ihddFirst, 0);
	if ((chdd = ihddLim - ihddFirst) > 0)
		{
		struct XSR *pxsr2;

		/* if we get here, make sure we planed for this (see warning above) */
		Assert(pxsp->fCalledXDeleteHdrText);

		CaFromIhddFirstLim(docHdr, ihddFirst, ihddLim,  &caFirst);

		/* Delete the text in docHdr */

		pxsr2 = PxsAlloc(pxbc);
		CheckPxs(fPlan, pxsr2, nxsDelHdrText, 0);
		pxsr2->pcaDel = &caFirst;
		pxsr2->fn = fnNil;
		pxsr2->fc = fc0;
		pxsr2->dfc = fc0;

		XReplace(fPlan, pxbc, pxsr2);

		if (fPlan)
			PostTn(pxbc, tntHplc, PdodDoc(docHdr)->hplchdd, -chdd);
		else
			FOpenPlc(PdodDoc(docHdr)->hplchdd, ihddFirst, -chdd);
		}
	vdocHdrEdit = doc;
}


/* X  A D D  H D R  T E X T */
/* copies any headers in the source to the designated section in the
destination. It is assumed that the sed's/sep's have already been copied.
*/
/*  %%Function:XAddHdrText %%Owner:peterj %%reviewed: 6/28/89 */
EXPORT XAddHdrText(fPlan, pxbc, pcaSrc, pcaDest)
BOOL fPlan;
struct XBC *pxbc;
struct CA *pcaSrc, *pcaDest;
{
	struct DOD *pdod = PdodDoc(pcaSrc->doc);
	int docHdrSrc = pdod->docHdr;
	int docHdrDest, docHdr, ww;
	struct PLC **hplchddHdrDest;
	int ihddDest;
	int ihddLim, ihddFirst;
	int chdd;
	int docDest = pcaDest->doc;
	struct PLC **hplcsed;
	int csed, ised;
	CP cpLim;
	struct CA caFirst, caLim, caDest, caT;
	struct XSR *pxsr2;

	if (!pdod->fMother)
		return(fTrue);

	hplcsed = PdodDoc(pcaSrc->doc)->hplcsed;
	if ((csed = IInPlc(hplcsed, pcaSrc->cpLim)-IInPlc(hplcsed, pcaSrc->cpFirst))
			<= 0)
		return;

	docHdrDest = PdodDoc(docDest)->docHdr;
	if (docHdrSrc == docNil)
		{
		if (docHdrDest == docNil || fPlan)
			return;
		goto LCheckSects;
		}

/* count headers up to beginning of last section included in copy. if
	copy includes end of section, count headers to end of section */
	caT = *pcaSrc;
	CacheSectSedMac(caT.doc, caT.cpLim - 1);
	if (caT.cpLim < caSect.cpLim)
		caT.cpLim = caSect.cpFirst;
	ihddLim = IhddFromDocCp(&caT, &ihddFirst, 0);

	if ((chdd = ihddLim - ihddFirst) > 0)
		{
		CaFromIhddFirstLim(docHdrSrc, ihddFirst, ihddLim, &caFirst);

/* now we have Src text. Find destination. */
		if (fPlan)
			{
			if (docHdrDest == docNil)
				{
				if ((docHdrDest = DocCreateSub(docDest, dkHdr)) == docNil)
					{
LPostAbort:
					PostTn(pxbc, tntAbort, NULL, 0);
					goto LRet;
					}
				PdodDoc(docDest)->docHdr = docHdrDest;
				}
			if ((hplchddHdrDest = PdodDoc(docHdrDest)->hplchdd) == hNil &&
					(hplchddHdrDest = HplcCreateEdc(mpdochdod[docHdrDest], edcHdd))
					== hNil)
				goto LPostAbort;
			}
		else
			{
			Assert(docHdrDest != docNil);
			hplchddHdrDest = PdodDoc(docHdrDest)->hplchdd;
			AssertH(hplchddHdrDest);
			}

		CacheSect(docDest, pcaDest->cpFirst);
		if ((ihddDest = IhddFromDocCp(PcaSet(&caT,docDest,cp0,caSect.cpFirst),
				0, 0)) == -1)
			{
			Assert(fFalse); /* this should never happen (?) */
			goto LRet;
			}
		CaFromIhdd(docHdrDest, ihddDest, &caDest);
		caDest.cpLim = caDest.cpFirst;

		pxsr2 = PxsAlloc(pxbc);
		CheckPxs(fPlan, pxsr2, nxsAddHdrText, 0);
		pxsr2->pcaDel = &caDest;
		pxsr2->pcaIns = &caFirst;

		XReplaceCps(fPlan, pxbc, pxsr2);

		if (fPlan)
			PostTn(pxbc, tntHplc, hplchddHdrDest, chdd);
		else
			{
/* note: AdjustCp left ihddDest unadjusted */
			PutCpPlc(hplchddHdrDest, ihddDest,
					CpPlc(hplchddHdrDest, ihddDest) + DcpCa(&caFirst));

/* open up space in the destination and copy the cp's from the source */
			AssertDo(FOpenPlc(hplchddHdrDest, ihddDest, chdd));

			CopyMultPlc(chdd, PdodDoc(docHdrSrc)->hplchdd, ihddFirst,
					hplchddHdrDest, ihddDest,
					caDest.cpFirst - caFirst.cpFirst, 0, 0);
			}
		}

	if (fPlan)
		goto LRet;

/* invalidate page view for all displayed headers */
LCheckSects:
	if (docHdrDest != docNil)
		{
		ised = IInPlc(PdodDoc(pcaDest->doc)->hplcsed, pcaDest->cpFirst);
		docHdr = docHdrDest;
		while ((docHdr = PdodDoc(docHdr)->docHdr) != docNil)
			{
			pdod = PdodDoc(docHdr);
			if (pdod->ised < ised)
				continue;
			pdod->ised += csed;
			InvalPageView(docHdr);
			}
		}

LEnd: /* need to do this in !fPlan case only... */
	vdocHdrEdit = docDest;
LRet:
	caSect.doc = docNil; /* protection -- used CacheSectSedMac */
}


#ifdef WIN
/* X  S E T  G R P F  I H D T  P C A */
/* apply a grpprl to pca to change the grpfIhdt */
/*  %%Function:XSetGrpfIhdtPca %%Owner:peterj %%reviewed: 6/28/89 */
XSetGrpfIhdtPca(fPlan, pxbc, pxsp, grpfIhdt, pca)
BOOL fPlan;
struct XBC *pxbc;
struct XSP *pxsp;
int grpfIhdt;
struct CA *pca;
{
	CHAR rgb[2];
#ifdef DEBUG
	struct ESPRM *pesprm;
	pesprm = &dnsprm[sprmSGrpfIhdt];
	Assert(pesprm->b == offset(SEP,grpfIhdt));
	Assert(pesprm->sgc == sgcSep);
	Assert(pesprm->cch == 2);
#endif

	Assert(!PdodDoc(pca->doc)->fShort);
	rgb[0] = sprmSGrpfIhdt;
	rgb[1] = (CHAR)grpfIhdt;
	XApplyGrpprlDocCp(fPlan, pxbc, pxsp, rgb, 2, pca->doc, pca->cpLim-1);
	if (!fPlan && caSect.doc == pca->doc)
		caSect.doc = docNil; /* blow cache */
}


/* X  A P P L Y  G R P P R L  D O C  C P */
/* perform a bifuricated applygrpprl. Only good for section sprms currently.
passed in pca must include only one piece. */
/*  %%Function:XApplyGrpprlDocCp %Owner:peterj %%reviewed: 6/28/89 */
XApplyGrpprlDocCp(fPlan, pxbc, pxsp, grpprl, cb, doc, cp)
BOOL fPlan;
struct XBC *pxbc;
struct XSP *pxsp;
char *grpprl;
int cb;
int doc;
CP cp;
{
	int ipcd;
	CP dcp;
	struct DOD **hdod = mpdochdod[doc];
	struct PLC **hplcpcd = (*hdod)->hplcpcd;
	struct PCD pcd;

	Assert(cb > 0);

	if (cp > CpMacDoc(doc))
		{
		cp = CpMacDoc(doc);
		dcp = ccpEop;
		}
	else
		dcp = 1;

/* ensure that, for section sprms, the section table exists */
	Assert(dnsprm[*grpprl/*psprm*/].sgc == sgcSep &&
			!(*hdod)->fShort && (*hdod)->hplcsed != hNil);

/* First get address of piece table and split off desired pieces. */
	ipcd = ipcd = IpcdSplit(hplcpcd, cp);
	if (ipcd == iNil || IpcdSplit(hplcpcd, cp+dcp) != ipcd+1)
		{
		Assert(fPlan);
LAbort:
		PostTn(pxbc, tntAbort, NULL, 0);
		return;
		}

/* Now just add this sprm to the piece. */
	GetPlc(hplcpcd, ipcd, &pcd);
	if (fPlan)
		{
		pxsp->prm = PrmAppend(pcd.prm, grpprl, cb);
		/* check for failure of PrmAppend */
		if (vmerr.fFmtFailed)
			goto LAbort;
		/* don't reclaim this prc if FreeUnusedPrcs is called */
		vmerr.prmDontReclaim = pxsp->prm;
		}
	else
		{
		pcd.prm = pxsp->prm;
		PutPlc(hplcpcd, ipcd, &pcd);
		}

#ifdef WIN
	InvalVisiCache();
#endif /* WIN */
}


#endif /* WIN */

#ifdef WIN

/* X  D E L E T E  F I E L D S */
/*  Delete any live fields which begin in the range [cpFirst,cpLim) from the
	plcfld.

	NOTE:  It is assumed that cpFirst:cpLim is a valid range.  A valid range
	is one where there are no unmatched field begin-ends.
*/
/*  %%Function:XDeleteFields %%Owner:peterj %%reviewed: 6/28/89 */
EXPORT XDeleteFields(fPlan, pxbc, pxsl, pca)
BOOL fPlan;
struct XBC *pxbc;
struct XSL *pxsl;
struct CA *pca;
{
	struct PLC **hplcfld = PdodDoc(pca->doc)->hplcfld;

	AssertH(hplcfld);

	if (fPlan)
		{
		int ifldLim, ifldFirst, ifldMac;
		ifldFirst = IInPlcRef (hplcfld, pca->cpFirst);
		Assert (ifldFirst <= IMacPlc(hplcfld));
		Assert (ifldFirst >= -1);
		if (ifldFirst > -1)
			{
			ifldLim = IInPlcRef(hplcfld, pca->cpLim);
			if ((pxsl->cfld = ifldFirst - ifldLim) < 0)
				PostTn(pxbc, tntHplc, hplcfld, pxsl->cfld);
			}
		else
			pxsl->cfld = 0;
		}
	else  if (pxsl->cfld < 0)
		{
		PdodDoc(pca->doc)->fFldNestedValid = fFalse;
		pxsl->ifld = IInPlcRef (hplcfld, pca->cpFirst);
		Assert(pxsl->ifld >= 0);
		Assert(pxsl->ifld - pxsl->cfld <= IMacPlc(hplcfld));
		FOpenPlc (hplcfld, pxsl->ifld, pxsl->cfld);
		if (vddes.cDclClient > 0)
			vfDdeIdle = fTrue;
		if (!PdodDoc(pca->doc)->fHasNoSeqLev)
			vidf.fInvalSeqLev = PdodDoc(pca->doc)->fInvalSeqLev = fTrue;
		}
}


/* X  C O P Y  F I E L D S */
/* Copy all live fields from pcaSrc to pcaDest. */
/*  %%Function:XCopyFields %%Owner:peterj %%reviewed: 6/28/89 */
EXPORT XCopyFields(fPlan, pxbc, pcaSrc, pcaDest)
BOOL fPlan;
struct XBC *pxbc;
struct CA *pcaSrc, *pcaDest;
{
	int ifld = IfldNextField (pcaSrc->doc, pcaSrc->cpFirst);
	CP dcpAdj = pcaDest->cpFirst - pcaSrc->cpFirst;
	struct FLCD flcd;
	int cfld = 0;

	GetIfldFlcd (pcaSrc->doc, ifld, &flcd);

	while (ifld != ifldNil && flcd.cpFirst < pcaSrc->cpLim)
		{
		/*  else not a valid range to be copying */
		Assert (flcd.cpFirst + flcd.dcpInst + flcd.dcpResult <= pcaSrc->cpLim);

		/* copy this field to docDest */
		if (fPlan)
			cfld += 2 + (flcd.dcpResult ? 1 : 0);
		else
			{
			FInsertFltDcps (flcd.flt, pcaDest->doc, flcd.cpFirst+dcpAdj, 
					flcd.dcpInst, flcd.dcpResult, &flcd);

			if ((uns)(flcd.flt-fltSeqLevOut)<=(fltSeqLevNum-fltSeqLevOut))
				/* invalidate auto numbered fields */
				vidf.fInvalSeqLev = PdodDoc(pcaDest->doc)->fInvalSeqLev = fTrue;
			}

		ifld = IfldAfterFlcd (pcaSrc->doc, ifld, &flcd);
		}
	if (cfld > 0)
		{
		Assert(fPlan);
		if (!FAssureHplcfld(pcaDest->doc, 0))
			PostTn(pxbc, tntAbort, NULL, 0);
		else
			PostTn(pxbc, tntHplc, PdodDoc(pcaDest->doc)->hplcfld, cfld);
		}
}


/* X  D E L E T E  B K M K S */
/* Delete bookmarks in enclosed in pca.  truncate bookmarks that are
partially in the range. */
/*  %%Function:XDeleteBkmks %%Owner:peterj %%reviewed: 6/28/89 */
EXPORT XDeleteBkmks(fPlan, pxbc, pca, fAll)
BOOL fPlan;
struct XBC *pxbc;
struct CA *pca;
BOOL fAll; /* if true an ins pt bkmk at cpFirst will be deleted */
{
	CP cpFirstBkmk;
	CP cpLimBkmk;
	CP cpFirst = pca->cpFirst;
	CP cpLim = pca->cpLim;
	struct DOD *pdod = PdodDoc(pca->doc);
	struct PLC **hplcbkf = pdod->hplcbkf;
	struct PLC **hplcbkl = pdod->hplcbkl;
	struct STTB **hsttb = pdod->hsttbBkmk;
	int ibkf;
	int cBkmkDel, cbSttbDel;
	struct BKF bkf;

	Assert (!pdod->fShort);

	if (hplcbkf == hNil || hplcbkl == hNil || hsttb == hNil)
		return; /* no bookmarks to delete */

	if (fPlan)
		{
		cBkmkDel = 0;
		cbSttbDel = 0;
		}

/* scan backwards through bookmarks deleting those that are in the range
(backwards so we don't have to adjust when we delete) */
	for (ibkf=IMacPlc(hplcbkf)-1; ibkf >= 0; --ibkf)
		if ((cpFirstBkmk = CpPlc(hplcbkf, ibkf)) >= cpFirst &&
				cpFirstBkmk < cpLim)
			{
			GetPlc(hplcbkf, ibkf, &bkf);
			if ((cpLimBkmk = CpPlc(hplcbkl, bkf.ibkl)) <= cpLim &&
					(cpLimBkmk > cpFirst || fAll))
				if (fPlan)
					{
					/* 2=overhead of sttb entry (ibst), 1=length byte of st */
					cbSttbDel += 2 + 1 + *HpstFromSttb(hsttb, ibkf);
					cBkmkDel++;
					}
				else
					{
#ifdef PCJ
					CommSzNum(SzShared("deleting ibkf = "), ibkf);
#endif /* PCJ */
					DeleteIbkf(pca->doc, ibkf, hsttb, hplcbkf, hplcbkl, NULL);
					}
			}

	if (!fPlan)
		{
		/* truncate intersecting portions of bookmarks which are only partially
			enclosed in the range */
		ChangeCpsInRange(hplcbkf, cpFirst+1, cpLim, cpLim);
		ChangeCpsInRange(hplcbkl, cpFirst+1, cpLim, cpFirst);
		}
	else  if (cBkmkDel)
		{
		PostTn(pxbc, tntHsttb, hsttb, -cbSttbDel);
		PostTn(pxbc, tntHplc, hplcbkf, -cBkmkDel);
		PostTn(pxbc, tntHplc, hplcbkl, -cBkmkDel);
		PostTn(pxbc, tntDelBkmk, pca->doc, 0);
		}
}


/* C H A N G E  C P S  I N  R A N G E */
/*  change all rgcp entries in the range [cpFirst,cpLim) to be cpNew. */
/*  %%Function:ChangeCpsInRange %%Owner:peterj %%reviewed: 6/28/89 */
ChangeCpsInRange(hplc, cpFirst, cpLim, cpNew)
struct PLC **hplc;
CP cpFirst, cpLim, cpNew;
{
	int icp, iMac;

	iMac = IMacPlc(hplc);
	for (icp = max(IInPlcCheck(hplc, cpFirst), 0); icp < iMac; icp++)
		{
		CP cp = CpPlc(hplc, icp);
		if (cp >= cpLim)
			break;
		if (cp >= cpFirst)
			{
#ifdef PCJ
			CommSzNum(SzShared("Changing icp = "), icp);
#endif /* PCJ */
			PutCpPlc(hplc, icp, cpNew);
			}
		}
}


/* X  C O P Y  B K M K S */
/*  Copy bookmarks in pcaSrc to pcaDest.  (pcaDest->cpLim ignored) */
/*  %%Function:XCopyBkmks %%Owner:peterj %%reviewed: 6/28/89 */
EXPORT XCopyBkmks(fPlan, pxbc, pxsb, pcaSrc, pcaDest)
BOOL fPlan;
struct XBC *pxbc;
struct XSB *pxsb;
struct CA *pcaSrc, *pcaDest;

{
	int docDest = pcaDest->doc;
	struct DOD *pdod = PdodDoc(pcaSrc->doc);
	struct STTB **hsttb = pdod->hsttbBkmk;
	struct PLC **hplcbkf = pdod->hplcbkf;
	struct PLC **hplcbkl = pdod->hplcbkl;
	struct STTB **hsttbDest = hNil;
	struct PLC **hplcbkfDest;
	struct PLC **hplcbklDest;
	int ibkf, ibkfLim, ibklLim, iMac;
	struct BKF bkf;
	struct CA caT;
	CP dcp;
	int cBkmkIns, cbSttbIns;

	caT = *pcaSrc;

	Assert (docDest != caT.doc);
	Assert (!PdodDoc (docDest)->fShort && !PdodDoc (caT.doc)->fShort);
	Assert (caT.cpLim > caT.cpFirst);

	if (caT.cpFirst >= CpMacDoc(caT.doc))
		return;

	if (caT.cpLim > CpMacDoc(caT.doc))
		caT.cpLim = CpMacDoc(caT.doc);

	if (hplcbkf == hNil || hplcbkl == hNil || hsttb == hNil)
		return; /* no bookmarks to copy */

	/* adjustment for starting cp in the different documents */
	dcp = pcaDest->cpFirst - caT.cpFirst;

	/* hplcbkf and hplcbkl are related plcs. they should have the same iMac */
	Assert(IMacPlc(hplcbkf) == IMacPlc(hplcbkl));
	if ((iMac = IMacPlc(hplcbkf)) == 0)
		return;

	if (fPlan)
		{
		cBkmkIns = 0;
		cbSttbIns = 0;

		pxsb->ibkf = pxsb->ibkfLim = 0;

		/*  ibklLim is first ibkl st cp > caT.cpLim */
		if (CpPlc(hplcbkl, 0) <= caT.cpLim)
			{
			ibklLim = IInPlc (hplcbkl, caT.cpLim);
			while (CpPlc(hplcbkl, ibklLim) <= caT.cpLim && ibklLim < iMac)
				ibklLim++;
			}
		else
			return; /* none fully enclosed */

		/*  ibkfLim is first ibkf st cp >= caT.cpLim */
		if (CpPlc(hplcbkf, 0) < caT.cpLim)
			{
			ibkfLim = IInPlcMult (hplcbkf, caT.cpLim);
/* FUTURE davidlu(pj): this should be unnecessary when the new implementation
	if IInPlc(Mult)Ref is in place (change the above to Ref) */
			while (CpPlc(hplcbkf, ibkfLim) < caT.cpLim && ibkfLim < iMac)
				ibkfLim++;
			}
		else
			return; /* none fully enclosed */

		/*  ibkf is first ibkf st cp >= caT.cpFirst */
		if (CpPlc(hplcbkf, 0) <= caT.cpFirst)
			{
			ibkf = IInPlcMult (hplcbkf, caT.cpFirst);
/* FUTURE davidlu(pj): this should be unnecessary when the new implementation
	if IInPlc(Mult)Ref is in place (change the above to Ref) */
			while (CpPlc(hplcbkf, ibkf) < caT.cpFirst && ibkf < iMac)
				ibkf++;
			}
		else
			ibkf = 0;

		pxsb->ibkf = ibkf;
		pxsb->ibkfLim = ibkfLim;
		pxsb->ibklLim = ibklLim;
		}
	else
		{
		ibkf = pxsb->ibkf;
		ibkfLim = pxsb->ibkfLim;
		ibklLim = pxsb->ibklLim;
		}

	while (ibkf < ibkfLim)
		{
		GetPlc(hplcbkf, ibkf, &bkf);
		if (bkf.ibkl < ibklLim)
			if (fPlan)
				{
				if (hsttbDest == hNil)
					/* get the destination handles */
					{
					if (!FAssureBkmksForDoc(docDest))
						{
						PostTn(pxbc, tntAbort, NULL, 0);
						return;
						}
					else
						{
						struct DOD *pdodT = PdodDoc(docDest);
						hsttbDest = pdodT->hsttbBkmk;
						hplcbkfDest = pdodT->hplcbkf;
						hplcbklDest = pdodT->hplcbkl;
						}
					}
				cbSttbIns += 3 + *HpstFromSttb(hsttb, ibkf);
				cBkmkIns++;
				}
			else
				{
				char *pst, st [cchBkmkMax];
				struct CA caBkmk;

				PcaSet(&caBkmk, caT.doc, CpPlc(hplcbkf, ibkf),
						CpPlc(hplcbkl, bkf.ibkl));

				Assert( FCaInCa( &caBkmk, pcaSrc ) );

				caBkmk.cpFirst += dcp;  /* convert to dest units */
				caBkmk.cpLim += dcp;
				caBkmk.doc = docDest;

				GetStFromSttb(hsttb, ibkf, st);
				Assert(st[0] < cchBkmkMax);

				Assert (DcpCa(&caBkmk) >= 0 && caBkmk.cpFirst >= cp0 );
				AssertDo(FInsertStBkmk(&caBkmk, st, NULL));
				}
		ibkf++;
		}

	if (fPlan && cBkmkIns)
		{
		PostTn(pxbc, tntHsttb, hsttbDest, cbSttbIns);
		PostTn(pxbc, tntHplc, hplcbkfDest, cBkmkIns);
		PostTn(pxbc, tntHplc, hplcbklDest, cBkmkIns);
		}
}


/* F  A S S U R E  B K M K S  F O R  D O C */
/*  Assure that the necessary structures for bookmarks exist for the passed in
doc. */
/*  %%Function:FAssureBkmksForDoc %%Owner:peterj %%reviewed: 6/28/89 */
FAssureBkmksForDoc(doc)
int doc;
{
	struct DOD *pdod = PdodDoc(doc);

	Assert(!pdod->fShort);

	if (pdod->hsttbBkmk == hNil)
		{
		struct STTB **hsttb = hNil;
		struct PLC **hplcbkf = hNil;
		struct PLC **hplcbkl = hNil;

		Assert (pdod->hplcbkl == hNil && pdod->hplcbkf == hNil);
		if ((hsttb = HsttbInit (0, fTrue)) == hNil ||
				(hplcbkf=HplcInit(cbBKF, 1, CpMac2Doc(doc), fTrue/*fExt*/)) == hNil ||
				(hplcbkl=HplcInit(cbBKL, 1, CpMac2Doc(doc), fTrue/*fExt*/)) == hNil)
			{
			FreeHsttb (hsttb);
			FreeHplc (hplcbkf);
			FreeHplc (hplcbkl);
			return fFalse;
			}
		Win((*hplcbkf)->fMult = fTrue);
		Win((*hplcbkl)->fMult = fTrue);
		pdod = PdodDoc(doc);
		pdod->hsttbBkmk = hsttb;
		pdod->hplcbkf = hplcbkf;
		pdod->hplcbkl = hplcbkl;
		pdod->fFormatted = fTrue;
		}
	return fTrue;
}


/* F  I N S E R T  S T  B K M K */
/* Insert st into the bookmark structures of document doc. Limits st 
to cchBkmkMax characters.  Initializes the bookmark structures of doc
if not already initialized.  If pfUndoCancel is non-null then the
undo document will be set up, and the flag returned in pfUndoCancel
will be true iff the user refused to continue without undo.
*/
/*  %%Function:FInsertStBkmk %%Owner:peterj %%reviewed: 6/28/89 */
FInsertStBkmk (pca, st, pfUndoCancel)
struct CA *pca;
char *st;
int *pfUndoCancel;
{
	int doc = pca->doc;
	struct DOD **hdod = mpdochdod [doc];
	struct STTB **hsttb;
	struct PLC **hplcbkf;
	struct PLC **hplcbkl;
	int ibkf, ibkl, ibkfMac;
	struct BKF bkf;
	struct CA caUndo;

	Assert (hdod != hNil);
	Assert (!(*hdod)->fShort);
	Assert (pca->cpFirst >= cp0 && 
			pca->cpLim < CpMac1Doc(pca->doc) && 
			pca->cpFirst <= pca->cpLim);

	if (pfUndoCancel)
		{
		*pfUndoCancel = fFalse;
		if (!FSetPcaForBkmkUndo(&caUndo, doc, pca->cpFirst, pca->cpLim, fTrue))
			/* alas, cannot undo some bookmark actions */
			{
			SetUndoNil();
			pfUndoCancel = NULL;
			}
		}

	/* insure st ok size */
	if (*st >= cchBkmkMax)
		*st = cchBkmkMax - 1;

	/*  only insert legal bookmark names */
	if (!FLegalBkmkName (st))
		return fFalse;

	/* initialize bookmarks if needed */
	if (!FAssureBkmksForDoc(doc))
		return fFalse;

	hsttb = (*hdod)->hsttbBkmk;
	hplcbkf = (*hdod)->hplcbkf;
	hplcbkl = (*hdod)->hplcbkl;

	if (IMacPlc(hplcbkf) >= ibkmkMax)
		/* limit on number of bookmarks has been reached */
		return fFalse;

	/*  if bookmark already exists, delete it */
	if (FSearchSttbUnsorted (hsttb, st, &ibkf))
		{
		/* keep same case as it had before */
		GetStFromSttb(hsttb, ibkf, st);
		DeleteIbkf (doc, ibkf, hsttb, hplcbkf, hplcbkl, pfUndoCancel);
		if (pfUndoCancel && *pfUndoCancel)
			return fFalse;
		}

	else  if (pfUndoCancel)
		{
		AssureLegalSel(&caUndo);
		if (!FSetUndoB1(ucmChangeBkmk, uccFormat, &caUndo))
			{
			*pfUndoCancel = fTrue;
			return fFalse;
			}
		}

	/* Insert new bookmark */

	/* determine if we can grow everything enough */
	if (!vfInCommit)
		/* already guaranteed if vfInCommit */
		{
		if (!FStretchSttb(hsttb, 1, *st) || !FStretchPlc(hplcbkf, 1)
				|| !FStretchPlc(hplcbkl, 1))
			/* cannot grow structs to contain new data, bag out! */
			return fFalse;
		}

	if (CpPlc(hplcbkl, 0) < pca->cpLim)
		{
/* FUTURE davidlu(pj): a call to the fixed IInPlcRef should replace this */
		ibkl = IInPlc (hplcbkl, pca->cpLim);
		while (CpPlc(hplcbkl, ibkl) < pca->cpLim)
			ibkl++;
		}
	else
		ibkl = 0;
	ibkfMac = IMacPlc(hplcbkf);
	for (ibkf = 0; ibkf < ibkfMac; ibkf++)
		{
		GetPlc(hplcbkf, ibkf, &bkf);
		if (bkf.ibkl >= ibkl)
			{
			bkf.ibkl++;
			PutPlcLast(hplcbkf, ibkf, &bkf);
			}
		}

	if (CpPlc(hplcbkf, 0) < pca->cpFirst)
		{
/* FUTURE davidlu(pj): a call to the fixed IInPlcRef should replace this */
		ibkf = IInPlc (hplcbkf, pca->cpFirst);
		while (CpPlc(hplcbkf, ibkf) < pca->cpFirst)
			ibkf++;
		}
	else
		ibkf = 0;

	/* fix up dde server pl (refers to ibkfs) */
	if (vddes.hplddli != hNil)
		{
		int iddli;
		struct DDLI ddli;

		for ( iddli = 0 ; iddli < IMacPl(vddes.hplddli); iddli++ )
			{
			GetPl( vddes.hplddli, iddli, &ddli );
			if (ddli.ibkf >= ibkf && ddli.ibkf != ibkmkNil &&
					PdcldDcl(ddli.dcl)->doc == doc)
				{
				ddli.ibkf++;
				PutPl( vddes.hplddli, iddli, &ddli );
				}
			}

		if (vddes.fInvalidDdli)     /* so we will try to validate later */
			vfDdeIdle = fTrue;
		}

	bkf.ibkl = ibkl;
	Assert (ibkf <= ibkfMac && ibkl <= ibkfMac);

	/*  these will work because of the stretches above */
	AssertDo(FInsStInSttb (hsttb, ibkf, st));
	AssertDo(FInsertInPlc (hplcbkf, ibkf, pca->cpFirst, &bkf));
	AssertDo(FInsertInPlc (hplcbkl, ibkl, pca->cpLim, NULL));

	/* check to see if we need to adjust Find Next info */
	if (fni.is == isGotoBkmk && fni.doc == pca->doc && ibkf <= fni.ibkf)
		fni.ibkf++;

	/* dirty the document */
	(*hdod)->fDirty = fTrue;
	if (pfUndoCancel)
		SetUndoAfter(&caUndo);
	return fTrue;
}


/* D E L E T E  I B K F */
/*  Delete a bookmark indexed by ibkf.  does not delete structures and
assumes bookmark structures exist. */
/*  %%Function:DeleteIbkf %%Owner:peterj %%reviewed: 6/28/89 */
DeleteIbkf (doc, ibkfDel, hsttb, hplcbkf, hplcbkl, pfUndoCancel)
int doc;
int ibkfDel;
struct STTB **hsttb;
struct PLC **hplcbkf;
struct PLC **hplcbkl;
int *pfUndoCancel;
{
	int ibkl, ibkfT, ibkfMac;
	struct BKF bkf;
	struct CA caUndo;

	Assert (ibkfDel >= 0 && ibkfDel < ibkmkMax);

	GetPlc(hplcbkf, ibkfDel, &bkf);
	ibkl = bkf.ibkl;

	if (pfUndoCancel)
		{
		*pfUndoCancel = fFalse;
		if (!FSetPcaForBkmkUndo(&caUndo, doc, CpPlc(hplcbkf, ibkfDel), 
				CpPlc(hplcbkl, ibkl), fFalse))
			/* alas, cannot undo some bookmarks */
			{
			SetUndoNil();
			pfUndoCancel = NULL;
			}
		else
			{
			AssureLegalSel(&caUndo);
			if (!FSetUndoB1(ucmChangeBkmk, uccFormat, &caUndo))
				{
				*pfUndoCancel = fTrue;
				return;
				}
			}
		}

	/*  delete string and cpFirst entry */
	DeleteIFromSttb (hsttb, ibkfDel);
	FOpenPlc (hplcbkf, ibkfDel, -1);

	/*  update pointers from cpFirst plc to cpLim plc */
	ibkfMac = IMacPlc(hplcbkf);
	for (ibkfT = 0; ibkfT < ibkfMac; ibkfT++)
		{
		GetPlc(hplcbkf, ibkfT, &bkf);
		Assert (bkf.ibkl != ibkl);
		if (bkf.ibkl > ibkl)
			{
			bkf.ibkl--;
			PutPlcLast(hplcbkf, ibkfT, &bkf);
			}
		}

	/*  delete entry from cpLim plc */
	FOpenPlc (hplcbkl, ibkl, -1);

	/* fix up dde server pl */
	if (vddes.hplddli != hNil)
		{
		int iddli;
		struct DDLI ddli;

		for ( iddli = 0; iddli < IMacPl( vddes.hplddli ); iddli++ )
			{
			GetPl( vddes.hplddli, iddli, &ddli );
			if (ddli.ibkf >= ibkfDel && ddli.ibkf != ibkmkNil &&
					PdcldDcl (ddli.dcl)->doc == doc)
				{
				if (ddli.ibkf == ibkfDel)
					{
					ddli.ibkf = ibkmkNil;
					vddes.fInvalidDdli = fTrue; /* dde link points to nothing */
					DdeDbgCommAtom ("Invalidating Link", ddli.dcl, ddli.atomItem);
					}
				else
					ddli.ibkf--;
				PutPl( vddes.hplddli, iddli, &ddli );
				}
			}
		}

	/* adjust Find Next info */
	if (fni.is == isGotoBkmk && fni.doc == doc)
		{
		if (ibkfDel == fni.ibkf)
			{
			if (vfbSearch.fInfo)
				fni.is = isSearch;
			else
				fni.is = isNever;
			}
		else  if (ibkfDel < fni.ibkf)
			fni.ibkf--;
		}

	if (pfUndoCancel)
		SetUndoAfter(&caUndo);
}


/* F  S E T  P C A  F O R  B K M K  U N D O */
/*  because of the way bookmark invalidation/adjustment is performed, some
bookmark deletions cannot be undone. */
/*  %%Function:FSetPcaForBkmkUndo %%Owner:peterj %%reviewed: 6/28/89 */
FSetPcaForBkmkUndo(pcaUndo, doc, cpFirst, cpLim, fInsertion)
struct CA *pcaUndo;
int doc;
CP cpFirst, cpLim;
BOOL fInsertion; /* true for insertion of bkmk, false of deletion */
{
	Assert(fInsertion == 0 || fInsertion == 1);

	if (cpLim >= CpMacDoc(doc) || (fInsertion && cpFirst == cp0))
		return fFalse;

	PcaSet(pcaUndo, doc, cpFirst-fInsertion, cpLim+1);
	return fTrue;
}


#endif /* WIN */


#ifdef PROFILE
/*  this is here so that appended native code does not appear in previous
	function in pcode profiles. */
Edit_Last(){}
#endif /* PROFILE */

/* ADD NEW CODE *ABOVE* Edit_Last() */
