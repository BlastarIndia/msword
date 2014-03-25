#define NOMINMAX
#define NOBRUSH
#define NOICON
#define NOPEN
#define NOGDICAPMASKS
#define NOCLIPBOARD
#define NOKEYSTATE
#define NOSHOWINDOW
#define NOCREATESTRUCT
#define NODRAWTEXT
#define NOMB
#define NOMEMMGR
#define NOMETAFILE
#define NOWH
#define NOWNDCLASS
#define NOREGION
#define NOSOUND
#define NOCOMM
#define NOWINMESSAGES
#define NONCMESSAGES
#define NOSYSMETRICS
#define NORASTEROPS
#define NOBITMAP
#define NOCOLOR
#define NOFONT
#define NOGDI
#define NOHDC
#define NOOPENFILE
#define NOSCROLL
#define NOTEXTMETRIC
#define NOWINOFFSETS
#define NOKANJI


#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "doc.h"
#include "props.h"
#include "sel.h"
#include "disp.h"
#include "file.h"
#include "format.h"
#include "debug.h"
#include "ch.h"
#include "screen.h"
#include "opuscmd.h"
#include "doslib.h"
#include "dmdefs.h"
#include "print.h"
#include "prompt.h"
#include "message.h"
#include "layout.h"
#include "heap.h"
#include "field.h"
#include "dlbenum.h"
#include "error.h"
#include "preview.h"
#include "rerr.h"
#include "rareflag.h"
#define AVGWORD
#include "inter.h"

#include "idd.h"
#include "sdmdefs.h"
#include "sdmver.h"
#include "sdm.h"
#include "sdmtmpl.h"
#include "sdmparse.h"

#include "doc.hs"
#include "doc.sdm"

#include "sumstat.h"

#include "cust.hs"
#include "cust.sdm"
#include "docsum.hs"
#include "docsum.sdm"
#include "docstat.hs"
#include "docstat.sdm"

extern BOOL fElActive;
extern struct SEL selCur;
extern int docGlobalDot;
extern struct PREF vpref;
extern HWND vhwndStatLine;
extern struct DMFLAGS DMFlags;
extern int fnDMEnum;
extern struct MERR	vmerr;
extern struct CA caHdt;
extern CHAR             stEmpty[];
extern int vlm;
extern HWND vhwndPgPrvw;
extern PVS vpvs;
extern CHAR szNone[];
extern int vcBtnFldClicks;
extern BOOL          vfAwfulNoise;


BOOL FCkMarginsDoc();
long CApproxWordCch();
long LRound3L();


/* Cab Extension for Doc Summary Info */
typedef struct _cedsi
	{
	BOOL fEnableOK;
	NDD * pndd;
} CEDSI;

int	cchAvgWord10 = cchAvgWord10Def;

/*  %%Function:  PdttmToSt  %%Owner:  bobz       */

PdttmToSt(pch, st, cchMax)
CHAR    *pch;
CHAR     st[];
int      cchMax;
{
	struct DTTM dttm;
	CHAR        szPic[cchMaxPic];

	bltbyte(pch, &dttm, sizeof(struct DTTM));
	if (dttm.lDttm == 0L)
		{
		st[0] = '\0';
		}
	else
		{
		GetDefaultSzPic(szPic, fTrue);
		st[0] = (CHAR ) (CchFormatDttmPic(&dttm, szPic, &st[1], cchMax - 1));
		}
}


/*  %%Function:  PdttmToStTD  %%Owner:  bobz       */

PdttmToStTD(pch, st, cchMax)
CHAR    *pch;
CHAR     st[];
int      cchMax;
{
	struct DTTM dttm;
	CHAR	chSave;
	CHAR        szPic[cchMaxPic * 2 + 1];

	bltbyte(pch, &dttm, sizeof(struct DTTM));
	if (dttm.lDttm == 0L)
		{
		st[0] = '\0';
		}
	else
		{
		GetDefSzPicDT(szPic);
		st[0] = CchFormatDttmPic(&dttm, szPic, &st[1], cchMax - 1);
		}
}


/*  %%Function:  PunsToSt  %%Owner:  bobz       */

PunsToSt(pch, st, cchMax)
CHAR    *pch;
CHAR     st[];
int      cchMax;
{
	unsigned u;
	long     l;

	u = *((unsigned *) pch);
	l = (long) (u);

	PlongToSt((CHAR *) (&l), st, cchMax);
}


csconst CHAR stMin[]    = StKey("Minute",MinDef);
csconst CHAR stMins[]   = StKey("Minutes",MinsDef);

/*  %%Function:  PlminToSt  %%Owner:  bobz       */

PlminToSt(pch, st, cchMax)
CHAR    *pch;
CHAR     st[];
int      cchMax;
{
	CHAR    *pchMac;
	long     l;

	l = *((long *) (pch));
	PlongToSt((CHAR *) (&l), st, cchMax);

	pchMac = st + st[0] + 1;
	if (l == 1L)
		{
		if (st[0] + stMin[0] + 1< cchMax)
			{
			*pchMac++ = chSpace;
			bltbx((CHAR FAR *) (&stMin[1]), (CHAR FAR *) (pchMac),
					(int) (stMin[0]));
			st[0] += stMin[0] + 1;
			}
		}
	else
		{
		if (st[0] + stMins[0] + 1 < cchMax)
			{
			*pchMac ++ = chSpace;
			bltbx((CHAR FAR *) (&stMins[1]), (CHAR FAR *) (pchMac),
					(int) (stMins[0]));
			st[0] += stMins[0] + 1;
			}
		}
}


/*  %%Function:  PlongToSt  %%Owner:  bobz       */

PlongToSt(pch, st, cchMax)
CHAR    *pch;
CHAR     st[];
int      cchMax;
{
	long     l;
	CHAR    *pchCur, *pchMac;
	CHAR    *stT;

	extern struct ITR vitr;

	bltbyte(pch, &l, sizeof(long));
	stT = &st[1];
	st[0] = (CHAR) (CchLongToPpch(l, &stT));

	/* Insert 100th character. */
	pchCur = pchMac = st + st[0] + 1;
	for (pchCur -= 3; pchCur > &st[1]; pchCur -= 3)
		{
		bltbyte(pchCur, pchCur + 1, pchMac - pchCur);
		pchMac++;
		*pchCur = vitr.chThousand;
		(st[0])++;
		}
}







/* Masks to be used for a return value from GrpfWillDirtyPdopHcab() */
#define grpfLRDirty       0x0001
#define grpfDirty         0x0002
#define grpfFtn           0x0004
#define grpfMarginsOnly   0x0008


/*  %%Function:  CmdDocument  %%Owner:  bobz       */

