#define NOGDICAPMASKS
#define NOICON
#define NOKEYSTATE
#define NOSYSCOMMANDS
#define NOMENUS
#define NORASTEROPS
#define NOSYSMETRICS
#define NOCLIPBOARD
#define NODRAWTEXT
#define NOMETALFILE
#define NOVIRTUALKEYCODES
#define NOREGION
#define NOSOUND
#define NOKANJI
#define NOCOMM
#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "ch.h"
#include "props.h"
#include "doc.h"
#include "sel.h"
#include "disp.h"
#include "prm.h"
#include "layout.h"
#include "resource.h"
#include "debug.h"
#define HEADER
#define NOIDISAVEPRINT
#define NOSTRUNDO
#include "field.h"
#include "dlbenum.h"
#include "cmd.h"
#include "message.h"
#include "error.h"
#include "rareflag.h"
#include "screen.h"
#include "scc.h"
#include "inter.h"

#include "idd.h"
#include "sdmdefs.h"
#include "sdmver.h"
#include "sdm.h"
#include "sdmtmpl.h"
#include "sdmparse.h"

#include "header.hs"
#include "header.sdm"
#include "inspgnum.hs"
#include "inspgnum.sdm"

#include "ibdefs.h"



/* sdm globals for dialog handling */
extern HWND             vhwndApp;
extern struct STTB **   vhsttbWnd;
extern int              wwCur;
extern int              mwCur;
extern struct MWD       **hmwdCur;
extern int              vdocHdrEdit;
extern int              vdocTemp;
extern struct WWD       **hwwdCur;
extern struct SEP       vsepFetch;
extern struct DOD       **mpdochdod[];
extern BOOL             vfSeeSel;
extern struct SEL       selCur;
extern int              vssc;
extern struct CA        caSect;
extern struct WWD       **mpwwhwwd[];
extern struct MERR      vmerr;
extern struct MWD       **mpmwhmwd[];
extern struct PREF      vpref;
extern struct UAB       vuab;
#ifdef DEBUG
extern int              cHpFreeze;
#endif
extern CHAR             rgchEop[];
extern BOOL		vfRecording;
extern struct ESPRM     dnsprm[];
extern int              docMac;
extern int          	vdocScratch;
extern int              vihdtNew;
extern struct PAP       vpapFetch;

extern CP CpMomFromHdr();


csconst char mpipgnfc[] = {
	nfcArabic, nfcLCLetter, nfcUCLetter,
			nfcLCRoman, nfcUCRoman};


/* nfcArabic nfcUCRoman nfcLCRoman nfcUCLetter nfcLCLetter */
csconst char mpnfcipg[] = {        
	0,         4,         3,          2,          1};


/* SPrm and ninch Type */
typedef struct _spt
	{
	char sprm;
	int  wNinchVal;
} SPT;

csconst SPT mpiagsptSep[] =
{
/* map is dependent on the order of the CABHEADER  entries above */
	/* KLUDGE: overloaded cab entry for benefit of NewPropToCab() */
	sprmSFPgnRestart,	uNinch,		/* uHdrLB -- doubles as fRestartPgn */
	sprmSPgnStart,	uNinch,		/* uHdrPgStartAt */
	/* KLUDGE: overloaded cab entry for benefit of NewPropToCab() */
	sprmSNfcPgn,	uNinch,		/* iHdrPgFormat -- doubles as nfc */
	sprmSDyaHdrTop,	uNinch,		/* uHdrFromTop */
	sprmSDyaHdrBottom,	uNinch,		/* uHdrFromBottom */
	sprmSFTitlePage,	uNinchCheck,	/* fHdrFirstPage */
	sprmNoop,		uNinchCheck,	/* fHdrFacingPage */ /* from dop */
};


#define iagSepMac (sizeof (mpiagsptSep) / sizeof (SPT))


typedef struct /* for the list box in the Edit Header dialog box */
	{
	struct
		{
		unsigned char fHeader : 1;
		unsigned char fFooter : 1;
		unsigned char fFirstHeader : 1;
		unsigned char fFirstFooter : 1;
		unsigned char fEvenHeader : 1;
		unsigned char fEvenFooter : 1;
		unsigned char fOddHeader : 1;
		unsigned char fOddFooter : 1;
		};
	} HTD; /* Header Type Description */

HTD vhtdFirstFacingPage = {
		/* fFirstPage && fFacingPage */ { 0, 0, 1, 1, 1, 1, 1, 1 }
	};							


/*
		!fFirstPage && !fFacingPage   {1, 1, 0, 0, 0, 0, 0, 0}
		by ~vhtdFirstFacingPage
*/

HTD vhtdFirstPage = {
		/* fFirstPage && !fFacingPage*/ { 1, 1, 1, 1, 0, 0, 0, 0 }
	};					


/*
		!fFirstPage && fFacingPage
		by ~vhtdFirstPage             {0, 0, 0, 0, 1, 1, 1, 1}
*/

unsigned char vhtdHdrListBox;

csconst int mpIhtstIhdt[] = {
	ihdtTRight, /* Header */
	ihdtBRight, /* Footer */
	ihdtTFirst, /* First Header */
	ihdtBFirst, /* First Footer */
	ihdtTLeft,  /* Even Header */
	ihdtBLeft,  /* Even Footer */
	ihdtTRight, /* Odd Header */
	ihdtBRight  /* Odd Footer */
};


csconst int mpIFtnSepIhdt[] = {
	ihdtTFtn,     /* Footnote Separator */
	ihdtTFtnCont, /* Footnote Cont. Separator */
	ihdtBFtnCont  /* Footnote Cont. Notice */
};


#define ihtstMax (8)
#define StCS(s) StMap(s, 1)

#define ihtstHdr        0
#define ihtstFtr        1
#define ihtstFirstHdr   2
#define ihtstFirstFtr   3
#define ihtstEvenHdr    4
#define ihtstEvenFtr    5
#define ihtstOddHdr     6
#define ihtstOddFtr     7


csconst CHAR rghtst [][]=
{
	StKey("Header",HtHeader),                  /* ihtstHdr */
	StKey("Footer",HtFooter),                  /* ihtstFtr */
	StKey("First Header",FirstHeader),       /* ihtstFirstHdr */
	StKey("First Footer",FirstFooter),       /* ihtstFirstFtr */
	StKey("Even Header",EvenHeader),         /* ihtstEvenHdr */
	StKey("Even Footer",EvenFooter),         /* ihtstEvenFtr */
	StKey("Odd Header",OddHeader),           /* ihtstOddHdr */
	StKey("Odd Footer",OddFooter)            /* ihtstOddFtr */
};


csconst CHAR rgFtnSepSt [][] =
{
	StKey("Footnote Separator",FootnoteSeparator),
			StKey("Footnote Cont. Separator",FootnoteContSeparator),
			StKey("Footnote Cont. Notice",FootnoteContNotice)
};


#undef StCS

/* radio button index for Insert Page Number */
#define iTypeTop    0
#define iTypeBottom 1

#define iPosLeft    0
#define iPosCenter  1
#define iPosRight   2