CMD CmdDocument(pcmb)
CMB * pcmb;
{
	int doc;

	doc = DocMother(selCur.doc);

	if (pcmb->fDefaults)
		{
		if (!FFillHcabDoc(pcmb->hcab, doc))
			return cmdNoMemory;
		}

	if (pcmb->fDialog)
		{
		CHAR dlt [sizeof (dltDocument)];

		BltDlt(dltDocument, dlt);

		switch (TmcOurDoDlg(dlt, pcmb))
			{
#ifdef DEBUG
		default:
			Assert(fFalse);
			return cmdError;
#endif
		case tmcError:
			return cmdError;

		case tmcCancel:
			return cmdCancelled;

		case tmcOK:
			break;
			}
		}

	if (pcmb->fCheck)
		{
		HCABDOCUMENT hcab;
		TMC tmcT;

		hcab = pcmb->hcab;
		if ((*hcab)->iFNStartAt < 1)
			{
			RtError(rerrIllegalFunctionCall);
			Assert(fFalse); /* NOT REACHED */
			}

		if (!FCkMarginsDoc(hcab, &tmcT))
			return cmdError;
		}

	if (pcmb->fAction)
		{
		int grpf, fFail = fFalse;

		SetDopInval(doc, pcmb->hcab, &grpf);

		/* check for changed template */
		ChangeDotFromHcab (pcmb->hcab, doc);

		if (vlm == lmPreview)
			{
			HDC hdc = GetDC(vhwndPgPrvw);
			if (grpf & grpfMarginsOnly && vpvs.tShowAreas != tAreasOff)
				{
				SetVdxpGutter(PdodDoc(vpvs.docPrvw)->dop.dxaGutter);
				ShowAreas(hdc, fFalse); /* erase */
				vpvs.fAreasKnown = fFalse;
				ShowAreas(hdc, fFalse); /* draw */
				vpvs.fModeMargin = fTrue;
				}
			else
				{
				ShowAreas(hdc, fFalse); /* erase */
				fFail = !FChangePrvwDoc(hdc, fTrue, fTrue);
				}
			vpvs.fFacing = FFacingPages(vpvs.docPrvw);
			ReleaseDC(vhwndPgPrvw, hdc);
			if (fFail)
				{
				EndPreviewMode();
				return cmdNoMemory;
				}
			}
		}

	return cmdOK;
}



csconst TMC rgtmcDoc[] =   /* edit controls that must be non-blank */
{  
	tmcDocPageWidth,
			tmcDocPageHeight,
			tmcDocDefTabs,
			tmcDocTMargin, 
			tmcDocBMargin, 
			tmcDocLMargin, 
			tmcDocRMargin,
			tmcDocGutter,
			tmcDocFNStartAt };


#define itmcMacDoc  (sizeof(rgtmcDoc)/sizeof(int))

/*  %%Function:  FDlgDocument  %%Owner:  bobz       */

BOOL FDlgDocument(dlm, tmc, wNew, wOld, wParam)
DLM	dlm;
TMC	tmc;
WORD	wOld, wNew, wParam;
{
	HCAB hcab;
	CHAR sz [cchMaxSz];
	TMC rgtmc[itmcMacDoc];

	switch (dlm)
		{
	case dlmInit:
		if (!PdodMother(selCur.doc)->fDoc)
			EnableTmc(tmcDocTemplate, fFalse);

		/* don't like to do this at init, but to do it at default
			we would have to make these cabtext, and allocate, etc.
			I think this os faster
		*/
		Assert(HcabDlgCur());
		if (((CABDOCUMENT *) *(HcabDlgCur()))->fMirrorMargins)
			{
			SetTmcText(tmcDocLMText, SzSharedKey("I&nside:",MenuInside));
			SetTmcText(tmcDocRMText, SzSharedKey("&Outside:",MenuOutside));
			}

		break;

	case dlmChange:
		if (tmc == tmcDocFNStartAt)
			{ /* unset Restart # if enter anything other than "1" */
			GetTmcText(tmcDocFNStartAt, sz, cchMaxSz);
			CchStripString(sz, CchSz(sz) - 1);
			if (sz[0] != '\0' && (sz[0] != '1' || sz[1] != '\0'))
				SetTmcVal(tmcDocFNRestartEach, fFalse);
			}
		switch (tmc)
			{
		case tmcDocPageWidth:
		case tmcDocPageHeight:
		case tmcDocDefTabs:
		case tmcDocTMargin:
		case tmcDocBMargin:
		case tmcDocLMargin:
		case tmcDocRMargin:
		case tmcDocFNStartAt:

			bltbyte(rgtmcDoc, rgtmc, sizeof(rgtmcDoc));
			GrayRgtmcOnBlank(rgtmc, itmcMacDoc, tmcOK);
			/* set defaults should always be in same
				state as ok */
			EnableTmc(tmcDocSetDef, FEnabledTmc(tmcOK));
		default:
			break;
			}
		break;

	case dlmClick:
		if (tmc == tmcDocFNRestartEach && 
				ValGetTmc(tmcDocFNRestartEach) != 0)
			{
			SetTmcText(tmcDocFNStartAt, SzSharedKey("1",NumeralOne));
			}
		else  if (tmc == tmcDocSetDef)
			{
			/* ACTION */
			/* failure may be oom, in which case sdm should abort, or
				invalid values, where we should stay in the dialog
			*/
			if ((hcab = HcabFromDlg(fFalse)) == hcabNotFilled)
				return fFalse;
			if (hcab == hcabNull)
				/* sdm filter will take down dialog */
				return (fTrue);

			if (FSetDocDefaultsHcab(hcab))
				if (FEnabledTmc(tmcOK))
					SetFocusTmc(tmcOK);
			}
		else  if (tmc == tmcDocMirror)
			{
			if (wNew == fTrue)
				{
				SetTmcText(tmcDocLMText, SzSharedKey("I&nside:",MenuInside));
				SetTmcText(tmcDocRMText, SzSharedKey("&Outside:",MenuOutside));
				}
			else
				{
				SetTmcText(tmcDocLMText, SzSharedKey("&Left:",MenuLeft));
				SetTmcText(tmcDocRMText, SzSharedKey("&Right:",MenuRight));
				}
			}
		break;

 	case dlmTerm:
		if (tmc != tmcOK)
			break;

		/* failure may be oom, in which case sdm should abort, or
			invalid values, where we should stay in the dialog
		*/
		if ((hcab = HcabFromDlg(fFalse)) == hcabNotFilled)
			return fFalse;
		if (hcab == hcabNull)
			/* sdm will take down dialog */
			return (fTrue);

		if (!FCkMarginsDoc(hcab, &tmc))
			{
			if (tmc != tmcNull)
				{
				SetTmcTxs(tmc, TxsOfFirstLim(0, ichLimLast));
				}
			/* Stay in dialog. */
			return fFalse;
			}
		if (!FCheckDotTmc(hcab, tmcDocTemplate))
			{
			/* Stay in dialog */
			SetTmcTxs(tmcDocTemplate, TxsOfFirstLim(0, ichLimLast));
			return fFalse;
			}

		break;
		}

	return fTrue;
}


/* F  C H E C K  DOT  T M C */
/*  %%Function:  FCheckDotTmc   %%Owner:  bobz       */

FCheckDotTmc (hcab, tmc)
HCAB hcab;
int tmc;

{
	BOOL fOnDisk, fOnLine;
	CHAR stz [ichMaxFile+1];
	CHAR st[ichMaxFile];

	GetCabStz (hcab, stz, ichMaxFile+1, Iag(CABDOCUMENT, hszTemplate));

	if (!FStNormal(stz))
		{
		if (FFindFileSpec(stz, st, grpfpiDot, nfoDot))
			CopySt(st, stz);

		if (!FValidFilename (stz, st, NULL, &fOnDisk, &fOnLine, nfoDot)
				|| !(fOnLine || fOnDisk))
			{
			ErrorEid(eidInvalidDOT, "FCheckDotTmc");
			return fFalse;
			}
		}
	return fTrue;
}


/* C H A N G E  D O T  F R O M  H C A B */
/*  %%Function:  ChangeDotFromHcab   %%Owner:  bobz       */

ChangeDotFromHcab (hcab, doc)
HCAB hcab;
int doc;

{
	int docDot;
	struct STTB **hsttb;
	CHAR stz [ichMaxFile+1], stDot [ichMaxFile];

	if (PdodDoc(doc)->fDoc)
		{
		GetCabStz (hcab, stz, ichMaxFile, Iag(CABDOCUMENT, hszTemplate));
		if (FStNormal(stz))
			docDot = docNil;
		else
			{
			AssertDo(FNormalizeStFile(stz, stDot, nfoDot));
			if ((docDot = DocOpenDot (stDot, fTrue, doc)) == docNil)
				return;
			}

		if ((hsttb = PdodDoc(doc)->hsttbAssoc) != hNil)
			GetStFromSttb(PdodDoc(doc)->hsttbAssoc, ibstAssocDot, stz);
		else
			stz[0]=0;

		if (PdodDoc(doc)->docDot != docDot)
			{
			ChangeDocDot (doc, docDot);
			PdodDoc(doc)->fDirty = fTrue;
			}
		/* allow user to disconnect from template that was not found */
		else  if (docDot == docNil && stz[0] != 0)
			{
			stz[0] = 0;
			AssertH(hsttb);
			/* making same size or smaller, won't fail */
			AssertDo(FChangeStInSttb (hsttb, ibstAssocDot, stEmpty));
			PdodDoc(doc)->fDirty = fTrue;
			}
		}
}




/*  %%Function:  FSetDocDefaultsHcab  %%Owner:  bobz       */

FSetDocDefaultsHcab(hcab)
HCAB hcab;
{
	struct DOD * pdod;
	int grpf;
	int docDot;
	TMC tmcT;

	if (!FCkMarginsDoc(hcab, &tmcT))
		return fFalse;

	pdod = PdodMother(selCur.doc);
	Assert(pdod->fMother);

	/* If there is a document type, set the defaults there */
	if (pdod->fDoc && pdod->docDot != docNil)
		docDot = pdod->docDot;
	else
	/* set them in the global document template */
		docDot = docGlobalDot;

	SetDopInval(docDot, hcab, &grpf);
	return fTrue;
}



/*  %%Function:  FFillHcabDoc  %%Owner:  bobz       */

FFillHcabDoc(hcab, doc)
HCAB hcab;
int doc;
{
	CABDOCUMENT * pcabdoc;
	struct DOP * pdop;
	struct DOD * pdod;
	CHAR st [ichMaxFile];

	FreezeHp();
	pcabdoc = (CABDOCUMENT *) *hcab;

	/* Get the document properties. */
	pdod = PdodDoc(doc);
	Assert(!pdod->fShort); /* fShort does not have dop */
	pdop = &pdod->dop;

	/* Initialize the page width/height. */
	pcabdoc->uDocPageWidth  = pdop->xaPage;
	pcabdoc->uDocPageHeight = pdop->yaPage;

	/* Initialize the margins. */
	pcabdoc->uDocLMargin    = pdop->dxaLeft;
	pcabdoc->uDocRMargin    = pdop->dxaRight;
	pcabdoc->uDocTMargin    = pdop->dyaTop;
	pcabdoc->uDocBMargin    = pdop->dyaBottom;
	pcabdoc->uDocGutter     = pdop->dxaGutter;

	/* Footnote stuff */
	pcabdoc->iFN            = pdop->fpc;
	pcabdoc->iFNStartAt     = pdop->nFtn;
	pcabdoc->fFNRestart     = pdop->fFtnRestart;

	/* Miscellaneous stuff */
	pcabdoc->uDocDefTabs    = pdop->dxaTab;
	pcabdoc->fWidowControl  = pdop->fWidowControl;
	pcabdoc->fMirrorMargins = pdop->fMirrorMargins;

	MeltHp();

	st[0] = 0;

	/* template */
	if (pdod->fDoc)
		{
		if (pdod->docDot != docNil)
			{
			GetDocSt(pdod->docDot, st, gdsoFullPath);
			Assert (st[0]);
			}
		else  if (pdod->hsttbAssoc != hNil)
			GetStFromSttb(pdod->hsttbAssoc, ibstAssocDot, st);
		}

	/* heap pointers may be invalid now */

	FSetCabSt(hcab, st, Iag(CABDOCUMENT, hszTemplate));
	/* fMemFail will be true if FSetCabSt fails */
	return (!vmerr.fMemFail);



}



/*  %%Function:  SetDopFromHcab  %%Owner:  bobz       */

SetDopFromHcab(pdop, hcab)
struct DOP * pdop;
HCAB hcab;
{
	CABDOCUMENT * pcabdoc;

	pcabdoc = (CABDOCUMENT *) *hcab;

	/* Page Size/Orientation */
	pdop->xaPage        = pcabdoc->uDocPageWidth;
	pdop->yaPage        = pcabdoc->uDocPageHeight;

	/* Margins   */
	pdop->dxaLeft       = pcabdoc->uDocLMargin;
	pdop->dxaRight      = pcabdoc->uDocRMargin;
	pdop->dyaTop        = pcabdoc->uDocTMargin;
	pdop->dyaBottom     = pcabdoc->uDocBMargin;
	pdop->dxaGutter     = pcabdoc->uDocGutter;

	/* Footnotes */
	pdop->fpc           = pcabdoc->iFN;
	pdop->nFtn          = pcabdoc->iFNStartAt;
	pdop->fFtnRestart   = pcabdoc->fFNRestart;

	/* Miscellaneous */
	pdop->dxaTab        = pcabdoc->uDocDefTabs;
	pdop->fWidowControl = pcabdoc->fWidowControl;
	pdop->fMirrorMargins = pcabdoc->fMirrorMargins;

	/* note: since we don't do this with sprms and do not keep around
		the old dop, we can't undo this. bz.
	*/

	caHdt.doc = docNil;
	SetUndoNil();
}


/*  %%Function:  GrpfWillDirtyPdopHcab  %%Owner:  bobz       */

int GrpfWillDirtyPdopHcab(pdop, hcab)
struct DOP * pdop;
HCAB hcab;
{
	CABDOCUMENT * pcabdoc;
	int grpf, grpfNoChangeSoFar;

	pcabdoc = (CABDOCUMENT *) *hcab;
	grpf = 0;

	/* Invalidations: We are trying to catch changes which can
						affect the height of all paras in the doc.
						--- from Mac W2 code. */
	/* Note:  Margins checked farther down.... */

	if (pcabdoc->uDocPageWidth != pdop->xaPage ||
			pcabdoc->uDocDefTabs != pdop->dxaTab)
		{
		grpf |= grpfLRDirty;
		}

	/* Check if we dirtied up the document. */

	if (pcabdoc->uDocPageHeight != pdop->yaPage        ||
			pcabdoc->iFN            != pdop->fpc           ||
			pcabdoc->iFNStartAt     != pdop->nFtn          ||
			pcabdoc->fFNRestart     != pdop->fFtnRestart   ||
			pcabdoc->uDocDefTabs    != pdop->dxaTab        ||
			pcabdoc->fMirrorMargins != pdop->fMirrorMargins||
			pcabdoc->fWidowControl  != pdop->fWidowControl
			)
		{
		grpf |= grpfDirty;
		}

	/* check for turning on/off facing pages */

	if (pcabdoc->uDocGutter == 0 && pdop->dxaGutter != 0 ||
			pcabdoc->uDocGutter != 0 && pdop->dxaGutter == 0)
		grpf |= grpfDirty;

	/* Check if we dirtied up the footnote. */
	if (pcabdoc->fFNRestart     != pdop->fFtnRestart   ||
			pcabdoc->iFNStartAt     != pdop->nFtn)
		{
		grpf |= grpfFtn;
		}


	/* Possible margin change checked separately in case preview is
	up and we want to be smart about how much we have to redraw. */

	grpfNoChangeSoFar = (grpf == 0) ? grpfMarginsOnly : 0;
	if (pcabdoc->uDocLMargin + pcabdoc->uDocRMargin + pcabdoc->uDocGutter
			!= pdop->dxaLeft + pdop->dxaRight + pdop->dxaGutter)
		grpf |= grpfLRDirty + grpfNoChangeSoFar;

	if (pcabdoc->uDocTMargin    != pdop->dyaTop        ||
			pcabdoc->uDocBMargin    != pdop->dyaBottom)
		grpf |= grpfDirty + grpfNoChangeSoFar;

	return grpf;
}