/* ***
Facts about header/footer and footnote separators

.. the window that displays the individual header/footer text for editing
	is docHdrDisp, it has back pointer to its mother doc.
.. docHdrDisp is fDispHdr (which is fShort).
.. the mother doc points to docHdr which has ALL the header/footer text and
	footnote separators.
.. docHdr is fShort and fHdr, it is never displayed.
.. docHdr has a hplchdd that stores all the cp ranges of where the footnote
	separators and header/footer text are.
.. ihdd is the index into hplchdd.
.. the footnote separators (if exist) always come first, then the
	header/footer for each section (in ihdt order).
.. grpfIhdt in dop is for footnote separators, they occupied bit column 0, 1, 2.
.. grpfIhdt in sep is for header/footer for that section, they occupied bit column
	0, 1, 2, 3, 4, 5.
.. to find a ihdd, you have to start from the dop's grpfIhdt, then CacheSect
	cp0 for each sep's grpfIhdt, each occurance of a bit in the grpfIhdt is
	counted and summed to get a ihdd.
.. ihdt is used to distinguish the 6 possible types of header and footer and
	the 3 possible footnote separators.
.. ihdtFoo is the column number within grpfIhdt
*** */


/* %%Function:GetCaHdrMom %%Owner:chic */
GetCaHdrMom(pca, ww)
struct CA *pca;
int ww;
{
	struct WWD *pwwd = PwwdWw(ww);
	struct CA *pcaSrc;
	CP cp;
	int wk;

	FreezeHp();
	wk = pwwd->wk;

	Assert(wk == wkDoc || wk == wkHdr || wk == wkPage || wk == wkOutline);
	if (wk == wkDoc || wk == wkOutline)
		{
		pcaSrc = ((ww == wwCur) ? &selCur.ca : &(pwwd->sels.ca));
		blt(pcaSrc, pca, cwCA);
		pca->cpLim = pca->cpFirst;
		}
	else
		{
		pca->doc = DocMother(DocBaseWw(ww));
		if (wk == wkHdr)
			{
			Assert(PmwdWw(ww)->fSplit);
			wk = PwwdWw(ww = WwOther(ww))->wk;
			pwwd = PwwdWw(ww);
			}
		if (wk == wkPage)
			cp = CpMin(CpPlc(PdodDoc(pca->doc)->hplcpgd, pwwd->ipgd), CpMacDocEdit(pca->doc));
		else
			{
			pcaSrc = ((ww == wwCur) ? &selCur.ca : &(pwwd->sels.ca));
			cp = pcaSrc->cpFirst;
			}
		pca->cpFirst = pca->cpLim = cp;
		}

	Assert(pca->doc >= docMinNormal && pca->doc < docMac);
	Assert(PdodDoc(pca->doc)->fMother);
	Assert(pca->cpFirst >= cp0 && pca->cpFirst <= CpMacDocEdit(pca->doc));
	MeltHp();
}


/* %%Function:CmdInsHeader %%Owner:chic */
CMD CmdInsHeader(pcmb)
CMB * pcmb;
{
	CABHEADER *pcabheader;
	int wk = PwwdWw(wwCur)->wk;
	struct CA caMom;
	SPT mpiagspt [sizeof (mpiagsptSep) / sizeof (SPT)];
	struct SELS selsMom;
	struct SELS selsSave;
	struct SELS *pselNow;
	int iHdrLB;
	CMD cmd = cmdOK;
	BOOL fRestartPgn;
	int nfc;
	int ihdt;
	BOOL f1stPageSave = fFalse;
	BOOL fFacePageSave = fFalse;
	BOOL fSetDocHdrEdit, fChangeWw = fFalse;

/* only allow edit header in doc window and header window */
	if (!(wk == wkDoc || wk == wkPage || wk == wkHdr || wk == wkOutline))
		{
		Beep();
		return cmdError;
		}

	if (!(*hmwdCur)->fSplit && !(*hwwdCur)->fPageView && !FCanSplit(hmwdCur, wkHdr))
		{
		ErrorEid(eidWindowTooSmall,"CmdInsHeader");
		return cmdError;
		}

	bltbx((CHAR FAR *)mpiagsptSep, (CHAR FAR *)mpiagspt,
			sizeof (mpiagsptSep));
/* Save away the mother doc and cp range */
	GetCaHdrMom(&caMom, wwCur);

/* force selection in mother doc so that section properties are applied to mother */
	if (wk == wkHdr)
		{
		fChangeWw = fTrue;
		Assert (vssc == sscNil);
		pselNow = PselActive();
		TurnOffSel(pselNow); /* avoid ghost cursor left behind */
		NewCurWw(WwOther(wwCur), fFalse /*fDoNotSelect*/);
		wk = PwwdWw(wwCur)->wk;
		}

	if (wk == wkPage)
		{
		/* if vssc != sscNil, selCur is not the currently active
		selection, and it could have the wrong doc and ww in it.
		However, dyadic operations that set vssc should have been
		cancelled by this command, and so we should not run into
		trouble. At any rate, we save and restore the result of
		PSelActive, not selCur so we should avoid problems if later
		uses are set up properly.    bz
		*/
		Assert (vssc == sscNil);
		pselNow = PselActive();
		if (pselNow->doc != caMom.doc || 
		/* in case selcur is in a different section than the cpFirst of the showing page */
		(CacheSect(caMom.doc, caMom.cpFirst), !FInCa(pselNow->doc, pselNow->cpFirst, &caSect)))
			{
			TurnOffSel(pselNow);
			pselNow->doc = caMom.doc;
			pselNow->sk = skNil;
			SelectIns(pselNow, caMom.cpFirst);
			}
		}


	if (pcmb->fDefaults)
		{
		CacheSect(caMom.doc, caMom.cpFirst);

		NewPropToCab(pcmb->hcab, &vsepFetch, mpiagspt);

		pcabheader = (CABHEADER *) *pcmb->hcab;
		pcabheader->sab = 0;

	/* KLUDGE: overloaded cab entry for benefit of NewPropToCab() */
		fRestartPgn = pcabheader->uHdrLB;
		pcabheader->uHdrLB = 0;

	/* KLUDGE: overloaded cab entry for benefit of NewPropToCab() */
		nfc = pcabheader->iHdrPgFormat;

		if (nfc < 0 || sizeof (mpnfcipg) <= nfc)
			{
			pcabheader->iHdrPgFormat = wNinch;
			}
		else
			{
			pcabheader->iHdrPgFormat = (int) (mpnfcipg[nfc]);
			}

		pcabheader->fHdrFacingPage = PdodDoc(caMom.doc)->dop.fFacingPages;
		fFacePageSave = pcabheader->fHdrFacingPage;
		f1stPageSave = pcabheader->fHdrFirstPage;
		GenHdrListBox(f1stPageSave, fFacePageSave);
		}

	if (pcmb->fDialog)
		{
		char dlt [sizeof (dltHeader)];

		BltDlt(dltHeader, dlt);
		vrf.fPgv = fChangeWw ? fFalse : (*hwwdCur)->fPageView;

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
			return cmdCancelled;

		case tmcOK:
			break;
			}
		}

	if (!pcmb->fAction)
		return cmdOK;

	StartLongOp();
	pcabheader = (CABHEADER *) *pcmb->hcab;
	GenHdrListBox(pcabheader->fHdrFirstPage, pcabheader->fHdrFacingPage);

	if (pcabheader->uHdrLB == uNinch)
		pcabheader->uHdrLB = 0; /* just to be safe */

	/* convert blank to zero */
	if (pcabheader->uHdrPgStartAt == wNinch)
		pcabheader->uHdrPgStartAt = 0;
	else  if (pcabheader->uHdrPgStartAt >= 1)
		fRestartPgn = fTrue;
	else  
		fRestartPgn = fFalse;

	if (pcabheader->iHdrPgFormat >= 0 &&
			pcabheader->iHdrPgFormat < sizeof(mpipgnfc))
		{
		nfc = (int) (mpipgnfc[pcabheader->iHdrPgFormat]);
		}
	else
		{
		nfc = nfcNil;
		}

	/* KLUDGE: overloaded cab entry for benefit of NewPropToCab() */
	iHdrLB = (*(HCABHEADER) pcmb->hcab)->uHdrLB;
	(*(HCABHEADER) pcmb->hcab)->uHdrLB = fRestartPgn;
	(*(HCABHEADER) pcmb->hcab)->iHdrPgFormat = nfc;

	if (!FNewPropFromRgProp(pcmb->hcab, fTrue /* fCab */, mpiagspt))
		{
		EndLongOp(fFalse);
		return cmdCancelled;
		}
	pcabheader = *pcmb->hcab;
	if (PdodDoc(caMom.doc)->dop.fFacingPages != pcabheader->fHdrFacingPage)
		{
		DirtyDoc(caMom.doc);
		PdodDoc(caMom.doc)->dop.fFacingPages = pcabheader->fHdrFacingPage;
		}
	fSetDocHdrEdit = (fFacePageSave != pcabheader->fHdrFacingPage) ||
			(f1stPageSave != pcabheader->fHdrFirstPage);

	if (!fChangeWw && (*hwwdCur)->fPageView)
		{
		if ((cmd = CmdPageViewHeader(mpIhtstIhdt[iHdrLB] /* ihdt */, NULL)) == cmdOK)
			SetUndoNil();
		}
	else  if (!FOpenHeaderIhtst(IhtstFromIhdrLB(iHdrLB), fTrue))
		cmd = cmdNoMemory;

	if (fSetDocHdrEdit)
		vdocHdrEdit = caMom.doc; /* to refresh header iconbar text */

	EndLongOp(fFalse /*fAll*/);

	return cmd;
}