/*  %%Function:  FCkMarginsDoc  %%Owner:  bobz       */

BOOL FCkMarginsDoc(hcab, ptmc)
HCAB hcab;
TMC *ptmc;
{
	extern struct CA caSect;
	extern struct SEP vsepFetch;

	CABDOCUMENT * pcabdoc;
	unsigned uWidth;
	int eid;
	int doc;
	CP cp;
	BOOL fOK;

	*ptmc = tmcNull;
	fOK = fTrue;
	eid = eidMTL;
	if (hcab == hNil)
		{
		fOK = fFalse;
		eid = eidNoMemory;
		goto LRet;
		}
	pcabdoc = (CABDOCUMENT *) *hcab;

	if (pcabdoc->uDocPageWidth  == uNinch ||
			pcabdoc->uDocPageHeight == uNinch ||
			pcabdoc->uDocDefTabs    == uNinch ||
			pcabdoc->uDocTMargin    == uNinch ||
			pcabdoc->uDocBMargin    == uNinch ||
			pcabdoc->uDocLMargin    == uNinch ||
			pcabdoc->uDocRMargin    == uNinch ||
			pcabdoc->uDocGutter     == uNinch)
		{
		fOK = fFalse;
		}
	else  if (pcabdoc->uDocPageHeight <=
			abs(pcabdoc->uDocTMargin) + abs(pcabdoc->uDocBMargin))
		{
		fOK = fFalse;
		}
	else
		{
		uWidth = pcabdoc->uDocLMargin + pcabdoc->uDocRMargin +
				pcabdoc->uDocGutter;
		doc = DocMother(selCur.doc);
		for (cp = cp0; cp < CpMacDoc(doc);)
			{
			CacheSect(doc, cp);
			pcabdoc = (CABDOCUMENT *) *hcab;
			if (pcabdoc->uDocPageWidth <=
					uWidth + (vsepFetch.ccolM1 * 
					vsepFetch.dxaColumns))
				{
				fOK = fFalse;
				break;
				}
			else  if (pcabdoc->uDocDefTabs >=
					(pcabdoc->uDocPageWidth -
					(uWidth + vsepFetch.ccolM1 * 
					vsepFetch.dxaColumns)) /
					(vsepFetch.ccolM1 + 1))
				{
				fOK = fFalse;
				*ptmc = tmcDocDefTabs;
				eid = eidTabPosTooLarge;
				break;
				}

			cp = caSect.cpLim;
			}
		}

LRet:
	if (!fOK)
		ErrorEid(eid, "FCkMarginsDoc");

	return fOK;
}



csconst struct
{
	int iag;
	int ibst;
} 


rgiagibstSummaryInfo [] =
{
	{ Iag (CABDOCSUM, hszTitle),      ibstAssocTitle      },
	{ Iag (CABDOCSUM, hszSubject),    ibstAssocSubject    },
	{ Iag (CABDOCSUM, hszAuthor),     ibstAssocAuthor     },
	{ Iag (CABDOCSUM, hszKeyWords),   ibstAssocKeyWords   },
	{ Iag (CABDOCSUM, hszComments),   ibstAssocComments   },

};


#define iMacRgSummaryInfo  5




#define iUnitMax    4


csconst int mputi[] = {
	0, 2, 1, 3, uNinch, uNinch, uNinch};


csconst int mpiut[] = {
	utInch, utCm, utPt, utPica};


#define IFromUt(ut)   (mputi[(ut)]);
#define UtFromI(_i)   (((_i) >= iUnitMax) ? utMax : mpiut[(_i)])
#define IFromIAS(iAS) (iAS)
#define IASFromI(i)   (i)

/*  %%Function:  CmdCustomize  %%Owner:  bobz       */

CMD CmdCustomize(pcmb)
CMB * pcmb;
{
	CABCUSTOMIZE * pcabcustomize;

	if (pcmb->fDefaults)
		{
		pcabcustomize = (CABCUSTOMIZE *) *pcmb->hcab;
		pcabcustomize->sab = 0;
		pcabcustomize->cBtnFldClicks = vcBtnFldClicks;
		pcabcustomize->iAS         = IFromIAS(vpref.iAS);
		pcabcustomize->iUnit       = IFromUt(vpref.ut);
		pcabcustomize->fAutoDelete = vpref.fAutoDelete;
		pcabcustomize->fBkgrndPag  = vpref.fBkgrndPag;
		pcabcustomize->fPromptSI   = vpref.fPromptSI;

		FSetCabSt(pcmb->hcab, vpref.stUsrInitl, 
				Iag(CABCUSTOMIZE, hszInitials));
		FSetCabSt(pcmb->hcab, vpref.stUsrName,
				Iag(CABCUSTOMIZE, hszName));
		/* fMemFail will be true if FSetCabSt fails */
		if (vmerr.fMemFail)
			return cmdNoMemory;

		}

	if (pcmb->fDialog)
		{
		TMC tmc;
		int cBtnFieldClicksSav;
		CHAR dlt [sizeof (dltCustomize)];

		/* local copy of csconst dgd structure */
		BltDlt(dltCustomize, dlt);

		/* 2 is a valid value for cBtnFieldClicks, but not for
			a checkbox, so we kludge things here to keep sdm happy */
		pcabcustomize = (CABCUSTOMIZE *) *pcmb->hcab;
		cBtnFieldClicksSav = pcabcustomize->cBtnFldClicks;
		pcabcustomize->cBtnFldClicks = 0;

		tmc = TmcOurDoDlg(dlt, pcmb);

		((CABCUSTOMIZE *) *pcmb->hcab)->cBtnFldClicks = 
				cBtnFieldClicksSav;

		switch (tmc)
			{
#ifdef DEBUG
		default:
			Assert(fFalse);
			return cmdError;
#endif
		case tmcError:
			return cmdError;

		case tmcCancel:
			return cmdCancelled;

		case tmcOK:
			break;
			}
		}
		
	if (fElActive && pcmb->fCheck)
		{
		char szBuf [ichMaxBufDlg];
		
		GetCabSz(pcmb->hcab, szBuf, sizeof (szBuf), 
			Iag(CABCUSTOMIZE, hszName));
		if (CchStripString(szBuf, CchSz(szBuf) - 1) == 0)
			RtError(rerrIllegalFunctionCall);
		}

	if (pcmb->fAction)
		{
		int ut;

		pcabcustomize = (CABCUSTOMIZE *) *pcmb->hcab;

		if ((uns) (pcabcustomize->cBtnFldClicks - 1) > 1)
			RtError(rerrIllegalFunctionCall);
		vcBtnFldClicks = pcabcustomize->cBtnFldClicks;

		vpref.iAS = IASFromI(pcabcustomize->iAS);

		ut = UtFromI(pcabcustomize->iUnit);
		Assert(ut < utMaxUser);

		if (vpref.ut != ut)
			{
			vpref.ut = ut;

			if (vhwndStatLine)
				{
				InvalidateRect(vhwndStatLine, 
						(LPRECT) NULL, fTrue);
				}

			selCur.fUpdateRuler = fTrue;
			}

		vpref.fAutoDelete = pcabcustomize->fAutoDelete;
		vpref.fBkgrndPag = pcabcustomize->fBkgrndPag;
		vpref.fPromptSI = pcabcustomize->fPromptSI;

		UpdateUserName(pcmb, fTrue /* fFromCustomize */ );
		}

	return cmdOK;
}


/*  %%Function:  FDlgCustomize  %%Owner:  bobz       */