/*  DlgfHeader
	This function is called by the edit header/footer dialog.
	On initialization, it fills the list box with listing of
	available header/footer types depending on the First Page Special
	and Facing Pages values.
	The two check boxes - First page and Facing pages dominate
	what is shown in the list box.
	The link with previous check box is changed according
	to which header type is selected in the list box,
	it is checked by default for sections other than the 1st section
	unless there is already an entry for that header type.
	For first section, the link with previous is always grayed and unchecked.
	When Edit button is pushed, the dialog disappears and a window for
	the selected header/footer type is brought up to be edited.
*/

/* %%Function:FDlgHeader %%Owner:chic */
BOOL FDlgHeader(dlm, tmc, wNew, wOld, wParam)
DLM dlm;
TMC tmc;
WORD wOld, wNew, wParam;
{
	CABHEADER *pcabheader;


	switch (dlm)
		{
	case dlmClick:
		if (tmc == tmcHdrOption)
			{
			 /* FSetDlgSab not safe if dialog coming down */
			if (FIsDlgDying() || !FSetDlgSab(sabHDROptions))
				{
				Beep();
				return fFalse;
				}
			SetFocusTmc(tmcHdrPgStartAt);
			EnableTmc(tmcHdrOption, fFalse);
			return fFalse; /* so it doesn't get recorded! */
			}
		else  if (tmc == tmcHdrFirstPage || tmc == tmcHdrFacingPage)
/* set up list box based on setting of first page and facing page */
			UpdateHdrLB();
		else  if (tmc == tmcHdrLB)
/* disable OK if click beyond last item in list box */
			EnableTmc(tmcOK, (int)wNew >= 0);
		break;
		} /* end of switch */

	return fTrue;
}


/* %%Function:WListHeader %%Owner:chic */
EXPORT WORD WListHeader(tmm, sz, isz, filler, tmc, wParam)
TMM tmm;
char * sz;
int isz;
WORD filler;
TMC tmc;
WORD wParam;
{

	/* this function is only used the first time the box is filled */

	if (tmm == tmmCount)
		UpdateHdrLB();

	return 0;
}




/* %%Function:FInFirstSection %%Owner:chic */
FInFirstSection(doc, cp)
int doc;
CP cp;
{
	Assert(!PdodDoc(doc)->fShort);
	CacheSect(doc, cp);
	return (caSect.cpFirst == cp0);
}



/* %%Function:UpdateHdrLB %%Owner:chic */
UpdateHdrLB()
	/* Update header type list box in Edit header dialog */
{
	BOOL fFirstPage = ValGetTmc(tmcHdrFirstPage);
	BOOL fFacingPage = ValGetTmc(tmcHdrFacingPage);
	int ihtst;
	unsigned int fMask = 1;
	CHAR st[64];

	StartListBoxUpdate(tmcHdrLB);
/* NOTE : assumed BOOL to 1 or 0 */
	if (vrf.fPgv)
		{
/* in pageview mode, list box always display simple header/footer */
		fFirstPage = fFacingPage = fFalse;
		}

	GenHdrListBox(fFirstPage, fFacingPage);

	for (ihtst = 0; ihtst < ihtstMax; ihtst++)
		{
		if (vhtdHdrListBox & fMask)
			{
			CopyCsSt(rghtst[ihtst], st);
			StToSzInPlace(st);
			AddListBoxEntry(tmcHdrLB, st);
			}
		fMask <<= 1;
		}

	EndListBoxUpdate(tmcHdrLB);
	SetTmcVal(tmcHdrLB, 0);
	SetFocusTmc(tmcHdrLB);

}




/* %%Function:GenHdrListBox %%Owner:chic */
GenHdrListBox(fFirstPage, fFacingPage)
int fFirstPage;
int fFacingPage;
{
/* Generate contents for the header type list box */

	switch (fFirstPage + fFacingPage)
		{
	case 0:
		vhtdHdrListBox = ~((unsigned char)vhtdFirstFacingPage);
		break;
	case 1:
		vhtdHdrListBox = (fFirstPage ?
				vhtdFirstPage : ~((unsigned char)vhtdFirstPage));
		break;
	case 2:
		vhtdHdrListBox = (unsigned char)vhtdFirstFacingPage;
		break;
	default :
		Assert(fFalse);
		break;
		}
}




/* %%Function:IhtstFromIhdrLB %%Owner:chic */
IhtstFromIhdrLB(iHdrLB)
int iHdrLB;
{
/* From the ith item in the header type list box, find out the
	index to the header type string.
	The header types showing in the list box are indicated
	by a 1 in the vhtdHdrListBox in the corresponding bit position.
*/
	int ihtst;
	int iLB = 0;
	unsigned int fMask = 1;
	int ihtstLast;

	Assert(iHdrLB >= 0);
	for (ihtst = 0; ihtst < ihtstMax; ihtst++, fMask <<= 1)
		{
		if (vhtdHdrListBox & fMask)
			{
			ihtstLast = ihtst;
			if (iLB++ == iHdrLB)
				break;
			}
		}
	if (ihtst >= ihtstMax)
		ihtst = ihtstLast;
	return(ihtst);
}




/* F O P E N  H E A D E R  I H T S T */
/* open header ihdt from the document in the current window.
Plan:
	0. check if current ww holds fDoc doc. save doc, cp from selCur.
	1. if current mw is not split, create a hdr split pane
		else if lower pane is hdr, save contents
	2. make lower pane current.
	3. load ihdd from cp/ihdt into lower pane.
*/
/* %%Function:FOpenHeaderIhtst %%Owner:chic */
FOpenHeaderIhtst(ihtst, fEditHdr)
int ihtst; /* index to header type or footnote separator string table */
int fEditHdr; /* False ==> edit footnote separator */
{
	int ihdt = fEditHdr ? mpIhtstIhdt[ihtst] : mpIFtnSepIhdt[ihtst];
	int docHdr, docHdrDisp;
	int wwHdr;
	BOOL fRecordingSav ;
	struct CA caMom;
	extern struct SCC  vsccAbove;


/* This routine can be called when edit header in mother doc, header doc,
	edit footnote separator in mother doc, footnote doc.
*/
	if (ihdt < ihdtMaxSep)
		{ /* header */
		GetCaHdrMom(&caMom, wwCur);
		}
	else  if (!FReportSplitErr(wkHdr)) /* catch EL statements and ftn sep from ftn db */
		{ /* footnote separator */
		caMom.doc = DocMother(selCur.doc);
		caMom.cpFirst = cp0;
		}
	else  
		return fFalse;

/* create the header subdoc - the one that stores all header/footer text */
	if ((docHdr = PdodDoc(caMom.doc)->docHdr) == docNil &&
			(docHdr = DocCreateSub(caMom.doc, dkHdr)) == docNil)
		return fFalse;

/* create hplchdd and stored into dod (by position of edc) */
	if (PdodDoc(docHdr)->hplchdd == hNil &&
			HplcCreateEdc(mpdochdod[docHdr], edcHdd) == hNil)
		/* assume we just created docHdr and destroy it */
		{
		DisposeDoc(docHdr);
		PdodDoc(caMom.doc)->docHdr = docNil;
		return fFalse;
		}

/* save existing text in header window, if any or create header window */
	if (!FCleanUpHdr(caMom.doc, fFalse, fFalse))
		return fFalse;


/* get a docHdrDisp, create one if not already there */
/* docHdrDisp is the doc that shows individual header/footer text for
	editing */
	if ((docHdrDisp = DocDisplayHdr(caMom.doc, caMom.cpFirst, ihdt)) == docNil)
		return fFalse;
/* This prevents being disposed during FCleanUpHdr before it has a ww refer to it */
	PdodDoc(docHdrDisp)->fDoNotDispose = fTrue; 

/* now docHdrDisp is either 
1. a newly created fDispHdr doc
2. an existing fDispHdr doc that displays the same header we are going 
	to show
*/

/* bring in the header text */
	NewTextHdr(caMom.doc, caMom.cpFirst, docHdrDisp, ihdt, wwNil, fTrue /*fSelect*/);
	InhibitRecorder(&fRecordingSav, fTrue);
	if (!FShowFtnAtn(docHdrDisp, wkHdr))
		{
		PdodDoc(docHdrDisp)->fDoNotDispose = fFalse;
		InhibitRecorder(&fRecordingSav, fFalse);
		FCleanUpHdr(caMom.doc, fFalse, fFalse);
		return fFalse;
		}
	PdodDoc(docHdrDisp)->fDoNotDispose = fFalse;
	InhibitRecorder(&fRecordingSav, fFalse);

	wwHdr = (*hmwdCur)->wwLower;
/* called in FCreateSplit or FShowFtnAtn to avoid unnecessary icons showing up 
then get hidden. 
	SetupIconBarButton(wwHdr, caMom.doc, caMom.cpFirst, ihdt);
*/
	SetHdrIBarName(docHdrDisp, ihtst, wwHdr);

	FCleanUpHdr(caMom.doc, fFalse, fFalse);
	SetUndoNil();
	SelectIns(&selCur, cp0);
	if (!(*hwwdCur)->fPageView)
		SynchSccWw(&vsccAbove, wwHdr);
	return fTrue;
}




/***************************/
/* G e t  H e a d e r  S t */
/* get header description part :
i.e. [First|Odd|Even] Header | Footer [(S#)]
or   Footnote [Cont] Separator | Notice
and append to the end of pst, fixing cch in st[0] also.
*/
/* %%Function:GetHeaderSt %%Owner:chic */
GetHeaderSt(doc, ihtst, st, cchMax)
int doc;
int ihtst;
char *st;
int cchMax;
{
	char *pch;
	struct PLC **hplcsed;
	int ihdt = PdodDoc(doc)->ihdt;
	int ised = PdodDoc(doc)->ised;

	Assert(ihtst >= 0);
	if (ihdt >= ihdtMaxSep)
		{ /* footnote separator */
		Assert(ihtst < 3);
		CopyCsSt(rgFtnSepSt[ihtst], st);
		}
	else
		{
		Assert(ihtst < ihtstMax);
		hplcsed = PdodMother(doc)->hplcsed;

		CopyCsSt(rghtst[ihtst], st);

		/* add the section identifier if more than one section */
		pch = &st[st[0] + 1];
		*pch++ = ' ';
		*pch++ = '(';
		*pch++ = chSectionID;
		if (hplcsed != hNil && (*hplcsed)->iMac > 1)
			st[0] += 4 + CchIntToPpch(ised + 1, &pch);
		else
			{
			*pch++ = '1';
			st[0] += 5;
			}
		*pch++ = ')';
		}
	Assert(st[0] < cchMax);
}


/* %%Function:CmdInsertPageField %%Owner:chic */
CMD CmdInsertPageField(pcmb)
CMB * pcmb;
{
	SwitchToHdrWw(wwCur);
	return CmdInsFltSzAtSelCur(fltPage, NULL /*szArgs*/, bcmHdrPage, 
			fTrue /*fCalc*/, fTrue /*fShowDefault*/, fTrue /*fShowResult*/);
}


/* %%Function:CmdInsertDateField %%Owner:chic */
CMD CmdInsertDateField(pcmb)
CMB * pcmb;
{
	SwitchToHdrWw(wwCur);
	return CmdInsFltSzAtSelCur(fltDate, NULL /*szArgs*/, bcmHdrDate, 
			fTrue /*fCalc*/, fTrue /*fShowDefault*/, fTrue /*fShowResult*/);
}


/* %%Function:CmdInsertTimeField %%Owner:chic */
CMD CmdInsertTimeField(pcmb)
CMB * pcmb;
{
	SwitchToHdrWw(wwCur);
	return CmdInsFltSzAtSelCur(fltTime, NULL /*szArgs*/, bcmHdrTime, 
			fTrue /*fCalc*/, fTrue /*fShowDefault*/, fTrue /*fShowResult*/);
}


/* %%Function:FCanLinkPrev %%Owner:chic */
BOOL FCanLinkPrev(ww)
int ww;
{
	int docHdrDisp, docMom;
	CP cpMom;
	struct DOD *pdod;
	struct WWD *pwwd;

	if (ww == wwNil || (pwwd = PwwdWw(ww))->wk != wkHdr || 
			pwwd->hwndIconBar == NULL || pwwd->fPageView)
		return fFalse;

	docHdrDisp = PdrGalley(PwwdWw(ww))->doc;
	pdod = PdodDoc(docHdrDisp);
	docMom = DocMother(docHdrDisp);

	if (pdod->ihdt >= ihdtMaxSep)
		/* editing footnote */
		return pdod->fDirty || IhddFromIhdtFtn(docMom, pdod->ihdt) >= 0;

	/* header/footer */
	else  if ((cpMom = CpMomFromHdr(docHdrDisp)) > cp0 && pdod->fDirty)
		return fTrue;

	else
		{
		int grpfIhdt = 1 << pdod->ihdt;

		CacheSect(docMom, cpMom);
		return !FInFirstSection(docMom, cpMom) &&
				!pdod->fHdrSame && (vsepFetch.grpfIhdt & grpfIhdt);
		}
}