BOOL FDlgCustomize(dlm, tmc, wNew, wOld, wParam)
DLM	dlm;
TMC	tmc;
WORD	wOld, wNew, wParam;
{

    extern CHAR szEmpty[];
	CHAR sz [ichMaxBufDlg + 1];

	switch (dlm)
		{
	default:
		break;
	case dlmInit:
		{
		Assert(HcabDlgCur());
		GetCabSz(HcabDlgCur(), sz, ichMaxBufDlg,
			Iag(CABCUSTOMIZE, hszName));
		GrayBtnOnSzBlank(sz, tmcOK);
		break;
		}
	case dlmChange:
		if (tmc == tmcCustName)
			GrayButtonOnBlank(tmcCustName, tmcOK);
		break;
		}


	return fTrue;
}



/*  %%Function:  FDlgDocStat  %%Owner:  bobz       */

BOOL FDlgDocStat(dlm, tmc, wNew, wOld, wParam)
DLM dlm;
TMC tmc;
WORD wOld, wNew, wParam;
{
	NDD  *pndd;
	pndd = (NDD *) PcmbDlgCur()->pv;
	switch (dlm)
		{
	default:
		break;

	case dlmInit:
		EnableTmc(tmcSTCWords, !pndd->fWordsApprox);
		EnableTmc(tmcSTCPg, !pndd->fPagesApprox);
		EnableTmc(tmcSTUpdate,
				!PdodDoc(pndd->doc)->fLockForEdit && FPrinterOK());
		break;

	case dlmKey:
	/* make ESC close the dialog */
		if (wNew == VK_ESCAPE)
			EndDlg(tmcCancel);
		break;

	case dlmClick:
		if (tmc == tmcSTUpdate)
			{
			CHAR sz [ichMaxNum * 2 + 1]; /* * 2 because of longs */

			/* ignore return value */
			FUpdateDocStats(pndd);

			/* Set Text */
			PlongToSt(&PdodDoc(pndd->doc)->dop.cCh, sz, cchMaxSz);
			StToSzInPlace(sz);
			SetTmcText(tmcSTCch, sz);

			if (!pndd->fWordsApprox)
				{
				PlongToSt(&PdodDoc(pndd->doc)->dop.cWords, 
						sz, cchMaxSz);
				StToSzInPlace(sz);
				SetTmcText(tmcSTCWords, sz);
				}
			EnableTmc(tmcSTCWords, !pndd->fWordsApprox);

			if (!pndd->fPagesApprox)
				{
				PunsToSt(&PdodDoc(pndd->doc)->dop.cPg, 
						sz, cchMaxSz);
				StToSzInPlace(sz);
				SetTmcText(tmcSTCPg, sz);
				}

			EnableTmc(tmcSTCPg, !pndd->fPagesApprox);
			SetFocusTmc(tmcOK);
			/* break; */
			}
		break;
		}

	return fTrue;
}


/*  %%Function:  FUpdateDocStats  %%Owner:  bobz       */


FUpdateDocStats(pndd)
NDD * pndd;
{
	struct RPL rpl;

	/*  assure we have stack space */
	ReturnOnNoStack(8192,fFalse,fTrue); /* ref bug#3233, repag can take lots of stack space */

	StartLongOp();

	SetWords(&rpl, pgnMax, cwRPL);
	rpl.cp = cpMax;
/* BL 11/9/88: do repaginate outside of UpdateStats, to avoid testing stack
	limits inside LbcFormatBlock. */
	if (FRepaginateDoc( pndd->doc, &rpl, patReport ))
		{
		UpdateStats(pndd);
		}

	EndLongOp( fFalse /* fAll */ );

	/* return value indicates stack ok, not success */
	return fTrue;
}


PdttmToStTD(CHAR *, CHAR *, int);
PunsToSt(CHAR *, CHAR *, int);
PlongToSt(CHAR *, CHAR *, int);


csconst struct
{
	int iag;
	int (*pfnFormat)();
	int ofst;
} 


rgsfi[] =
{
	{ Iag(CABDOCSTAT, hszCDate), &PdttmToStTD, 
	offset(DOP, dttmCreated) },
	{ Iag(CABDOCSTAT, hszRDate), &PdttmToStTD, 
	offset(DOP, dttmRevised) },
	{ Iag(CABDOCSTAT, hszRevNum), &PunsToSt, 
	offset(DOP, nRevision) },
	{ Iag(CABDOCSTAT, hszLPDate), &PdttmToStTD, 
	offset(DOP, dttmLastPrint) },
	{ Iag(CABDOCSTAT, hszCPg), &PunsToSt, 
	offset(DOP, cPg) },
	{ Iag(CABDOCSTAT, hszCWords), &PlongToSt, 
	offset(DOP, cWords) },
	{ Iag(CABDOCSTAT, hszCch), &PlongToSt, 
	offset(DOP, cCh) }
	};		


#define isfiMax     7
#define isfiCWords  5
#define isfiCPg	    4


csconst CHAR stApprox[] = StKey(" (approx.)",ApproxDef);

/*  %%Function:  FGetDocSumStatHcab  %%Owner:  bobz       */

FGetDocSumStatHcab(hcabSum, hcabStat, pndd, tmcTitle)
HCAB hcabSum;
HCAB hcabStat;
NDD * pndd;
TMC tmcTitle;
{
	CHAR * stPath;
	CHAR * pch;
	int i;
	struct FNS fns;
	CHAR st [cchMaxSz+1];
	CHAR stFile [ichMaxFile];
	CHAR szClean [ichMaxFile];

	Assert(pndd->hsttbAssoc != hNil);

	if (pndd->stFName == NULL)
		GetDocSt(pndd->doc, stFile, gdsoFullPath);
	else
		CopySt(pndd->stFName, stFile);
	Assert(!PdodDoc(pndd->doc)->fShort);
	if (!pndd->fUseSt && PdodDoc(pndd->doc)->fn == fnNil)
		{
		stPath = fns.stPath;
		stPath[0] = '\0';
		}
	else  if (FNormalizeStFile(stFile, st, PdodDoc(pndd->doc)->fDot?nfoDot:nfoNormal))
		{
		StFileToPfns(st, &fns);

		CopySt(fns.stShortName, stFile);
		StStAppend(stFile, fns.stExtension);

		pch = fns.stPath + *(fns.stPath);
		if (*pch == '\\')
			*pch = '\0';
		stPath = fns.stPath;
		stPath[0] -= 1;
		}
	else
		{
		stFile[0] = '\0';
		stPath = stFile;
		}

	StToSzInPlace(stFile);
	StToSzInPlace(stPath);

#define szFile stFile
#define szPath stPath
	if (hcabSum != hNil)
		{
		SanitizeSz(szFile, szClean, ichMaxFile, fFalse);
		if (!FSetCabSz(hcabSum, szClean, Iag(CABDOCSUM, hszFNameSum)))
			return fFalse;
		SanitizeSz(szPath, szClean, ichMaxFile, fFalse);
		if (!FSetCabSz(hcabSum, szClean, Iag(CABDOCSUM, hszDirSum)))
			return fFalse;

		/*  get all of the strings out of hsttbAssoc */
		for (i = 0; i < iMacRgSummaryInfo; i++)
			{
			GetStFromSttb(pndd->hsttbAssoc,
					rgiagibstSummaryInfo[i].ibst, st);
			if (!FSetCabSt(hcabSum, st, rgiagibstSummaryInfo[i].iag))
				return fFalse;
			}

		if (fElActive && !FDocStatsToCab(hcabSum, 
				pndd, tmcTitle, szPath, szFile, fTrue))
			{
			return fFalse;
			}
		}

	if (hcabStat != hNil)
		{
		if (!FDocStatsToCab(hcabStat, pndd, tmcTitle, szPath, 
				szFile, fFalse))
			{
			return fFalse;
			}
		}
#undef szFile
#undef szPath

	return fTrue;
}