/* %%Function:CmdHdrLinkPrev %%Owner:chic */
CMD CmdHdrLinkPrev(pcmb)
CMB *pcmb;
{
/* Invoked by the "Link to Previous" (AKA "Reset") button in the header icon bar
*/
	int docMom;
	int ihdt;
	int ihdd;
	struct CA caIns, caDel;

/* must be in a header or footnote separator window and the button is not grayed */
	if (!FCanLinkPrev(wwCur))
		{
		Beep();
		return cmdError;
		}
	docMom = DocMother(selCur.doc);
	ihdt = PdodDoc(selCur.doc)->ihdt;

	if (ihdt < ihdtMaxSep)
		{
		if (PdodDoc(selCur.doc)->ised <= 0)
			{ /* in first section, link should be disabled */
			Beep();
			return cmdError;
			}
		if (IdMessageBoxMstRgwMb(mstHdrConfirmDelete, NULL, MB_YESNOQUESTION)
				!= IDYES)
			return cmdCancelled;
		FCaFromIhdt(docMom, CpMomFromHdr(selCur.doc), ihdt, fTrue, &caIns, &ihdd);
		}
	else  /* footnote separator */
	
		{
		ihdd = (ihdt == ihdtTFtn) ? ihddTFtn :
				(ihdt == ihdtTFtnCont) ? ihddTFtnCont : ihddNil;
		CaFromIhddSpec(docMom, ihdd, &caIns);
		}
	Assert(caIns.cpFirst != cpNil);
	Assert(caIns.cpLim != cpNil);
	SetSameAsPrevCtl(wwCur, fFalse /* fEnable*/);

	PcaSet( &caDel, selCur.doc, cp0, CpMacDocEdit(selCur.doc) + 
			/* if there is text, use its'EOP, else leave docHdrDisp's EOP */
	((caIns.cpLim == caIns.cpFirst) ? cp0 : ccpEop));
#ifdef DEBUG
	if (caIns.doc == docNil)
		Assert(DcpCa(&caIns) == cp0);
#endif


#ifdef DONEELSEWHERE
	TestInvalAgain();  /* clear repeat if cmd smashed by changed doc */
#endif


/* we have to set doc dirty now so that undo will actually change the
header; otherwise, SaveHeader wouldn't know anything changed */
	PdodDoc(selCur.doc)->fDirty = fTrue; /* so that the fDirty bit get copy to docUndo */
	if (!FSetUndoB1(bcmHdrLinkPrev, uccPaste, &caDel))
		return cmdCancelled;
	TurnOffSel(&selCur);
	if (!FReplaceCps( &caDel, &caIns ))
	/* so that undo will undo what got moved in */
		caIns.cpLim = CpMacDoc(selCur.doc);
	SelectIns(&selCur, cp0);
	caIns.doc = selCur.doc;
	caIns.cpLim -= caIns.cpFirst;
	caIns.cpFirst = cp0;
	SetUndoAfter(&caIns);
	PdodDoc(selCur.doc)->fDirty = fFalse;
	Debug(vdbs.fCkDoc ? CkHplchdd(docMom) : 0);
	PdodDoc(selCur.doc)->fHdrSame = fTrue;
	return cmdOK;
}




/* %%Function:CmdCloseLowerPane %%Owner:chic */
CmdCloseLowerPane(pcmb)
CMB *pcmb;
{

	if (hmwdCur == hNil || !(*hmwdCur)->fSplit)
		{
		Beep();
		return cmdError;
		}
	else
		{
		BOOL fRecordingSav ;
		InhibitRecorder(&fRecordingSav, fTrue);
		KillSplit(hmwdCur, fTrue/*fLower*/, fFalse/*fAjustWidth*/);
		InhibitRecorder(&fRecordingSav, fFalse);
		return cmdOK;
		}
}






/* %%Function:SetSameAsPrevCtl %%Owner:chic */
SetSameAsPrevCtl(wwHdr, fEnable)
int wwHdr;
int fEnable;
{
	int docHdrDisp = PdrGalley(PwwdWw(wwHdr))->doc;
	int ww, wwT;
	HWND hwndIB;

	Assert(docHdrDisp != docNil);
	Assert(PdodDoc(docHdrDisp)->fDispHdr);
	Assert(docHdrDisp != vdocTemp);
	for (ww = wwNil; (wwT = WwDisp(docHdrDisp, ww, fFalse)) != wwNil; ww = wwT)
		{
		hwndIB = PwwdWw(wwT)->hwndIconBar;
		if (PwwdWw(wwT)->fPageView || hwndIB == NULL)
			continue;
		Assert(PwwdWw(wwT)->wk == wkHdr);
		DisableIibbHwndIb(hwndIB, iibbHdrLinkPrev, !fEnable);
		GrayIibbHwndIb(hwndIB, iibbHdrLinkPrev, !fEnable);
		}
}




/* %%Function:SetHdrIBarName %%Owner:chic */
SetHdrIBarName(docHdrDisp, ihtst, ww)
int docHdrDisp;
int ihtst;
int ww;
{
	char stTitle[cchMaxSz];

	Assert(docHdrDisp && ww);
	GetHeaderSt(docHdrDisp, ihtst, stTitle, cchMaxSz);
	StToSzInPlace(stTitle);
	Assert(PwwdWw(ww)->hwndIconBar);
	SetIibbTextHwndIb(PwwdWw(ww)->hwndIconBar, iibbHdrIBarDesp, stTitle);
}


/* %%Function:CmdPageNumbers %%Owner:chic */
CMD CmdPageNumbers(pcmb)
CMB * pcmb;
{
	CABINSPGNUM *pcabinspgnum;
	CMD cmd;
	struct CA caMom;
	int jc;
	BOOL fHdr;
	int iPos;

	if (PdodDoc(selCur.doc)->fDispHdr)
		/* make it dirty so it get replaced with the page number header */
		PdodDoc(selCur.doc)->fDirty = fTrue;


	if (!FCleanUpHdr(selCur.doc, fFalse, fFalse))
		return cmdError;

	if (pcmb->fDefaults)
		{
		pcabinspgnum = (CABINSPGNUM *) *pcmb->hcab;
		if (PdodDoc(selCur.doc)->fDispHdr)
			{
			pcabinspgnum->iType = (FHeaderIhdt(PdodDoc(selCur.doc)->ihdt) ? iTypeTop : iTypeBottom);
			}
		else
			pcabinspgnum->iType = iTypeBottom;
		pcabinspgnum->iPos = iPosRight;
		}

	if (pcmb->fDialog)
		{
		char dlt [sizeof (dltInsPgNum)];

		BltDlt(dltInsPgNum, dlt);

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
			return cmdCancelled;

		case tmcOK:
			break;
			}
		}

	if (!pcmb->fAction)
		return cmdOK;

	StartLongOp();
	GetCaHdrMom(&caMom, wwCur);
	FreezeHp();
	pcabinspgnum = (CABINSPGNUM *) *pcmb->hcab;
	fHdr = pcabinspgnum->iType == iTypeTop;
	iPos = pcabinspgnum->iPos;
	MeltHp();
	jc = iPos == iPosLeft ? jcLeft : (iPos == iPosCenter ? jcCenter : jcRight);

/* generate a page field paragraph in vdocScratch */
	if (DocCreateScratch(docNil) == docNil)
		{
		cmd = cmdNoMemory;
		goto LRet;
		}

	cmd = CmdDoPgNum(fHdr, jc, &caMom, fFalse /* fFromRTF */);

	ReleaseDocScratch();
	if (cmd == cmdOK)
		{
		ApplyPgNumSetting(&caMom);
		SetUndoNil(); /* cannot undo header stuff */
		InvalPageView(selCur.doc);/* update any pageview window */
		SelectIns(&selCur, selCur.cpFirst);
		}

LRet:
	EndLongOp(fFalse);
	return cmd;
}




/* %%Function:CmdDoPgNum %%Owner:chic */
CMD CmdDoPgNum(fHdr, jc, pca, fFromRTF)
BOOL fHdr;
int jc;
struct CA *pca;
BOOL fFromRTF;
{
	struct PAP pap;
	struct CHP chp;
	BOOL fConfirmSave = fFalse;
	int mpihdtihdd[ihdtMaxSep];
	BOOL fHaveHdr = fFalse;
	BOOL fHaveFtr = fFalse;
	int stc, cstcStd;
	int docHdr, docHdrDisp = docNil;
	int ihdt, ihdtHdr, ihdd;
	int grpfIhdt, grpfIhdtT;
	struct CA caHdr, caPgNum, caSectT;
	CHAR szKeyword [cchMaxFieldKeyword + 1];

	Assert(vdocScratch);
	ihdtHdr = fHdr ? ihdtTRight : ihdtBRight;
	if (!FEnsureStcDefined(vdocScratch, stc = (fHdr) ? stcHeader : stcFooter, &cstcStd))
		goto LNoMem;
	MapStc(PdodDoc(vdocScratch), stc, &chp, &pap);
	pap.jc = jc;
	if (!FInsertRgch(vdocScratch, cp0, rgchEop, (int)ccpEol, &chp, &pap) ||
			!FInsertFieldDocCp(vdocScratch, cp0, fFalse/*fSeparator*/))
		goto LNoMem;
	GetFltKeyword (fltPage, szKeyword);
	if (!FInsertRgch(vdocScratch, (CP)1, szKeyword, CchSz(szKeyword)-1, NULL, NULL))
		goto LNoMem;
	PcaSet(&caPgNum, vdocScratch, cp0, CpMacDocEdit(vdocScratch));

/* handle pageview */
	if (!fFromRTF && (*hwwdCur)->fPageView)
		{
		CMD cmd;

		vrf.fInsPgNum = fTrue;
		cmd = CmdPageViewHeader(ihdtHdr, &caPgNum);
		vrf.fInsPgNum = fFalse;
		return (cmd);
		}

/* check what we had */
	SetWords(mpihdtihdd, ihddNil, ihdtMaxSep);
	CacheSectSedMac(pca->doc, pca->cpFirst);
	caSectT = caSect;
	caSect.doc = docNil;
	grpfIhdt = vsepFetch.grpfIhdt;
	if (IhddFromDocCp(&caSectT, &ihdd, mpihdtihdd) != ihddNil)
		{
		for (ihdt = 0; ihdt < ihdtMaxSep; ihdt++)
			{
			switch (ihdt)
				{
			case ihdtTFirst:
			case ihdtBFirst:
				break;
			default:
				if (mpihdtihdd[ihdt] != ihddNil)
					{
/* we will ask confirm message if there are any even hdr/ftr (because we 
are turning off Odd/Even pages) or if we are inserting hdr and there are existing
odd header or we are inserting ftr and there are existing odd footer */
					switch (ihdt)
						{
					case ihdtTLeft:
					case ihdtTRight:
						fHaveHdr = fTrue;
						if (!fConfirmSave && (ihdt == ihdtTLeft || 
								(fHdr && ihdt == ihdtTRight)))
							fConfirmSave = fTrue;
						break;
					case ihdtBLeft:
					case ihdtBRight:
						fHaveFtr = fTrue;
						if (!fConfirmSave && (ihdt == ihdtBLeft || 
								(!fHdr && ihdt == ihdtBRight)))
							fConfirmSave = fTrue;
						break;
						}
					}
				break;
				}
			}
		}

	/* if rtf, we know any confirm is from a linked h/f and we will
		replace it
	*/
	if (!fFromRTF)
		if (fConfirmSave && (IdMessageBoxMstRgwMb(mstReplaceHdr, NULL,
				MB_YESNOQUESTION) != IDYES))
			{
			return (cmdCancelled);
			}


/* replace existing header/footer */
/* from RTF, always insert, so links are broken */
	if (!fFromRTF && ((fHdr && fHaveHdr) || (!fHdr && fHaveFtr)))
		{
		for (ihdt = 0; ihdt < ihdtMaxSep; ihdt++)
			{ /* only replace odd and even, leave first alone */
			if (mpihdtihdd[ihdt] != ihddNil &&
					((fHdr && (ihdt == ihdtTLeft || ihdt == ihdtTRight)) || 
					(!fHdr && (ihdt == ihdtBLeft || ihdt == ihdtBRight))))
				{
				if ((docHdrDisp = DocDispFromDocCpIhdt(pca->doc, pca->cpFirst, ihdt)) != docNil)
					{
/* if there is a display doc for this header, replace hdr text in the docHdrDisp
	so that changes are reflected in header pane.  FSaveHeader to save it back to
	docHdr
*/
					if (!FReplaceCps(PcaSet(&caHdr, docHdrDisp, cp0,
							CpMacDoc(docHdrDisp)), &caPgNum))
						goto LNoMem;
					else
						{
						PdodDoc(docHdrDisp)->fDirty = fTrue;
						if (!FSaveHeader(docHdrDisp, fFalse))
							{
							goto LNoMem;
							}
						}
					}
				else
					{
/* no display doc for this hdr, replace in docHdr directly */
					FCaFromIhdt(pca->doc, pca->cpFirst, ihdt, fFalse, &caHdr, &ihdd);
					if (!FReplaceCps(&caHdr, &caPgNum))
						goto LNoMem;
					}
				}
			}
		}
	else  /* insert new header/footer */
		
		{
/* create docHdr if not already exist */
		if ((docHdr = PdodDoc(pca->doc)->docHdr) == docNil &&
				(docHdr = DocCreateSub(pca->doc, dkHdr)) == docNil)
			goto LNoMem;
		if (PdodDoc(docHdr)->hplchdd == hNil &&
				HplcCreateEdc(mpdochdod[docHdr], edcHdd) == hNil)
			{
			DisposeDoc(docHdr);
			PdodDoc(pca->doc)->docHdr = docNil;
			goto LNoMem;
			}

/* ihdd is the first ihdd in current section, count through all the grpfIhdt bits
to find the ihdd to insert to */
		ihdt = 0;
		for (grpfIhdtT = grpfIhdt; grpfIhdt != 0 && ihdt < ihdtHdr; grpfIhdtT >>= 1, ihdt++)
			if (grpfIhdtT & 1)
				ihdd++;
		grpfIhdt |= 1 << ihdtHdr;
		Assert(ihdd != ihddNil);
		if (!FInsertIhdd(docHdr, ihdd))
			{
LNoMem:
			return (cmdNoMemory);
			}
		SetGrpfIhdtPca(grpfIhdt, &caSectT);
		ReplaceIhdd(docHdr, ihdd, &caPgNum);
		}

	return (cmdOK);
}