/*  %%Function:  FDocStatsToCab  %%Owner:  bobz       */

BOOL FDocStatsToCab(hcab, pndd, tmcTitle, szPath, szFile, fSumDlg)
HCAB hcab;
NDD * pndd;
TMC tmcTitle;
BOOL fSumDlg;
{

	int i, iagMin;
	CHAR st [cchMaxSz + 1];
	CHAR szClean [cchMaxSz + 1];
	iagMin = fSumDlg ? 
			Iag(CABDOCSUM, hszFName) - Iag(CABDOCSTAT, hszFName) : 0;

	FillStatInfo(pndd, fTrue);


	SanitizeSz(szFile, szClean, cchMaxSz+1, fFalse);
	if (!FSetCabSz(hcab, szClean, iagMin + Iag(CABDOCSTAT, hszFName)))
		return fFalse;
	SanitizeSz(szPath, szClean, cchMaxSz+1, fFalse);
	if (!FSetCabSz(hcab, szClean, iagMin + Iag(CABDOCSTAT, hszDir)))
		return fFalse;


	/* BLOCK */
#define sz st
	GetSzFromSttb(pndd->hsttbAssoc, ibstAssocDot, sz);
	if (sz[0] == 0)
		CchCopySz(szNone, sz);
	else
		QszUpper(sz);
	SanitizeSz(sz, szClean, cchMaxSz+1, fFalse);

	if (!FSetCabSz(hcab, szClean, iagMin + Iag(CABDOCSTAT, hszDOT)))
		return fFalse;

	if (tmcTitle == tmcNull)
		{
		GetStFromSttb(pndd->hsttbAssoc, ibstAssocTitle, st);
		StToSzInPlace(st);
		}
	else	
		GetTmcText(tmcTitle, sz, cchMaxSz);

	SanitizeSz(sz, szClean, cchMaxSz+1, fFalse);
	if (!FSetCabSz(hcab, szClean, iagMin + Iag(CABDOCSTAT,hszTitle)))
		return fFalse;
#undef sz

	GetStFromSttb(pndd->hsttbAssoc, ibstAssocLastRevBy, st);
	if (!FSetCabSt(hcab, st, iagMin + Iag(CABDOCSTAT, hszLRevBy)))
		return fFalse;


		/* BLOCK: get edited time */
		{
		long l;
		struct DOD * pdod;

		pdod = PdodDoc(pndd->doc);
		l = CMinutesFromDttms(pdod->dttmOpened, DttmCur()) +
				pdod->dop.tmEdited;
		PlminToSt(&l, st, cchMaxSz);
		if (!FSetCabSt(hcab, st, iagMin + Iag(CABDOCSTAT, hszEdMin)))
			return fFalse;
		}

		{
		struct DOP * pdop;

		for (i = 0; i < isfiMax; i++)
			{
			pdop = &PdodDoc(pndd->doc)->dop; /* HEAP mvmt inside loop */
			(*rgsfi[i].pfnFormat)
					((CHAR *) pdop + rgsfi[i].ofst, st,
					cchMaxSz);
			if (i == isfiCWords && pndd->fWordsApprox)
				{
				if (pndd->fAppendApprox)
					{
					AppendApproxSt(st);
					}
				}
			if (i == isfiCPg && pndd->fPagesApprox)
				{
				if (pndd->fAppendApprox)
					{
					AppendApproxSt(st);
					}
				}
			if (!FSetCabSt(hcab, st, iagMin + rgsfi[i].iag))
				return fFalse;
			}
		}

	return fTrue;
}


/*  %%Function:  AppendApproxSt  %%Owner:  bobz       */

AppendApproxSt(st)
CHAR	st[];
{
	CHAR	*pch;

	pch = st + st[0] + 1;
	bltbx((CHAR FAR *) (&stApprox[1]), (CHAR FAR *) pch,
			(int) (stApprox[0]));
	st[0] += (int) (stApprox[0]);
}


/*  %%Function:  CmdSummaryInfo  %%Owner:  bobz       */

CMD CmdSummaryInfo(pcmb)
CMB * pcmb;
{
	int doc;
	BOOL fDirty;

	doc = DocMother(selCur.doc);
	Assert (!PdodDoc(doc)->fShort);

	switch (TmcDoDocSummary(pcmb, doc, &fDirty,
			!PdodDoc(doc)->fLockForEdit /* fEnableOK */, NULL, fFalse))
		{
	case tmcOK:
		if (fDirty && fnDMEnum != fnNil)
			DMFlags.dma = dma_StartFromScratch;
		break;

	case tmcError:
		return cmdError;

	case tmcCancel:
		return cmdCancelled;
		}
	return cmdOK;
}


/*  %%Function:  TmcDoDocSummary  %%Owner:  bobz       */

int TmcDoDocSummary(pcmb, doc, pfDirty, fEnableOK, stFile, fUseSt)
CMB * pcmb;
int doc;
BOOL * pfDirty;
BOOL fEnableOK;
CHAR * stFile;
BOOL fUseSt;
{
	CEDSI cedsi;
	int tmc = tmcOK;
	NDD ndd;
	CMB cmb;
	BOOL fCloseDoc = fFalse;

	Assert(doc != docNil);
	Assert(PdodDoc(doc)->fMother);

	*pfDirty = fFalse;

	if (pcmb == NULL)
		{
		cmb.bcm = bcmSummaryInfo;
		if ((cmb.hcab = HcabAlloc(cabiCABDOCSUM)) == hNil)
			{
			tmc = tmcError;
			goto LReturn;
			}
		cmb.cmm = cmmNormal;
		pcmb = &cmb;
		}
	else
		cmb.hcab = hNil;

	SetBytes(&ndd, 0, sizeof(NDD));
LTryAgain:
	ndd.stFName = stFile;
	ndd.doc = doc;
	ndd.fAppendApprox = fFalse;
	ndd.fUseSt = fUseSt;
	ndd.pdop = NULL; /* Protect ourselves from heap movement. */
	if ((ndd.hsttbAssoc = HsttbAssocEnsure(doc)) == hNil)
		{
		tmc = tmcError;
		goto LReturn;
		}

	if (pcmb->fDefaults &&
			!FGetDocSumStatHcab(pcmb->hcab, hNil, &ndd, NULL))
		{
		tmc = tmcError;
		goto LReturn;
		}

	cedsi.fEnableOK = fEnableOK;
	cedsi.pndd = &ndd;
	pcmb->pv = &cedsi;

	Assert(pcmb->bcm == bcmSummaryInfo);

	if (pcmb->fDialog)
		{
		CHAR dlt [sizeof (dltDocSummary)];

		((CABDOCSUM *) *pcmb->hcab)->sab = 0;
		BltDlt(dltDocSummary, dlt);
		if ((tmc = TmcOurDoDlg(dlt, pcmb)) != tmcOK)
			{
			goto LReturn;
			}
		}

	if (pcmb->fAction)
		{

		if (fElActive && !fUseSt)
			{
			/* BLOCK */
				{ 
				char stFileNameOrig [ichMaxFile];
				char stFileName [ichMaxFile];

				GetCabSt(pcmb->hcab, stFileNameOrig,
						sizeof (stFileNameOrig), Iag(CABDOCSUM, hszFNameSum));
				GetCabSt(pcmb->hcab, stFileName, sizeof (stFileName),
						Iag(CABDOCSUM, hszFName));

				if (stFileName[0] != 0 && FNeSt(stFileName, stFileNameOrig))
					{
					if ((doc = DocFromSt(stFileName)) != 
							DocMother(selCur.doc))
						{
						if ((fCloseDoc = doc == docNil) && 
								(doc = DocOpenStDof(stFileName, 
								dofNoWindow, NULL)) == docNil)
							{
							tmc = tmcError;
							goto LReturn;
							}
						}

					stFile = stFileName;
					fUseSt = fTrue;
					goto LTryAgain;
					}
				}

			/* BLOCK */
				{
				char sz [cchMaxSz];

				GetCabSz(pcmb->hcab, sz, sizeof (sz), 
						Iag(CABDOCSUM, hszEdMin));
				if (sz[0] != '\0')
					{
					extern long LDaysFrom1900Dttm();
					struct DTTM dttm;
					long lminCur, lminBack;

					dttm = DttmCur();
					lminBack = WFromSzNumber(sz);
					lminCur = (LDaysFrom1900Dttm(dttm) * 24 + dttm.hr) * 60 +
							dttm.mint;
					PdodDoc(doc)->dttmOpened = dttm;
					PdodDoc(doc)->dop.tmEdited = lminBack;
					}
				}
			}

		if (pcmb->tmc == tmcDSUpdate)
			/*  should be ok even if it fails */
			FUpdateDocStats(&ndd);

/* BL 11/9/88: make action separate function to reduce stack demands on
	gosub to statistics dlg. */
		tmc = TmcDSAction(pcmb, &ndd, pfDirty);

		if (fCloseDoc)
			DisposeDoc(doc);
		}




LReturn:
	if (cmb.hcab != hNil)
	FreeCab(cmb.hcab);

	return tmc;
}


/*  %%Function:  TmcDSAction  %%Owner:  bobz       */

TmcDSAction(pcmb, pndd, pfDirty)
CMB *pcmb;
NDD * pndd;
BOOL * pfDirty;
{
	struct DOD * pdod;
	int ibst, i;
	CHAR st [cchMaxSz+1]; /* buffer for sts going from heap to cab */
	CHAR st2 [cchMaxSz+1];

	for (i = 0; i < iMacRgSummaryInfo; i++)
		{
		GetCabStz(pcmb->hcab, st, cchMaxSz+1, rgiagibstSummaryInfo[i].iag);
		ibst = rgiagibstSummaryInfo[i].ibst;
		GetStFromSttb(pndd->hsttbAssoc, ibst, st2);
		if (FNeSt(st, st2)) /* case sensitive check */
			{
			*pfDirty = fTrue;
			if (!FChangeStInSttb(pndd->hsttbAssoc, ibst, st))
				return tmcError;
			}
		}

	pdod = PdodDoc(pndd->doc);
	if (*pfDirty)
		{
		pdod->fDirty = fTrue;
		pdod->fFormatted = fTrue;
		}
	pdod->fPromptSI = fFalse;
	return tmcOK;
}



/*  %%Function:  FDlgDocSummary  %%Owner:  bobz       */

BOOL FDlgDocSummary(dlm, tmc, wNew, wOld, wParam)
DLM dlm;
TMC tmc;
WORD wOld, wNew, wParam;
{
	CEDSI * pcedsi = (CEDSI *) PcmbDlgCur()->pv;
	switch (dlm)
		{
	case dlmInit:
/* BL 11/9/88: make init separate func, to reduce stack demands on path to
	FGosubDocStat */
		DlgDSInit(pcedsi);

		if (fElActive || vrf.fInQueryEndSession)
			EnableTmc(tmcDSStat, fFalse);

		break;

#ifdef DEBUG
	case dlmExit:
		Assert(pcedsi->fEnableOK || tmc != tmcOK);
		break;
#endif

	case dlmChange:
		if (tmc == tmcDSComments && wNew > 255)
			{
			char sz[256];  /* intentional constant bz */

			Beep();
			vfAwfulNoise = fFalse; /* enable beep for illegal looks keys */
			 /* actually truncate the text */
			GetTmcText(tmc, sz, 256);
		    sz[255] = 0;  /* this does the truncation */
			SetTmcText(tmc, sz);
			 /* ins pt at end */
			SetTmcTxs(tmc, TxsOfFirstLim(ichLimLast, ichLimLast));
			}
		break;

	case dlmClick:
		if (tmc == tmcDSStat)
			{
/* Bug fix: show new title in statistics */
			switch (TmcGosubDocStat(pcedsi->pndd, tmcDSTitle))
				{
#ifdef DEBUG
			default:
			   	ReportSz("FDlgDocStat - unexpected tmc return ignored");
                /* fall thru intentional */
#endif
			case tmcCancel:
			case tmcOK:
				break;

			case tmcError:
				ErrorEid(eidNoMemory, "FDlgDocSummary");
				break;

				}

			SetFocusTmc(FEnabledTmc(tmcOK) ? tmcOK : tmcCancel);

			return fFalse; /* Do not record this button! */
			}
		break;
		}

	return fTrue;
}


/*  %%Function:  DlgDSInit  %%Owner:  bobz       */

DlgDSInit(pcedsi)
CEDSI * pcedsi;
{
	CHAR * pch;
	CHAR sz [ichMaxBufDlg];

	EnableTmc(tmcOK, pcedsi->fEnableOK);
/* shorten path if necessary */
	Assert(HcabDlgCur());
	GetCabSz(HcabDlgCur(), sz, ichMaxBufDlg, Iag(CABDOCSUM, hszDirSum));
	pch = sz;
	if (FTruncateTmcPathPsz(tmcDSDir, &pch))
		SetTmcText(tmcDSDir, pch);
}


/*  %%Function:  TmcGosubDocStat  %%Owner:  bobz       */

TmcGosubDocStat(pndd, tmcTitle)
NDD * pndd;
TMC tmcTitle;
{
	TMC tmc;
	CMB cmb;
	CHAR dlt [sizeof (dltDocStat)];

	/* avoid stack overflow */
	ReturnOnNoStack(2500,tmcCancel,fTrue);

	if ((cmb.hcab = HcabAlloc(cabiCABDOCSTAT)) == hNil)
		return tmcError;

	cmb.cmm = cmmNormal;
	cmb.pv = pndd;
	cmb.bcm = bcmNil;

	if (FGetDocSumStatHcab(hNil, cmb.hcab, pndd, tmcTitle))
		{
		BltDlt(dltDocStat, dlt);
		tmc = TmcOurDoDlg(dlt, &cmb);
		}
	else
		tmc = tmcError;

	FreeCab(cmb.hcab);

	return tmc;
}


/*  %%Function:  FillStatInfo  %%Owner:  bobz       */

FillStatInfo(pndd, fCWords)
NDD	*pndd;
BOOL	fCWords;
{
	int		doc;
	struct DOP	*pdop;
	struct DOD	*pdod;
	struct PLC	**hplcpgd;
	long		cw, cwApprox;

	FreezeHp();
	pdod = PdodDoc(doc = pndd->doc);
	pdop = &(pdod->dop);
	hplcpgd = pdod->hplcpgd;
	if (hplcpgd == hNil)
		{
		pdop->cPg = 1;
		pndd->fPagesApprox = fTrue;
		}
	else
		{
		int		 i, iMac;
		struct PGD	 pgd;

		pdop->cPg = iMac = IMacPlc(hplcpgd);

		pndd->fPagesApprox = fFalse;
		for (i = 0; i < iMac - 1; i++)
			{
			GetPlc(hplcpgd, i, &pgd);
			if (pgd.fUnk)
				{
				pndd->fPagesApprox = fTrue;
				break;
				}
			}
#ifdef DBGYXY
		CommSzNum(SzShared("pndd->fPagesApprox: "), (int) (pndd->fPagesApprox));
#endif
		}

	pdop->cCh = CpMacDocEdit(doc);
	if (pdod->docFtn != docNil)
		{
		pdop->cCh += CpMacDocEdit(pdod->docFtn);
		}

	if (fCWords)
		{
		cw = pdop->cWords - (cwApprox = CApproxWordCch(pdop->cCh));
		if (cw < 0) cw *= -1L;
		pndd->fWordsApprox = fFalse;
		if (!pdop->fExactCWords || cw >= dcwMin)
			{
			pdop->cWords = LRound3L(cwApprox);
			pdop->cWords = LRound3L(cwApprox);
			pdop->fExactCWords = fFalse;
			pndd->fWordsApprox = fTrue;
			}
		}
	MeltHp();
}