/* %%Function:FDlgInsPgNum %%Owner:chic */
FDlgInsPgNum(dlm, tmc, wNew, wOld, wParam)
DLM dlm;
TMC tmc;
WORD wOld, wNew, wParam;
{
	switch (dlm)
		{
	case dlmInit:
		if (PdodDoc(selCur.doc)->fDispHdr)
			EnableTmc(tmcIPNType, fFalse);
		break;
	default:
		break;
		} /* end of switch */

	return fTrue;
}


/*******************************/
/* D i s p o s e   W w   H d r */
/* %%Function:DisposeWwHdr %%Owner:chic */
EXPORT DisposeWwHdr(ww)
int ww;
{
	KillSplit(mpmwhmwd[PwwdWw(ww)->mw], fTrue /*fLower*/, fFalse /*fAdjustWidth*/);
}


IhtstFromDoc(docHdrDisp)
int docHdrDisp;
{
	int docMom = DocMother(docHdrDisp);
	int ihdt = PdodDoc(docHdrDisp)->ihdt;
	BOOL fFacingPage = PdodDoc(docMom)->dop.fFacingPages;
	int ihtst;

	Assert(PdodDoc(docHdrDisp)->fDispHdr);
	Assert(ihdt >= 0 && ihdt < ihdtMax);

	if (ihdt >= ihdtMaxSep) /* footnote separator */
		{
		ihtst = ihdt - ihdtMaxSep;
		}
	else  /* header */
		
		{
		switch (ihdt)
			{
		case ihdtTFirst:
			ihtst = ihtstFirstHdr;
			break;
		case ihdtBFirst:
			ihtst = ihtstFirstFtr;
			break;
		case ihdtTLeft:
			ihtst = ihtstEvenHdr;
			break;
		case ihdtBLeft:
			ihtst = ihtstEvenFtr;
			break;
		case ihdtTRight:
			ihtst = fFacingPage ? ihtstOddHdr : ihtstHdr;
			break;
		case ihdtBRight:
			ihtst = fFacingPage ? ihtstOddFtr : ihtstFtr;
			break;
#ifdef DEBUG
		default:
			Assert(fFalse);
			break;
#endif /* DEBUG */
			}
		}
	return ihtst;
}


/* switch focus to the header window if the given window is 
	the upper half pane.
*/
SwitchToHdrWw(ww)
int ww;
{
	int wwLower;
	struct MWD *pmwd = PmwdWw(ww);

	if (pmwd->fSplit && (wwLower = pmwd->wwLower) != ww)
		{
		if (PwwdWw(wwLower)->fHdr)
			NewCurWw(wwLower, fFalse);
		}
}


/* %%Function:ApplyPgNumSetting %%Owner:chic */
ApplyPgNumSetting(pca)
struct CA *pca;
{
	CHAR rgb[2];
	struct CA ca, caInval;

/* turn off different odd/even, turn on different first page */

#ifdef DEBUG
	struct ESPRM esprm;
	esprm = dnsprm[sprmSFTitlePage];
	Assert(esprm.cch == 2);
	Assert(!PdodDoc(pca->doc)->fShort);
#endif

	PdodDoc(pca->doc)->dop.fFacingPages = fFalse;
	rgb[0] = sprmSFTitlePage;
	rgb[1] = fTrue;
	bltbyte(pca, &ca, sizeof(struct CA));
	ExpandCaSprm(&ca, &caInval, rgb);
	Assert(ca.cpFirst != ca.cpLim);
	ApplyGrpprlCa(rgb, 2, &ca);
	InvalCp(&caInval);

}



/*******************************************/
/* C m d   P a g e   V i e w   H e a d e r */
/* %%Function:CmdPageViewHeader %%Owner:chic */
CmdPageViewHeader(ihdt, pcaSrc)
int ihdt;
struct CA *pcaSrc; /* 0 for MAC */
	/* pcaSrc used by WIN only to indicate insert page number command:
	   use whatever ihdt passed in and insert the content of pcaSrc as hdr/ftr
	   text
	*/
{
/* find the header or footer in the current window and move selection there.
	if none, create the appropriate header for this page */
	int doc = DocBaseWw(wwCur);
	int docHdr = PdodDoc(doc)->docHdr, docHdrDisp;
	int fHdr = FHeaderIhdt(ihdt), ihdd;
	int ipgd = (*hwwdCur)->ipgd, ipgdT, idr, idrMac;
	struct PLC **hplcpgd = PdodDoc(doc)->hplcpgd;
	struct DR *pdr;
	CP cpFirst = CpMin(CpMacDocEdit(doc), CpPlc(hplcpgd, ipgd)); /* if page consists of all footnotes, CpPlc will be cpMacDoc */
	struct CA caHdr, caT;
	struct PGD pgd;
	struct DRF drfFetch;
	BOOL fInsPageNum = (pcaSrc != 0); /* pcaSrc later may get a value */

#ifdef WIN
	if (fInsPageNum)
		{
		Assert(ihdt == ihdtTRight || ihdt == ihdtBRight);
		vihdtNew = ihdt;
		}
	else
#endif
			{
/* first figure out what ihdt we want */

			GetPlc(hplcpgd, ipgd, &pgd);
			CacheSect(doc, cpFirst);
			if (pgd.fAllFtn && cpFirst == caSect.cpFirst)
				{
				Assert(cpFirst > 0);
				CacheSect(doc, --cpFirst); /* backup from cpMacDoc */
				}
			vihdtNew = IhdtFromDocIpgd(doc, ipgd, cpFirst, fHdr);
			}

/* try to find a dr containing the header or footer */
	if (docHdr != docNil)
		for (idr = 0, idrMac = (*hwwdCur)->idrMac; idr < idrMac; idr++)
			{
			pdr = PdrFetch(hwwdCur, idr, &drfFetch);
			if (pdr->doc > 0 && pdr->cpFirst != cpNil && PdodDoc(pdr->doc)->dk == dkDispHdr)
				{
				docHdrDisp = pdr->doc;
				if (vihdtNew == PdodDoc(docHdrDisp)->ihdt)
					{
					FreePdrf(&drfFetch);
#ifdef WIN
/* If coming from Insert Page Number, do not replace the first page 
hdr/ftr - dumb spec said so */
					if (fInsPageNum)
						{
						struct CA ca;
						if (IdMessageBoxMstRgwMb(mstReplaceHdr, NULL, MB_YESNOQUESTION) != IDYES)
							{
							vihdtNew = ihdtNil;
							return (cmdCancelled);
							}
						FReplaceCps(PcaSet(&ca, docHdrDisp, cp0, CpMacDoc(docHdrDisp)), pcaSrc);
						PdodDoc(docHdrDisp)->fDirty = fTrue;
						FSaveHeader(docHdrDisp, fFalse);
						}
#endif
					goto LSelectHdr;
					}
				}
			FreePdrf(&drfFetch);
			}

/* doesn't exist, create a new header. it will inherit text from prev hdr */
	if (!fInsPageNum && PdodDoc(doc)->docHdr != docNil)
		{
#ifdef WIN /* the return flag is only interested to MAC for fSameAsPrev,
		has nothing to do with NoMem */
				FCaFromIhdt(doc, cpFirst, vihdtNew, fFalse, &caHdr, &ihdd);
#else

		if (!FCaFromIhdt(doc, cpFirst, vihdtNew, fFalse, &caHdr, &ihdd))
			goto LNoMem;
#endif
		if (caHdr.cpFirst != caHdr.cpLim)
			{
			SetUndoNil();
			if (!FReplaceCps(PcaSetWholeDoc(&caT, docUndo), &caHdr))
				goto LNoMem;
			pcaSrc = PcaSetWholeDoc(&caT, docUndo);
			}
		}
	CacheSect(doc, cpFirst); /* for FEnsureIhdtExists */
	if (!FEnsureIhdtExists(doc, &docHdr, vihdtNew, pcaSrc))
		{
LNoMem:
		vihdtNew = ihdtNil;
		return(cmdNoMemory);
		}

/* force recalc of the page, select the header */
	InvalPageView(doc);
	if ((ipgd = IpgdPldrFromIpgd(wwCur, ipgd)) == ipgdNil)
		goto LNoHdr;
	if (vmerr.fNoHdrDr)
		{
		ErrorEid(eidComplexSession, "CmdPageViewHeader");
		goto LNoHdr;
		}

	cpFirst = CpPlc(hplcpgd, ipgd);
	GetPlc(hplcpgd, ipgd, &pgd);
	CacheSect(doc, cpFirst);
	if (pgd.fAllFtn && cpFirst == caSect.cpFirst)
		{
		Assert(cpFirst > 0);
		--cpFirst;
		}

/* check if docHdrDisp and idr exist because we are about to make a
selection in the newly inserted header or footer. 
If insert page number, the header inserted may not end up on the current
page, so don't bother.
*/
	if (fInsPageNum && vihdtNew != IhdtFromDocIpgd(doc, ipgd, cpFirst, fHdr))
		{
		vihdtNew = ihdtNil;
		return cmdOK;
		}

	if ((docHdrDisp = DocDisplayHdr(doc, cpFirst, vihdtNew)) == docNil ||
			IdrFromHpldrDocCp(hwwdCur, wwCur, docHdrDisp, cp0, fFalse, fFalse) == idrNil)
		{
LNoHdr:
		vihdtNew = ihdtNil;
		return(cmdError);
		}
	PdodDoc(docHdrDisp)->fDirty = fTrue;	/* force save after undo */
LSelectHdr:
	vihdtNew = ihdtNil;
	TurnOffSel(&selCur);
	selCur.doc = docHdrDisp;
	SelectIns(&selCur, cp0);
	vfSeeSel = fTrue;
	return(cmdOK);
}