/*  %%Function:  LRound3L  %%Owner:  bobz       */

long LRound3L(l)
long	l;
{
	long	div;

	if (l < 1000L)
		{
		div = (100L < l) ? 100L :
				((10L < l) ? 10L : 1L);
		}
	else
		{
		div = 10L;
		while (l / div >= 1000L)
			{
			div *= 10L;
			}
		}
	return (div * ((l + (div / 2)) / div));
}


	typedef {
	int	doc;
	int	fmstCountingDoc;
	long	cpMac;
}	


CID;

/*  %%Function:  UpdateStats  %%Owner:  bobz       */

UpdateStats(pndd)
NDD	*pndd;
{
	long		cWords;
	CP		cp, cpNextRpt;
	CHAR		*pch;
	CHAR		*pchLim;
	CHAR		*pchHyph;
	BOOL		fSpace;
	int		doc;
	GRPFVISI grpfvisi;
	struct DOD	*pdod;
	int		icidMac, icid;
	CID		rgcid[2], *pcid;
	struct PPR    **hppr;
	extern struct FLI       vfli;
	extern struct PRI       vpri;
	extern struct PRSU		vprsu;
	extern int				vflm;

	doc = pndd->doc;
	FillStatInfo(pndd, fFalse);

	/* count words in main document + footnotes, if any. No footnotes or annotations. */
	pcid = &rgcid[0];
	pcid->doc = doc;
	pcid->fmstCountingDoc = fTrue;
	pcid->cpMac = CpMacDocEdit(doc);
	rgcid[1].doc  = PdodDoc(doc)->docFtn;
	icidMac = 1;
	if ((pcid = &rgcid[1])->doc != docNil)
		{
		icidMac = 2;
		pcid->fmstCountingDoc = fFalse;
		pcid->cpMac = CpMacDocEdit(pcid->doc);
		}
	cWords = 0L;

	/* visi conditions are set up as for printing, since we want to
	   count words in the printed document */

	grpfvisi.w = 0;
	grpfvisi.grpfShowResults = ~0;
	grpfvisi.flm = flmRepaginate;
	grpfvisi.fSeeHidden = vprsu.fSeeHidden;
	grpfvisi.grpfShowResults = vprsu.fShowResults ? ~0 : 0;
	grpfvisi.fForceField = fTrue;
	PwwdWw(wwTemp)->grpfvisi = grpfvisi;
	Debug( PwwdWw(wwTemp)->hdc = NULL );/* so we RIP in case it matters, which it shoudn't */

	for (icid = 0, pcid = &rgcid[0]; icid < icidMac; icid++, pcid++)
		{
		if (pcid->cpMac == 0)
			{
			continue;
			}
		hppr = HpprStartProgressReport(pcid->fmstCountingDoc ?
				mstCountingDoc : mstCountingDocFtn, 
				NULL, nIncrPercent, fFalse);
		ProgressReportPercent(hppr, cp0, pcid->cpMac, cp0, &cpNextRpt);
		LinkDocToWw(pcid->doc, wwTemp, wwNil);

		for (cp = cp0; cp < pcid->cpMac; cp = vfli.cpMac)
			{{
			if (cp >= cpNextRpt)
				{
				ProgressReportPercent(hppr, cp0, pcid->cpMac,
						cp, &cpNextRpt);
				}
			FormatLine(wwTemp, pcid->doc, cp);

/* FUTURE: this block of code is almost identical to UpdateDsi
			in print.c - should call that instead if possible */
#ifndef SWAPTUNE
			pch = vfli.rgch;
			pchLim = &vfli.rgch[vfli.ichMac];
			pchHyph = pch;
			fSpace = fTrue;
			while (pch < pchLim)
				{
				switch (*pch++)
					{
				case chSpace:
				case chNonBreakSpace:
				case chEop:
#ifdef CRLF
				case chReturn:
#endif
				case chSect:
				case chTable:
				case chTab:
				case chCRJ:
				case chPicture:
					fSpace = fTrue;
					break;
				case chHyphen:
					pchHyph = pch;
					/* fall through */
				default:
					if (fSpace)
						{
						cWords++;
						fSpace = fFalse;
						}
					break;
					}
				}
			if (pchHyph == pchLim)
				{
				cWords--; /* Hyphen at the end of line. */
				}
#endif 
			if (FQueryAbortCheck())
				return;
			}}
		ChangeProgressReport(hppr, 100);
		StopProgressReport(hppr, pdcRestoreImmed);
		}

	pndd->fWordsApprox = fFalse;
	pdod = PdodDoc(doc);
	pdod->fDirty = fTrue;
	pdod->dop.cWords = cWords;
	pdod->dop.fExactCWords = fTrue;
	/* Calculate the ratio. */
	if (cWords > 0L)
		{
		if (pdod->dop.cCh * 10L < pdod->dop.cCh)
			{
			cchAvgWord10 = (int) ((pdod->dop.cCh / cWords) * 10L);
			}
		else
			{
			cchAvgWord10 = (int) ((pdod->dop.cCh * 10L) / cWords);
			}
		}
}


/*  %%Function:  CApproxWordCch  %%Owner:  bobz       */

long CApproxWordCch(cch)
long	cch;
{
	if (cch * 10L > cch)
		return ((cch * 10L) / cchAvgWord10);
	else
		return ((cch / cchAvgWord10) * 10L);
}


/*  %%Function:  SetDopInval  %%Owner:  bobz       */

SetDopInval(doc, hcab, pgrpf)
int doc;
HCAB hcab;
int *pgrpf;
{
	int grpf;
	extern struct FLI vfli;	/* For InvalFli */

	{
	struct DOP * pdop;

	FreezeHp();
	pdop = &(PdodDoc(doc)->dop);
	grpf = GrpfWillDirtyPdopHcab(pdop, hcab);
	MeltHp();
	SetDopFromHcab(pdop, hcab);
	}

	/* Check if we dirtied up the document. */
	if (grpf != 0)
		{
		struct DOD * pdod = PdodDoc(doc);
		struct CA ca;

		FreezeHp();
		pdod->fDirty = pdod->fFormatted = fTrue;

		/* Force an update. */
		InvalPageView(doc);

		/* blow away and recalculate plcphe and plcpgd on next repag */
		pdod->fEnvDirty = fTrue;

		PcaSet(&ca, doc, cp0, CpMacDoc(doc));
		if (grpf & grpfLRDirty)
			{
			pdod->fLRDirty = fTrue;
			InvalDoc(doc);
			InvalTableProps(doc, cp0, CpMacDoc(doc), fTrue);
			InvalFli();
			}
		else  if (grpf & grpfFtn && pdod->docFtn != docNil)
			{
			Assert(pdod->hplcfrd != hNil);
			InvalCp(&ca);
			InvalCp(PcaSet(&ca, pdod->docFtn, cp0, 
					CpMacDoc(pdod->docFtn)));
			}
		if (doc == DocMother(selCur.doc))
			selCur.fUpdatePap = fTrue;
		MeltHp();
		}
	*pgrpf = grpf;
}