/****************************************/
/* F  E n s u r e  I h d t  E x i s t s */
/* should have called CacheSect(docMom, cp) before calling this */
/* %%Function:FEnsureIhdtExists %%Owner:chic */
FEnsureIhdtExists(docMom, pdocHdr, ihdt, pcaSrc)
int docMom;
int *pdocHdr;
int ihdt;
struct CA *pcaSrc;
{
	int docHdr = *pdocHdr;
	int stc, cstcStd, mpihdtihdd[ihdtMaxSep];
	int ihdd, ihdtT;
	int ihddT;
	struct CA ca, caInval;
	struct CHP chp;
	struct PAP pap;
	char rgb[2];

/* create the header subdoc if it doesn't exist */
	if (docHdr == docNil)
		{
		if ((docHdr = DocCreateSub(docMom, dkHdr)) == docNil ||
				HplcCreateEdc(mpdochdod[docHdr], edcHdd) == hNil)
			return fFalse;
		PdodDoc(docMom)->docHdr = docHdr;
		*pdocHdr = docHdr;
		}

/* add a single eop for the header */
/* if pcaSrc != NULL, use pcaSrc as header content */
	if ((vsepFetch.grpfIhdt & (1 << ihdt)) == 0)
		{
		rgb[0] = sprmSGrpfIhdt;
		rgb[1] = vsepFetch.grpfIhdt | (1 << ihdt);
		ca = caSect;
		ExpandCaSprm(&ca, &caInval, rgb);
		stc = FHeaderIhdt(ihdt)? stcHeader : stcFooter;
		if (!FEnsureStcDefined(docMom, stc, &cstcStd))
			return fFalse;
		MapStc(PdodDoc(docMom), stc, &chp, &pap);
		SetWords(mpihdtihdd, ihddNil, ihdtMaxSep);
		IhddFromDocCp(&ca, &ihdd, mpihdtihdd);
		for (ihdtT = 0; ihdtT < ihdt; )
			if ((ihddT = mpihdtihdd[ihdtT++]) != ihddNil &&
					ihddT >= ihdd)
				ihdd++;
		if (!FInsertIhdd(docHdr, ihdd))
			return(fFalse);
		ApplyGrpprlCa(rgb, 2, &ca);
		if (vmerr.fMemFail)
			goto LDeleteIhdd;
		CaFromIhdd(docHdr, ihdd, &ca);
		if (pcaSrc)
			ReplaceIhdd(docHdr, ihdd, pcaSrc);
		else
			FInsertRgch(docHdr, ca.cpFirst, rgchEop, cchEop, &chp, &pap);
		}
/* the header already exist, replace content with pcaSrc */
	else  if (pcaSrc)
		{
#ifdef WIN
		if (!vrf.fInsPgNum || IdMessageBoxMstRgwMb(mstReplaceHdr, NULL, MB_YESNOQUESTION) == IDYES)
#endif
			{
			FCaFromIhdt(caSect.doc, caSect.cpFirst, ihdt, fFalse, &ca, &ihdd);
			return FReplaceCps(&ca, pcaSrc);
			}
		return fFalse;
		}

	if (!vmerr.fMemFail && !vmerr.fDiskFail)
		return(fTrue);
LDeleteIhdd:
	ReplaceIhdd(docHdr, ihdd, PcaSetNil(&ca));
	return(fFalse);
}


/*************************************/
/* I h d t  F r o m  D o c  I p g d  */
/* %%Function: IhdtFromDocIpgd  %%Owner:chic */
/* Return the header/footer type based on the doc and ipgd.
fHdr indicates if we want a header type or a footer type.
*/
IhdtFromDocIpgd(doc, ipgd, cpFirst, fHdr)
int doc;
int ipgd;
CP cpFirst; /* can't simply use CpPlc from ipgd because of pages with only footnotes */
BOOL fHdr;
{
	struct PGD pgd;
	struct PLC **hplcpgd;

	Assert(!PdodDoc(doc)->fShort);
	hplcpgd = PdodDoc(doc)->hplcpgd;
	AssertH(hplcpgd);
	GetPlc(hplcpgd, ipgd, &pgd);
	Assert(FInCa(doc, cpFirst, &caSect));
/* If the cpFirst of the page is the same as the beginning of a section,
we return a firstpage hdr/ftr type.
If we have facing pages on and the current page number is even, we
return a even hdr/ftr type.
If we do not have facing pages on or the current page number is odd,
we return odd hdr/ftr type.
*/
	if (cpFirst == caSect.cpFirst && pgd.cl == 0 && vsepFetch.fTitlePage)
		return (fHdr ? ihdtTFirst : ihdtBFirst);
	else  if (PdodDoc(doc)->dop.fFacingPages && (pgd.pgn & 1) == 0)
		return (fHdr ? ihdtTLeft : ihdtBLeft);
	else
		return (fHdr ? ihdtTRight : ihdtBRight);
}
