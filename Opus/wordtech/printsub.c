/* P R I N T S U B . C */

#ifdef WIN
#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOWINMESSAGES
#define NONCMESSAGES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#define NOICON
#define NOKEYSTATE
#define NOSYSCOMMANDS
#define NOSHOWWINDOW

#define OEMRESOURCE
#define NOSYSMETRICS
#define NOBITMAP
#define NOCLIPBOARD
#define NOCREATESTRUCT
#define NOCTLMGR
#define NODRAWTEXT
#define NOFONT
#define NOMB
#define NOMEMMGR
#define NOMENUS
#define NOMETAFILE
#define NOMINMAX
#define NOOPENFILE
#define NORECT
#define NOREGION
#define NOSCROLL
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOWINOFFSETS
#define NOWNDCLASS
#define NOCOMM
#define NOKANJI
#endif

#define DIALOGS
#define WINDOWS
#define EVENTS
#define CONTROLS

#ifdef MAC
#include "toolbox.h"
#endif

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "doc.h"
#include "disp.h"
#include "props.h"
#include "border.h"
#include "sel.h"
#include "ch.h"
#include "inter.h"
#include "file.h"
#include "format.h"
#include "layout.h"
#include "print.h"
#include "debug.h"
#include "prm.h"
#include "fkp.h"

#ifdef MAC
#include "mac.h"
#include "dlg.h"
#include "printMac.h"
#include "font.h"
#include "resource.h"
#endif

#ifdef WIN
#include "field.h"
#include "prompt.h"
#include "message.h"
#include "screen.h"
#endif

/* G L O B A L S */
struct PRI      vpri;

int             vlm = lmNil;
int             vfPrvwDisp = fFalse;    /* so LoadFont does preview font */
int             vsectFirst, vsectLast;  /* section limits for printing */
int             vpgnFirst, vpgnLast;    /* page limits (within vsect*) */
FC              vfcSpec = (FC) -1;      /* FC on scratch file of rgch spec */
CP              vcpFirstLayout = cpNil; /* cpFirst of current layout page */
CP              vcpLimLayout = cpNil;   /* cpLim of current layout page */
ENV             venvLayout;             /* SetJmp/DoJmp env */

struct FLS      vfls;                   /* layout: format line state */
struct PLC      **vhplcfrl;             /* footnote references */
struct PLLR     **vhpllr;               /* for sorting LRs in print */
struct PLLR     **vhpllrAbs;            /* abs position objects */
struct CA       caHdt;                  /* describes rghdt contents */
int             vfRestartLayout;        /* abort layout because of apo */
struct HDT      vrghdt[ihdtMax];        /* header descriptor table */
struct PLLBS    **vhpllbs;              /* stacked LBS structures */
struct CA       caParaL;
struct PLLR	**vhpllrSpare;
BOOL		vfInFormatPage;
struct LBS	*vplbsFrame;
struct RC       vrcBand;                /* banding rect. Only valid during print */
CP              vcpAbsMic;


/* E X T E R N A L S */

extern BOOL             vfInsertMode;
extern int              vdocTemp;
extern int              vlm;
extern int              wwCur;
extern FC               vfcSpec;
extern struct WWD       **hwwdCur;
extern struct PLC    **vhplcfrl;
extern struct PREF      vpref;
extern CP		cpInsert;
extern CP		vcpFirstLayout;
extern struct CA        caPara;
extern struct CA	caPage;
extern struct CA        caSect;
extern struct CA        caTap;
extern struct TAP       vtapFetch;
extern struct TCC       vtcc;
extern struct TCXS      vtcxs;
extern struct HDT       vrghdt[];
extern struct DOD       **mpdochdod[];
extern struct FLS       vfls;
extern struct FLI       vfli;
extern struct CHP       vchpStc;
extern struct CHP       vchpFetch;
extern struct PAP       vpapFetch;
extern struct PAP       vpapStc;
extern struct SEP       vsepFetch;
extern int		vdocFetch;
extern struct SEL       selCur;
extern struct MERR      vmerr;
extern struct FMTSS     vfmtss;
extern struct PRSU      vprsu;
extern struct PLLR      **vhpllr;
extern struct FKPD      vfkpdChp;
#ifdef DEBUG
extern struct DBS       vdbs;
#endif

#ifdef MAC
extern int		vfVisiSave;
extern int		vfAsPrintSave;
extern PRR far *far	*vqqprrDefault;
extern int              vecPrint;
extern int              vwwHdr;
extern int              ftcLast;
extern struct ITR       vitr;
extern struct LLC	vllc;
extern struct FWCA	vrgfwca[];
extern BOOL             vfInsertMode;

int FLrGt();
DisplayTcLines();
#endif

#ifdef WIN
extern HWND             vhwndStatLine;
extern HWND             vhwndPgPrvw;
extern CP		vcpLimLayout;
extern int		vflm;
extern struct CA	caAdjust;
extern struct PMD	*vppmd;
extern struct PRI	vpri;
extern struct FTI       vfti;
extern struct SCI       vsci;
extern struct MERR	vmerr;
#endif

#ifdef MAC
FC FcAppendRgchToFn ();
#endif /* MAC */


/*********************************/
/* F  U p d a t e  H p l c p g d */
#ifndef WIN
NATIVE /* WINIGNORE - !WIN */
#endif /* !WIN */
/* %%Function:FUpdateHplcpgd %%Owner:chrism */
FUpdateHplcpgd(ww, doc, plbsText, plbsFtn, prpl, pat)
int ww;
int doc;	/* can be docFtn; rpl.cp should then be a ftn cp */
struct LBS *plbsText, *plbsFtn;
struct RPL *prpl;
int pat;
{
/* creates if necessary then scans plcpgd. Inserts all missing entries
	and updates all fUnk entries. prpl specifies the limits of repagination,
	which may be exceeded. ipgd of requested page is returned in prpl */
	int ipgd = 0, ipgdMac, cpgdFormat = 0, cpgdNoFormat = 0;
	int lbc = lbcNil, fUnk;
	int fFormat, fFtn = fFalse;
	struct DOD *pdod;
	struct PLC **hplcpgd, **hplcpgdFtn;
	struct PGD pgd;
	CP cpMac, cpCurText;
	int fNoAbort = fTrue;
#ifdef WIN
	struct PPR **hppr;
	int fPromptUp = FALSE;
#endif

/* set up the lbs structures for traversing the document */
	Scribble(ispUpdateHplcpgd, 'U');
#ifdef MAC
	SetCrs(prpl->crs);
	/* ensure that the grafport is the requesting doc window */
	if (vlm != lmPreview)
		PwwdSetWw(ww, cptDoc);
#endif
	pdod = PdodDoc(doc);
	if (pdod->fFtn)
		{
		doc = DocMother(doc);
		fFtn = fTrue;
		}

	if (!FInitLayout(WinMac(wwLayout, ww), doc, plbsText, plbsFtn))
		return fFalse;

	pdod = PdodDoc(doc);
	hplcpgd = pdod->hplcpgd;
	Assert(hplcpgd);
	Win( EnablePreload(pdod->fn) );
	if (plbsFtn->doc == docNil)
		hplcpgdFtn = hNil;
	else
		{
		pdod = PdodDoc(plbsFtn->doc);
		hplcpgdFtn = pdod->hplcpgd;
		}
	PutCpPlc(hplcpgd, IMacPlc(hplcpgd), CpMac2Doc(doc));
	cpMac = CpMacDocEdit(doc);
	SetPgdInvalid(0, plbsText, plbsFtn);

/* check for end of requested pagination */
	for (;;)
		{
		/* we have an entry at ipgd with correct cp - ensured by
			SetPgdInvalid */
		if (vmerr.fMemFail)
			{
			fNoAbort = fFalse;
			break;
			}
		cpCurText = CpFromCpCl(plbsText, fTrue);
		if (lbc == lbcEndOfDoc || ipgd > prpl->ipgd ||
				!fFtn && cpCurText > prpl->cp ||
				fFtn && plbsFtn->cp > prpl->cp ||
				cpCurText > cpMac && !plbsFtn->fContinue)
			{
			goto LTooFar;
			}
		if (ipgd == prpl->ipgd || 
				FPrintDone(plbsText, prpl->ised, prpl->pgn - ((prpl->pgn == 0) ? 0 : 1)))
			{
			break;
			}
		if (vlm == lmBRepag)
			{
			/* background repag: break if 3 pages formatted to
				allow page breaks to be shown; every 3 consecutive
				clean pages, check for user interrupt */
			if (cpgdFormat >= 3 || plbsText->fBkRepagStop)
				break;
			if (cpgdNoFormat >= 3)
				{
#ifdef MAC
				vlm = lmNil;
				InvalFli();
				ftcLast = -1;   /* reset font */
				SetFormatVisi(vfVisiSave);
				IdleTrack(fTrue);
				SetFormatVisi(fFalse);
				InvalFli();
				ftcLast = -1;
				fNoAbort = !FMsgPresent(mtyBRepag);
				vlm = lmBRepag;
#else /* WIN */
				fNoAbort = !FAbortLayout(plbsText->fOutline, plbsText);
#endif
				if (!fNoAbort)
					break;
				cpgdNoFormat = 0;
				}
			}

#ifdef MAC
		if (pat && vlm != lmPreview)
			SetPrintLlc(plbsText->doc, ipgd);
#else
		if ((pat & patmReport) && vlm != lmPreview)
			{
			if (!fPromptUp)
				{
				int rgw[2];
				MST mst;	/* Workaround CS Native bug (bl 9/20/88) */

				/* if printing from docUndo, get true mother doc...
				note: DocMother doesn't work on docUndo */
				rgw[0] = doc == docUndo ? PdodDoc(docUndo)->doc : doc;
				Assert(rgw[0] != docNil);
				rgw[1] = ipgd + 1;

/* if..else to Workaround CS Native bug (bl 9/20/88) */
				if (pat & patmAbort)
					hppr = HpprStartProgressReport (mstRepaginating,
							rgw, 1, fTrue);
				else
					hppr = HpprStartProgressReport (mstRepagNoAbort,
							rgw, 1, fFalse);
				fPromptUp = fTrue;
				}
			else
				ChangeProgressReport(hppr, ipgd+1);
			}
#endif

		fFormat = fFalse;
		if (ipgd >= (ipgdMac = IMacPlc(hplcpgd)))
			{
LTooFar:		
			ipgd = max(0, ipgd - 1);
			break;
			}

/* format the page if it's dirty, or just advance from page table if clean */
		GetPlc(hplcpgd, ipgd, &pgd);
		fUnk = pgd.fUnk;
		if (hplcpgdFtn != hNil && ipgd < IMacPlc(hplcpgdFtn))
			{
			GetPlc(hplcpgdFtn, ipgd, &pgd);
			fUnk |= pgd.fUnk;
			}
		if (!fUnk)
			{
			/* end of page known - advance lbs */
			if (ipgd < ipgdMac - 1)
				AdvanceLbsFromPgd(plbsText, plbsFtn, ipgd + 1);
			else
				{
				plbsText->cp = CpMacDoc(doc);
				plbsText->cl = 0;
				if (plbsFtn->doc != docNil)
					{
					plbsFtn->cp = CpMacDocEdit(plbsFtn->doc);
					plbsFtn->cl = 0;
					}
				plbsFtn->fContinue = fFalse;
				plbsText->lbc = lbcEndOfDoc;
				}
			++cpgdNoFormat;
			}
		else
			{
#ifdef MAC
			if (pat && vlm == lmPreview)
				ShowPgnP(ipgd, fTrue, fFalse);
#endif

			/* in this entry the end of the page is unknown, or
			there's nothing to advance from */
			lbc = LbcFormatPage(plbsText, plbsFtn);

			if (lbc == lbcAbort)
				{
				fNoAbort = fFalse;
				GetPlc( hplcpgd, ipgd, &pgd );
				pgd.fUnk = fTrue;
				PutPlcLast( hplcpgd, ipgd, &pgd );
				break;
				}
			/* FUTURE (chrism): when a pgd has fEmptyPage and fPgnRestart,
			   layout will always be called for it because the page numbers
			   don't match at FNeLbsPgd time. I don't know why this is, 
			   but we should fix it some day */
			/* meantime, don't count empty pages at all */
			if (!plbsText->fEmptyPage)
				{
				if (++cpgdFormat == 2 && (pat & patmDelayReport))
					pat |= patmReport;
				/* we care only about consecutive clean pages */
				cpgdNoFormat = 0;
				}
			Win( selCur.fUpdateStatLine = fTrue );
			fFormat = fTrue;
			}
		/* this also invalidates the next entry if necessary */
		SetPgdValid(ipgd++, plbsText, plbsFtn, fFormat);
		}

/* repagination complete or aborted */
	prpl->ipgd = ipgd;
	if (cpgdFormat > 0 || !fNoAbort)
		vtcc.caTap.doc = vtcc.doc = docNil; /* because we may have mucked w/ fLastRow */
#ifdef WIN
	if ((pat & patmReport) && fPromptUp && vlm != lmPreview 
			&& !(pat & patmChain))
		{
		Assert(!vfInFormatPage);
		vfInFormatPage = vlm == lmPagevw; /* KLUDGE: disable UpdateWw for page vw */
		StopProgressReport(hppr, pdcRestoreImmed);
		vfInFormatPage = fFalse;
		}
	DisablePreload();
#else
	/* be selective about clearing LLC */
	if (vllc.fGrayText)
		InvalLlc();
#endif
	Scribble(ispUpdateHplcpgd, ' ');
	return(fNoAbort);
}


/***************************/
/* F  I n i t  L a y o u t */
/* %%Function:FInitLayout %%Owner:chrism */
FInitLayout(ww, doc, plbsText, plbsFtn)
int ww, doc;
struct LBS *plbsText, *plbsFtn;
{
/* prepare for page layout: allocate layout rectangle plexes and set up
	layout state records */
	struct DOD **hdod, *pdod, **hdodFtn;
	struct DOP *pdop;
	int docFtn;
	struct PLC **hplcpgd;
	struct PLC **hplcpgdFtn;
	struct PGD pgdNew;
	struct PLLR **hpllr;
#ifdef WIN
	struct PLLNBC **hpllnbc = plbsText->hpllnbc;
#else
	int dxuInch;
	struct WWD *pwwd;
	struct STTB **hsttb;
	struct CHP chp;
	struct PAP pap;
	char st[cchMaxSz];
	char stPrinter[cchMaxSz];
#endif

	vmerr.fSavePheFailed = fFalse;	/* allow FSavePhe to try again */
	doc = DocMother(doc);	/* allow footnote doc */

#ifdef WIN
	/* to avoid severe problems with dead fields */
	AssureAllFieldsParsed(doc);
#endif /* WIN */

#ifdef MAC
	pwwd = PwwdWw(ww);
	if (pwwd->wk == wkFtn)
		ww = WwOther(ww);
	else  if (pwwd->wk == wkHdr)
		{
		if (doc != docUndo)
			ww = WwDisp(doc, wwNil, fFalse);
		else
			ww = WwDisp(DocMother(DocBaseWw(ww)), wwNil, fFalse);
		}
#endif

	hdod = mpdochdod[doc];
	if ((*hdod)->docHdr != docNil)
		FCleanUpHdr(doc, fFalse, fFalse);
	CacheSect(doc, cp0);
	AssertH(vfls.hplclnh);
	PutIMacPlc(vfls.hplclnh, 0);
	vfls.ca.doc = docNil;

/* init layout state */
	hpllr = plbsText->hpllr;
	SetWords(plbsText, 0, cwLBS);
	plbsText->hpllr = hpllr;
	hpllr = plbsFtn->hpllr;
	SetWords(plbsFtn, 0, cwLBS);
	plbsFtn->hpllr = hpllr;

	plbsText->doc = doc;
	plbsText->ww = ww;
#ifdef WIN
	plbsText->fSimpleLr = fTrue;
#endif
#ifndef JR
	plbsText->fOutline = PwwdWw(ww)->fOutline;
	plbsText->fFacingPages = FFacingPages(doc);
#endif /* JR */

#ifdef MAC
	/* allow continuous pgn's for next file chain */
	if ((plbsText->pgn = (*hdod)->dop.pgn) == 0)
		plbsText->pgn = vfmtss.pgnStart;
	if (plbsText->pgn == 0 || plbsText->pgn > pgnMaxStart)
		plbsText->pgn = 1;

	plbsText->lnn = (vsepFetch.nLnnMod == 0) ? lnnNil :
			(vsepFetch.lnc == lncContinue) ? (*hdod)->dop.nLnn : 1;
#else
	plbsText->pgn = max(vsepFetch.pgnStart, 1);
	if (plbsText->pgn > pgnMaxStart)
		plbsText->pgn = 1;

	if ((plbsText->hpllnbc = hpllnbc) == hNil)
		{
		plbsText->hpllnbc = HplInit(sizeof(struct LNBC), 1);
		if (plbsText->hpllnbc == hNil)
			return fFalse;
		}

	plbsText->lnn = (vsepFetch.nLnnMod == 0) ? lnnNil :
			vsepFetch.lnnMin + 1;
#endif /* MAC */

	if (vsepFetch.bkc == bkcEvenPage && (plbsText->pgn & 1))
		plbsText->pgn++;
	vfmtss.pgn = plbsText->pgn - 1; /* init */
	Assert(vfmtss.pgn >= 0);
	plbsText->fRight = (plbsText->pgn & 1) != 0;

	if (plbsText->hpllr == hNil)
		{
		plbsText->hpllr = HpllrInit(sizeof(struct LR), 3);
		if (plbsText->hpllr == hNil)
			return fFalse;
		}

	plbsFtn->doc = (*hdod)->docFtn;
	plbsFtn->lnn = lnnNil;
	plbsFtn->ww = ww;
	plbsFtn->fSimpleLr = plbsText->fSimpleLr;
	plbsFtn->pgn = plbsText->pgn;
	if (plbsFtn->hpllr == hNil)
		{
		plbsFtn->hpllr = HpllrInit(sizeof(struct LR), 3);
		if (plbsFtn->hpllr == hNil)
			return fFalse;
		}

/* initialize hplcpgd if necessary */
	SetWords(&pgdNew, 0, cwPGD);
	pgdNew.fUnk = fTrue;
	pgdNew.pgn = plbsText->pgn;
	if ((hplcpgd = (*hdod)->hplcpgd) == hNil)
		{
		hplcpgd = HplcInit(cbPGD, 1, (*hdod)->cpMac, fTrue /* ext rgFoo */);
		if (hplcpgd == hNil)
			return fFalse;
		Win((*hplcpgd)->fMult = fTrue);
		if (!FInsertInPlc(hplcpgd, 0, cp0, &pgdNew))
			{
			FreeHplc(hplcpgd);
			return(fFalse);
			}
		(*hdod)->hplcpgd = hplcpgd;
		}
	if ((docFtn = (*hdod)->docFtn) != docNil && (hplcpgdFtn = PdodDoc(docFtn)->hplcpgd) == hNil)
		{
		hdodFtn = mpdochdod[docFtn];
		hplcpgdFtn = HplcInit(cbPGD, 1, (*hdodFtn)->cpMac, fTrue /* ext rgFoo */);
		if (hplcpgdFtn == hNil)
			return fFalse;
		Win((*hplcpgdFtn)->fMult = fTrue);
		if (!FInsertInPlc(hplcpgdFtn, 0, cp0, &pgdNew))
			{
			FreeHplc(hplcpgdFtn);
			return(fFalse);
			}
		(*hdodFtn)->hplcpgd = hplcpgdFtn;
		}

#ifdef MAC

/* if the printer has changed since the last repagination, we can't trust
	the para heights */
	pdod = *hdod;	/* BEWARE memory movement */
	pdop = &pdod->dop;
	dxuInch = (pdod->qqprr == 0) ? vprsu.dxuInch : ((PRR far *)(*pdod->qqprr))->PrInfo.iHRes;
	Debug( if (!vdbs.fKeepHeights) 
		{ 
		)
				if (vprsu.prid != pdop->pridPhe || dxuInch != pdop->dxuInch ||
				!FRecallFlc(flkPrinter, pdod->hsttbFlc, stPrinter, st, 0) || 
				FNeSt(stPrinter, vprsu.st))
			{
LNoHeights:
			pdod = *hdod;	/* BEWARE memory movement */
			pdop = &pdod->dop;
			pdod->fRepag = pdod->fLRDirty = pdod->fOutlineDirty = fTrue;
			pdop->pridPhe = vprsu.prid;
			pdop->dxuInch = dxuInch;
			if (docFtn != docNil)
				{
				PdodDoc(docFtn)->fLRDirty = fTrue;
				FreePhplc(&PdodDoc(docFtn)->hplcphe);
				}
			if ((*hdod)->docHdr != docNil)
				{
				PdodDoc((*hdod)->docHdr)->fLRDirty = fTrue;
				FreePhplc(&PdodDoc((*hdod)->docHdr)->hplcphe);
				}
			if ((*hdod)->hplcphe != hNil)
				FreePhplc(&(*hdod)->hplcphe);
			hsttb = (*hdod)->hsttbFlc;
			RememberFlc2(flkPrinter, &hsttb, St("-"), vprsu.st, 0L);
			(*hdod)->hsttbFlc = hsttb;
			goto LInvalPgd;
			}
		Debug( 		} 
	);
	if ((*hdod)->dop.fSeeHidden != vpref.fSeeHidden || plbsText->fOutline)
		{
LInvalPgd:      
		SetPlcUnk(hplcpgd, cp0, (*hdod)->cpMac);
		if (docFtn != docNil)
			SetPlcUnk(hplcpgdFtn, cp0, PdodDoc(docFtn)->cpMac);
		}

	(*hdod)->dop.fSeeHidden = vpref.fSeeHidden;

/* forget any previous error */
	vecPrint =  PPRGLOBALS->iPrErr = 0;

/* determine if we should look for PostScript commands:
	printing with LaserWriter, PostScript style exists, style includes
	hidden char property */
	if (vprsu.prid == pridLaser && vlm == lmPrint && !vpref.fPrintHidden &&
			256 - stcPostScript <= (*hdod)->stsh.cstcStd)
		{
		MapStc(*hdod, stcPostScript, &chp, &pap);
		if (chp.fVanish)
			plbsText->fPostScript = plbsText->fSimpleLr = plbsFtn->fSimpleLr = fTrue;
		}
#else
	if (plbsText->fOutline)
		{
		SetPlcUnk(hplcpgd, cp0, (*hdod)->cpMac);
		if (docFtn != docNil)
			SetPlcUnk(hplcpgdFtn, cp0, PdodDoc(docFtn)->cpMac);
		}

#endif
	InvalFli();
#ifdef WIN
	InvalTableProps(selCur.doc, cp0, CpMacDoc(selCur.doc), fTrue/*fFierce*/);
#endif
	return fTrue;
}


/**********************/
/* E n d  L a y o u t */
/* %%Function:EndLayout %%Owner:chrism */
EndLayout(plbsText, plbsFtn)
struct LBS *plbsText, *plbsFtn;
{
/* free everything we can that's associated with layout */
	struct DOD *pdod;
	int ihdt;

	FreePhpl(&plbsText->hpllr);
	FreePhpl(&plbsFtn->hpllr);
	Win( FreePh(&plbsText->hpllnbc) );

/* guarantee not to fail only if it is free and allocate immediately */
	StartGuaranteedHeap();
	FreeHplc(vhplcfrl);
	vhplcfrl = HplcInit(sizeof(struct FRL), 1, cp0, fTrue /* ext rgFoo */);
	EndGuarantee();

	StartGuaranteedHeap();
	FreeHplc(vfls.hplclnh);
	vfls.hplclnh = HplcInit(sizeof(struct LNH), 1, cp0, fTrue /* ext rgFoo */);
	EndGuarantee();

	vfls.ca.doc = docNil;
	caPara.doc = docNil; /* because we may have mucked w/ fInTable */
#ifdef WIN
	InvalTableProps(selCur.doc, cp0, CpMacDoc(selCur.doc), fTrue/*fFierce*/);
#endif
	vcpFirstLayout = cpNil;
	Win( vcpLimLayout = cpNil );
	if (plbsText->fOutline)
		{
		/* the outline page table is worthless and dangerous */
		pdod = PdodDoc(plbsText->doc);
		SetPlcUnk(pdod->hplcpgd, cp0, pdod->cpMac);
		if (pdod->docFtn != docNil)
			{
			pdod = PdodDoc(pdod->docFtn);
			SetPlcUnk(pdod->hplcpgd, cp0, pdod->cpMac);
			}
		}
}


#ifdef MAC
/************************/
/* D i s p l a y  L r s */
/* %%Function:DisplayLrs %%Owner:NOTUSED */
DisplayLrs(plbsText, plbsFtn, fCanAbort)
struct LBS *plbsText, *plbsFtn;
int fCanAbort;
	/* display layout rectangles */
{
	int ilr, yl, ww;
	int ilrMac = (*plbsText->hpllr)->ilrMac;
	int dxuInch = (vfli.fPrint) ? vfli.dxuInch : DxsSetDxsInch(plbsText->doc);
	int dxaLeft, dxaLnn, dxa, xlRight, dyl;
	int fShort;
	int fTop, fDraw = fFalse, fBottom, dylAbove, dylBelow;
	int fTable;
	int *pw, **hw = 0;
	int fPSOver = fFalse;
	CP cpFirstTable, cpFirstLr, cp;
	char ch;
	struct DOD *pdod;
	struct PLC **hplcpad = 0;	/* not zero only for outline */
	struct CA caParaFirst;
	struct LR lr;
	struct TAP tapPrev;
	struct SEP vsepSave;
	struct LBS lbs;
	struct PAD pad;
	struct CHP chp;

	Scribble(2, 'D');
	Mac( InvalWcc() );	/* specifically for footnotes */
	vhpllr = plbsText->hpllr;
	SetWords(&lr, 0, cwLR);

#ifndef JR
/* send PostScript if under text */
	if (plbsText->fPostScript && !(fPSOver = PdodDoc(plbsText->doc)->dop.fPSOver))
		SendPostScript(plbsText, plbsFtn);
#endif /* JR */

/* for non-laser draft printing, sort the layout rectangles */
	if (vfli.fPrint && vprsu.fDraft && vprsu.prid != pridLaser &&
			(hw = HAllocate(ilrMac * sizeof(int))) != hNil)
		{
		for (pw = *hw, ilr = 0; ilr < ilrMac; )
			*pw++ = ilr++;
		Sort(*hw, ilrMac, FLrGt);
		}

/* check outline mode - we have to skip paras */
	if (plbsText->fOutline)
		hplcpad = PdodDoc(plbsText->doc)->hplcpad;

/* display all of the layout rectangles */
	vfli.fLayout = Mac(vfli.fStopAtPara =) fTrue;
	for (ilr = 0, cpFirstTable = cpNil; ilr < ilrMac; ilr++)
		{
		/* check for interrupt */
		if (fCanAbort && FPrAbort())
			break;
		bltLrp(LrpInPl(vhpllr, (hw) ? (*hw)[ilr] : ilr), &lr, sizeof(struct LR));

/* special character LR: chPage, etc. */
		if (lr.doc < 0)
			{
			if (FFormatLineFspec(plbsText->ww, plbsText->doc, lr.dxl, -lr.doc))
				{
				DisplayFliCore(lr.xl - vfli.xpLeft, lr.yl + vfli.dypLine);
				fDraw = fTrue;
				}
			continue;
			}

/* normal LR */
		if (lr.cp == cpNil)
			continue;
		fShort = PdodDoc(lr.doc)->fShort;
		RawLrCps(plbsText, &lr, ww = WwFromLbsDoc(plbsText, lr.doc));
		dxa = NMultDiv(lr.dxl, dxaInch, vfli.dxsInch);
		for (cpFirstLr = lr.cp; lr.cp < lr.cpLim; )
			{
			CachePara(lr.doc, lr.cp);
			fTable = FInTableVPapFetch(lr.doc, lr.cp);

/* outline mode and para not being shown */
			if (hplcpad)
				{
				GetPlc(hplcpad, IInPlc(hplcpad, lr.cp), &pad);
				if (!pad.fShow)
					{
					if (!fTable)
						lr.cp = caPara.cpLim;
					else
						{
						CpFirstTap1(lr.doc, lr.cp, plbsText->fOutline);
						lr.cp = caTap.cpLim;
						}
					continue;
					}
				}

/* table */
			if (fTable)
				{
/* absolute text encountered during a non-abs table lr */
				if (!plbsText->fOutline && FAbsPap(lr.doc, &vpapFetch) && lr.lrk != lrkAbs)
					{
					CpFirstTap1(lr.doc, lr.cp, plbsText->fOutline);
					lr.cp = caTap.cpLim;
					cpFirstTable = cpNil;	/* interrupted other rows */
					continue;
					}
				CpFirstTap1(lr.doc, lr.cp, plbsText->fOutline);
				Assert(caTap.cpFirst == lr.cp);
				fTop = cpFirstTable == cpNil || (lr.lrk == lrkAbs && lr.cp == cpFirstLr);
				CachePara(lr.doc, caTap.cpLim);
				/* may fail if row is followed by abs object */
				if ((lr.lrk == lrkAbs && caTap.cpLim >= lr.cpLim) || !FInTableVPapFetch(lr.doc, caTap.cpLim))
					fBottom = fTrue;
				else  if (caTap.cpLim != lr.cpLim)
					fBottom = fFalse;
				else  if (ilr == ilrMac - 1)
					fBottom = fTrue;
				else 					
					{
					LRP lrp;
					lrp = LrpInPl(vhpllr, ilr + 1);
					fBottom = lr.xl != lrp->xl || lr.doc != lrp->doc;
					}
				dyl = DypHeightTable(ww, lr.doc, lr.cp, fTop, fBottom, &dylAbove, &dylBelow);
				ScanTable(ww, lr.doc, lr.cp, DisplayTcLines, lr.xl, lr.yl + dylAbove);
				CpFirstTap1(lr.doc, lr.cp, plbsText->fOutline);
				Assert(FInCa(lr.doc,lr.cp,&vtcc.caTap));
				PrintTableBorders(ww, lr.xl, lr.yl, dyl, dylAbove, dylBelow, fTop,
						fBottom, &vtapFetch, &tapPrev);
				lr.yl += dyl + dylAbove + dylBelow;
				tapPrev = vtapFetch;
				cpFirstTable = (fBottom) ? cpNil : lr.cp;
				lr.cp = caTap.cpLim;
				fDraw = fTrue;
				continue;
				}

			cpFirstTable = cpNil;
/* absolute text encountered during a non-abs lr */
			if (hplcpad == hNil && FAbsPap(lr.doc, &vpapFetch) && lr.lrk != lrkAbs)
				{
				lr.cp = caPara.cpLim;
				continue;
				}

/* normal text - format a line */
			FormatLineDxa(ww, lr.doc, lr.cp, dxa);
			lr.cp = vfli.cpMac;     /* advance for next line */
			if (vfli.ichMac == 0)
				continue;
			if (vfli.fSplatBreak && vfli.ichMac == 1)
				{
				if (fShort || lr.lrk == lrkAbs)
					{
					lr.yl += vfli.dypLine;
					continue;
					}
				Assert(lr.cp == lr.cpLim);
				break;
				}
			DisplayFliCore(lr.xl, lr.yl += vfli.dypLine);
			fDraw = fTrue;

/* check for line number */
			if (lr.lnn == lnnNil)
				continue;
			if (vpapFetch.fNoLnn || vpapFetch.fSideBySide)
				continue;
			CacheSect(lr.doc, vfli.cpMin);
			if ((vfmtss.lnn = lr.lnn++) % vsepFetch.nLnnMod)
				continue;

/* line number */
			yl = lr.yl - vfli.dypBase;      /* baseline */
			dxaLnn = (vsepFetch.dxaLnn) ? vsepFetch.dxaLnn :
					((vsepFetch.ccolM1) ? dxaInch / 8 : dxaInch / 4);
			if (!FFormatLineFspec(plbsText->ww, plbsText->doc, 0, chLnn))
				continue;
			/* we go left requested amount from text, plus
				enough more to make line number right flush to
				that mark; have to ignore chEop at end */
			xlRight = vfli.xpRight - vfli.rgdxp[vfli.ichMac - 1];
			if (vfli.fPrint)
				xlRight = NMultDiv(xlRight, vfli.dxsInch, vfli.dxuInch);
			DisplayFliCore(lr.xl - xlRight -
					NMultDiv(dxaLnn, vfli.dxsInch, dxaInch),
					/* force lnn onto same baseline as normal text */
			yl + vfli.dypBase);
			}
		}

/* be sure the page gets drawn when printing */
	if (vfli.fPrint && !fDraw)
		{
		StandardChp(&chp);
		LoadFont(&chp, fTrue);
		pdod = PdodDoc(plbsText->doc);
		MoveTo(NMultDiv(pdod->dop.xaPage / 2, vfli.dxsInch, dxaInch),
				NMultDiv(pdod->dop.yaPage / 2, 72, dxaInch));
		DrawChar(' ');
		}

/* send PostScript if it's supposed to go OVER the text */
#ifndef JR
	if (fPSOver)
		SendPostScript(plbsText, plbsFtn);
#endif
	FreeH(hw);
	vfli.fLayout = Mac(vfli.fStopAtPara =) fFalse;
	Scribble(2, ' ');
}


/***********************************/
/* D i s p l a y   T c   L i n e s */
/* %%Function:DisplayTcLines %%Owner:NOTUSED */
DisplayTcLines(ww, xl, yl)
int ww, xl, yl;
{
/* draw a cell for print or preview or pic; vtcc is valid */
	int dyl = 0, dylMax;
	CP cp = vtcc.cpFirst;
	struct RC rcw;
	struct WDS wds;

	CachePara(vtcc.doc, cp);
	if (vpapFetch.fTtp)
		return;
	dylMax = (vtapFetch.dyaRowHeight >= 0) ? ylLarge : YlFromYa(abs(vtapFetch.dyaRowHeight));
	for (xl += vtcc.xpDrLeft; cp < vtcc.ca.cpLim; cp = vfli.cpMac)
		{
		/* dxa 0 is replaced by cell width in FormatLineDxa */
		FormatLineDxa(ww, vtcc.doc, cp, 0);
		if ((dyl += vfli.dypLine) > dylMax)
			{
			Assert(vtapFetch.dyaRowHeight < 0);
			if (dyl - vfli.dypLine < dylMax)
				{
				SaveWds(&wds);
				SetRect(&rcw, xl, yl,
						xl+vtcc.xpDrRight-vtcc.xpDrLeft,
						yl + dylMax - (dyl - vfli.dypLine));
				ClipRect(&rcw);
				DisplayFliCore(xl, yl += vfli.dypLine);
				RestoreWds(&wds);
				}
			break;
			}
		DisplayFliCore(xl, yl += vfli.dypLine);
		}
}


#endif /* MAC */


#ifdef WIN

/* %%Function:DisplayLrTable %%Owner:chrism */
DisplayLrTable(ww, plr, dytAbove)
int ww;
struct LR *plr;
int dytAbove;
{
/* draw a cell for print or preview; vtcc is valid */
	CP cp = vtcc.cpFirst;
	int yt, ytMax, *pdytLine, ylMax;
	int xl, yl, dylAbove, fPrvw;
	struct RC rc;

	CachePara(vtcc.doc, cp);
	if (vpapFetch.fTtp)
		return;

	xl = plr->xl + vtcc.xpDrLeft;
	rc.xpLeft = xl - vtcc.dxpOutLeft;
	rc.xpRight = plr->xl + vtcc.xpDrRight + vtcc.dxpOutRight;

	fPrvw = (vlm == lmPreview);
	if (fPrvw)
		{
		dylAbove = NMultDiv(dytAbove, vfli.dysInch, vfli.dyuInch);
		yl = plr->yl + dylAbove;
		yt = NMultDiv(yl, vfli.dyuInch, vfli.dysInch);
		pdytLine = &vfli.dytLine;
		}
	else
		{
		dylAbove = dytAbove;
		yt = yl = plr->yl + dylAbove;
		pdytLine = &vfli.dypLine;
		}

	/* Don't use YlFromYa because we need printer units! */
	if (vtapFetch.dyaRowHeight >= 0)
		ytMax = ylMax = ylLarge;
	else
		{
		ytMax = yt - dytAbove + NMultDiv(abs(vtapFetch.dyaRowHeight),vfli.dyuInch,czaInch);
		ylMax = fPrvw ? NMultDiv(ytMax, vfli.dysInch, vfli.dyuInch) : ytMax;
		}

	for (; cp < vtcc.ca.cpLim; cp = vfli.cpMac)
		{
		FormatLine(ww, vtcc.doc, cp);
		yt += *pdytLine;
		yl = fPrvw ? NMultDiv(yt, vfli.dysInch, vfli.dyuInch) : yt;
		rc.ypTop = yl - dylAbove - vfli.dypLine;
		rc.ypBottom = min(yl, ylMax);
		if (rc.ypTop > ylMax)
			break;
		if (fPrvw)
			{
			/*
			*  vfPrvwDisp affects ResetFont: if TRUE, current font is
			*  deselected from prvw window DC, else current font is
			*  deselected from all the other screen DCs.  Call Reset
			*  Font here with vfPrvwDisp fFalse to deselect the font
			*  left selected in by FormatLine above.
			*/
			ResetFont(fFalse);
			vfPrvwDisp = fTrue;
			}
		DisplayFliCore(ww, rc, xl, yl);
		if (fPrvw)
			{
			/*
			*  Call ResetFont with vfPrvwDisp fTrue to deselect font
			*  selected into prvw window DC by DisplayFliCore above.
			*/
			ResetFont(fFalse);
			vfPrvwDisp = fFalse;
			}
		else  if (plr->cpMacCounted != cpNil && cp >= plr->cpMacCounted)
			{
			UpdateDsi();
			plr->cpMacCounted = vfli.cpMac;
			}
		if (vfli.fRMark | vfli.fPicture | vfli.fGraphics | vfli.grpfBrc)
			plr->fGraphics = fTrue;
		}
	plr->fGraphics |= (vtcc.brcLeft > 0 || vtcc.brcRight > 0 ||
			vtcc.brcTop > 0 || (vtcc.brcBottom > 0 && vtcc.fLastRow));
}


#endif


/*************************************/
/* M a p  S t c  T o  T e m p  D o c */
/* %%Function:MapStcToTempDoc %%Owner:chrism */
MapStcToTempDoc(doc, stc)
int doc, stc;
{
/* applies looks of stc from a main doc to the temp doc, for lnn's etc. */
	int cb;
	struct CA ca;
	struct CHP chp;
	struct PAP pap;
	char grpprl[cbSectorPre35];

	Assert(vdocTemp != docNil);
	MapStc(PdodDoc(doc), stc, &chp, &pap);
	CachePara(vdocTemp, cp0);

	if (cb = CbGrpprlFromChp(grpprl, &chp, &vchpStc))
		{
		Assert(cb <= cbSectorPre35);
		ca.doc = vdocTemp;
		ca.cpFirst = cp0;
		ca.cpLim = CpMacDoc(vdocTemp);
		ApplyGrpprlCa(grpprl, cb, &ca);
		}
}


/**********************************/
/* F   C r e a t e   C h  S p e c */
/* vdocTemp will contain the desired ch with fSpec and followed by an eop.
Done with minimum fuss by reusing a ch sequence in the scratch file.
*/
/* %%Function:FCreateChSpec %%Owner:chrism */
FCreateChSpec(doc, ch)
int doc;
int ch; /* pass ch = 0 to ensure initialized and nothing more */
{
	int ich;
	struct CA caT;
	struct CHP chp;
	char rgch[chSpecLast];

#ifdef WIN
	if (doc == docUndo)
		{
		Assert(vlm == lmPrint);
		doc = PdodDoc(docUndo)->doc;
		}
#endif

	Assert(doc != docNil);
	Mac(Assert(ch >= 0 && ch <= chSpecLast));
	if (DocCreateTemp(doc) == docNil)
		return(fFalse);

	if (vfcSpec == (FC) -1)
		{
		for (ich = 1; ich <= chSpecLast; ich++)
			rgch[ich - 1] = ich;
		CachePara(vdocTemp, cp0);
		chp = vchpStc;
		chp.fSpec = fTrue;
		if (!FNewChpIns(vdocTemp, cp0, &chp, stcNil))
			return(fFalse);
		vfcSpec = FcAppendRgchToFn(fnScratch, rgch, chSpecLast);
		if (vfInsertMode)
			{
			/* failure here doesn't matter to our success */
			FNewChpIns(selCur.doc, cpInsert, &selCur.chp, stcNil);
			}
		}

	if (ch > 0)
		{
		if (!FRepl1(PcaSetWholeDoc(&caT, vdocTemp), fnScratch, vfcSpec + (FC) (ch - 1), (FC) 1))
			return(fFalse);
		/* correct cpMac: chEop at CpMacDocEdit, terminal chSect,
			guard chEop, plus one character we inserted */
		PdodDoc(vdocTemp)->cpMac = ccpEop * 3 + 1;

		/* blow some caches */
		InvalFli();
		caPara.doc = docNil;
		vdocFetch = docNil;
		}
	return(fTrue);
}


/*******************************/
/* S e t  P g d  I n v a l i d */
/* %%Function:SetPgdInvalid %%Owner:chrism */
NATIVE SetPgdInvalid(ipgd, plbsText, plbsFtn)
int ipgd;
struct LBS *plbsText, *plbsFtn;
{
/* ensure that the next pgd entry describes the next page, and is marked
	dirty if it is not in agreement with the current layout state */
	struct DOD *pdod, **hdod = mpdochdod[plbsText->doc];
	int docFtn;
	struct PLC **hplcpgd = (*hdod)->hplcpgd, **hplcpgdFtn;
	struct PGD pgd, pgdOld, pgdFtn, pgdOldFtn;

/* ensure that the next entry has the appropriate cp */
	EnsureCpPgd(plbsText, ipgd);
	if (vmerr.fMemFail)
		return;
	Assert(ipgd < IMacPlc(hplcpgd));
	if ((docFtn = (*hdod)->docFtn) != docNil)
		{
		pdod = PdodDoc(docFtn);
		hplcpgdFtn = pdod->hplcpgd;
		EnsureCpPgd(plbsFtn, ipgd);
		Assert(ipgd < IMacPlc(hplcpgdFtn));
		}
	GetPlc(hplcpgd, ipgd, &pgd);
	pgdOld = pgd;

/* NOTE: checking for NEXT page is no longer necessary due to dcpDepend */
/* we do not unconditionally invalidate the last page; instead, we
	invalidate the page under consideration when FUpdateHplcpgd stops */
	if (plbsFtn->doc != docNil)
		{
		GetPlc(hplcpgdFtn, ipgd, &pgdFtn);
		pgdOldFtn = pgdFtn;
		if (FNeLbsPgd(plbsFtn, &pgdFtn, fTrue))
			pgdFtn.fUnk = fTrue;	/* pgd out of synch with doc */
		pgdFtn.cl = plbsFtn->cl;
		pgdFtn.lnn = plbsFtn->lnn;
		pgdFtn.pgn = plbsFtn->pgn;
		if (FNeRgw(&pgdOldFtn, &pgdFtn, cwPGD))
			{
			(*hdod)->fRepag |= vlm != lmBRepag; /* main doc */
			PutPlc(hplcpgdFtn, ipgd, &pgdFtn);
			}
		pgd.fUnk |= pgdFtn.fUnk;
		}

	if (FNeLbsPgd(plbsText, &pgd, fFalse))
		pgd.fUnk = fTrue;	/* pgd out of synch with doc */
	pgd.fPgnRestart = plbsText->fPgnRestart;
	pgd.cl = plbsText->cl;
	pgd.lnn = plbsText->lnn;
	pgd.pgn = plbsText->pgn;
	pgd.fRight = plbsText->fRight;
	if (FNeRgw(&pgdOld, &pgd, cwPGD))
		{
		(*hdod)->fRepag |= vlm != lmBRepag;
		PutPlc(hplcpgd, ipgd, &pgd);
		}
}


/***************************/
/* S e t  P g SetPgdValidd  V a l i d */
/* %%Function:SetPgdValid %%Owner:chrism */
NATIVE SetPgdValid(ipgd, plbsText, plbsFtn, fFormat)
int ipgd;
struct LBS *plbsText, *plbsFtn;
/* indicates page has just been reformatted; if false, pgd does not need to
	be updated */
int fFormat;
{
	int dipgd, ipgdMac, ipgdT;
	struct DOD *pdod, **hdod = mpdochdod[plbsText->doc];
	struct PLC **hplcpgd = (*hdod)->hplcpgd, **hplcpgdFtn;
	CP cp, cpPage, cpNext, dcpPage;
	CP cpMac = CpMacDoc(plbsText->doc);
	CP cpTest;
	int clTest;
	struct CA caT;
	struct PGD pgd, pgdOld, pgdFtn, pgdOldFtn;

/* invalidate cp at newly validated page break, so new break gets displayed */
/* cp0 can happen with facing pages */
	if (fFormat && plbsText->cp != cp0)
		{
		if (ipgd == IMacPlc(hplcpgd) - 1)
			{
			cpTest = cpMac;
			clTest = 0;
			}
		else
			{
			cpTest = CpPlc(hplcpgd, ipgd+1);
			GetPlc(hplcpgd, ipgd + 1, &pgd);
			clTest = pgd.cl;
			}
		if (plbsText->cp != cpTest || plbsText->cl != clTest)
			{
			/* invalidate at end of new page - not necessarily 
				handled by SetPgdInvalid */
			cpNext = CpPlc(hplcpgd, ipgd + 1);
			/* cpNext CAN be zero after complex edits */
			if (cpNext == cp0)
				cpNext = cpMac;
			else
				cpNext = CpMin(cpMac, cpNext);
			InvalCp2(plbsText->doc, cpNext - 1, (CP) 1, fFalse);

			cpPage = plbsText->cp;
			if (plbsText->cl == 0)
				dcpPage = (CP) 1;
			else
				{
				CachePara(plbsText->doc, cpPage);
				dcpPage = caPara.cpLim - (cpPage = caPara.cpFirst);
				}
			InvalCp2(plbsText->doc, cpPage - 1, dcpPage, fFalse);
			caPage.doc = docNil;
			InvalLlc();
			}
		}

/* set pgd from lbs */
	GetPlc(hplcpgd, ipgd, &pgd);
	if (fFormat)
		{
		if ((vlm == lmBRepag || vlm == lmFRepag) && !plbsText->fOutline)
			{
			/* update any pageview windows that are showing this 
				page before we dispose the layout rects, but not
				if we come from print because print can have different 
					properties than the ww such as show hidden text. */
			/* outline also doesn't give us anything useful */
			HavePllr(plbsText->doc, plbsText, ipgd);
			}
		pgdOld = pgd;
		pgd.fUnk = fFalse;   /* valid entry */
		pgd.fEmptyPage = plbsText->fEmptyPage;
		pgd.fAllFtn = !plbsText->fEmptyPage && !plbsText->fCpOnPage;
		pgd.bkc = plbsText->bkc;
		pgd.pgn = vfmtss.pgn;
		pgd.fRight = !plbsText->fRight;
		pgd.dcpDepend = plbsText->dcpDepend;
		if (FNeRgw(&pgdOld, &pgd, cwPGD))
			{
			(*hdod)->fRepag |= vlm != lmBRepag;
			PutPlc(hplcpgd, ipgd, &pgd);
			}
		/* footnote hplcpgd */
		if (plbsFtn->doc != docNil)
			{
			pdod = PdodDoc(plbsFtn->doc);
			hplcpgdFtn = pdod->hplcpgd;
			GetPlc(hplcpgdFtn, ipgd, &pgdFtn);
			pgdOldFtn = pgdFtn;
			pgdFtn.fUnk = fFalse;   /* valid entry */
			pgdFtn.pgn = vfmtss.pgn;
			pgdFtn.dcpDepend = plbsFtn->dcpDepend;
			pgdFtn.fContinue = plbsFtn->fContinue;
			if (FNeRgw(&pgdOldFtn, &pgdFtn, cwPGD))
				{
				(*hdod)->fRepag |= vlm != lmBRepag; /* main doc */
				PutPlc(hplcpgdFtn, ipgd, &pgdFtn);
				}
			}
		}

/* end of main doc reached */
	if (plbsText->cp == cpMac)
		{
		if (!plbsFtn->fContinue)
			{
			/* no footnotes at end */
			ipgdMac = IMacPlc(hplcpgd);
			if ((dipgd = ipgd + 1 - ipgdMac) < 0)
				{
				/* make sure that no bogus entries got pushed to the
					end of the plc */
				for (ipgdT = ipgd + 1; ipgdT < ipgdMac; )
					InvalForPgd(plbsText->doc, ipgdT++);
				FOpenPlc(hplcpgd, ipgd + 1, dipgd);
				AdjustIpgds(plbsText->doc, ipgd + 1, -dipgd, fTrue);
				}
			if (plbsFtn->doc != docNil)
				{
				pdod = PdodDoc(plbsFtn->doc);
				hplcpgdFtn = pdod->hplcpgd;
				if ((dipgd = ipgd + 1 - IMacPlc(hplcpgdFtn)) < 0)
					FOpenPlc(hplcpgdFtn, ipgd + 1, dipgd);
				}
			goto LSetLlc;
			}
		}
	else  if (!pgd.fGhost)
		{
/* certain circumstances require that (cp,cl) be converted to a raw cp:
	1. when a paragraph spans a page: two plcpgd cp's would be the same, which
		we only allow for ghost entries
	2. when footnotes may be restarted on each page: cl != 0 is fatal because
		of FormatLine recursion via NAutoFtn->CachePage->CpFromCpCl->ClFormatLines
*/
		if (plbsText->cp <= CpPlc(hplcpgd, ipgd))
			{
			if (plbsText->cl == pgd.cl)
				{
				/* no advance illegal; failsafe */
				Assert(fFalse);
				CachePara(plbsText->doc, plbsText->cp);
				plbsText->cp = caPara.cpLim;
				plbsText->cl = 0;
				}
LReduceCpCl:            
			plbsText->cp = CpFromCpCl(plbsText, fTrue);
			plbsText->cl = 0;
			}
		else  if ((*hdod)->docFtn && plbsText->cl && (*hdod)->dop.fFtnRestart)
			{
			struct PLC **hplcfrd = (*hdod)->hplcfrd;
			int ifrdMac = IMacPlc(hplcfrd);
			if (ifrdMac > 0)
				goto LReduceCpCl;
			}
		}

/* CachePage may have reduced the cp/cl at the next page - check for equality
	and reduce the lbs cp/cl.
	We're looking for a calculated cp/cl in which the cp is less than the
	pgd's cp, but not too far back, and the next entry is clean and reduced
	and the reduced cp's match */
	ipgd++;
	if (fFormat && plbsText->cl != 0 && ipgd < IMacPlc(hplcpgd) &&
			!pgd.fAllFtn &&
			plbsText->cp < (cp = CpPlc(hplcpgd, ipgd)) &&
			plbsText->cp + 1000 >= cp)
		{
		GetPlc(hplcpgd, ipgd, &pgd);
		if (!pgd.fUnk && pgd.cl == 0 && cp == CpFromCpCl(plbsText, fTrue))
			{
			plbsText->cp = cp;
			plbsText->cl = 0;
			}
		}

/* ensure that the next entry will be valid or marked invalid */
	SetPgdInvalid(ipgd, plbsText, plbsFtn);

/* give the appearance of cleanliness on this page */
	ipgd--;
LSetLlc:
	;
#ifdef MAC
	if (vlm == lmBRepag && !vllc.fNumLock && !vllc.fProtect && 
			vllc.fGrayText && vllc.ipgd <= ipgd)
		{
		vllc.doc = docNil;
		CacheLlc(cpNil);
		}
#endif
}


/*****************************/
/* E n s u r e   C p   P g d */
#ifndef WIN
NATIVE /* WINIGNORE - !WIN */
#else
EXPORT /* EnsureCpPgd */
#endif
/* %%Function:EnsureCpPgd %%Owner:chrism */
EnsureCpPgd(plbs, ipgd)
struct LBS *plbs;
int ipgd;
{
/* force the next pgd entry to have correct cp */
	struct DOD *pdod, **hdod = mpdochdod[plbs->doc];
	struct PLC **hplcpgd = (*hdod)->hplcpgd;
	int cIns = 0, cDel = 0, c;
	CP cpT, cpLastPage, dcpLastPage;
	struct PGD pgd, pgdNew;

	SetWords(&pgdNew, 0, cwPGD);
	pgdNew.fUnk = fTrue;
	while ((cpT = CpPlc(hplcpgd, ipgd)) != plbs->cp ||
			ipgd == IMacPlc(hplcpgd))
		{
		if (cpT >= plbs->cp)
			{
			cIns++;
			if (!FInsertInPlc(hplcpgd, ipgd, plbs->cp, &pgdNew))
				return;
			caPage.doc = docNil;
			}
		else
			{
			if (!(*hdod)->fShort)
				InvalForPgd(plbs->doc, ipgd);
			caPage.doc = docNil;
			InvalLlc();
			cDel++;
			FOpenPlc(hplcpgd, ipgd, -1);
			}
		pdod = PdodMother(plbs->doc);
		pdod->fRepag |= vlm != lmBRepag;
		}
	AdjustIpgds(plbs->doc, ipgd, cIns, fFalse);
	AdjustIpgds(plbs->doc, ipgd, cDel, fTrue);
}


/******************************/
/* I n v a l   F o r  I p g d */
/* %%Function:InvalForPgd %%Owner:chrism */
InvalForPgd(doc, ipgd)
int doc, ipgd;
{
/* inval a page break we are deleting */
	struct PLC **hplcpgd = PdodDoc(doc)->hplcpgd;
	CP cpInval, dcpInval, cpMac = CpMacDocEdit(doc);
	struct PGD pgd;

	Assert(ipgd < IMacPlc(hplcpgd));
	cpInval = CpMin(cpMac, CpPlc(hplcpgd, ipgd));
	if (cpInval < 0)
		return;
	GetPlc(hplcpgd, ipgd, &pgd);
	if (pgd.cl == 0)
		dcpInval = (CP) 1;
	else
		{
		CachePara(doc, cpInval);
		cpInval = caPara.cpFirst;
		dcpInval = caPara.cpLim - cpInval;
		}
	InvalCp2(doc, cpInval - 1, dcpInval, fFalse);
}


/****************************************/
/* A d v a n c e  L b s  F r o m  P g d */
#ifndef WIN
NATIVE /* WINIGNORE - !WIN */
#else
EXPORT /* AdvanceLbsFromPgd */
#endif
/* %%Function:AdvanceLbsFromPgd %%Owner:chrism */
AdvanceLbsFromPgd(plbsText, plbsFtn, ipgd)
struct LBS *plbsText, *plbsFtn;
int ipgd;
{
/* when a pgd entry is valid, skip the page by advancing the lbs from the
	following pgd entry (the following entry is passed) */
	struct DOD *pdod;
	struct PLC **hplcpgd;
	struct PGD pgd;

/* footnote lbs */
	if (plbsFtn->doc != docNil)
		{
		pdod = PdodDoc(plbsFtn->doc);
		hplcpgd = pdod->hplcpgd;
		plbsFtn->cp = CpPlc(hplcpgd, ipgd);
		GetPlc(hplcpgd, ipgd, &pgd);
		plbsFtn->cl = pgd.cl;
		plbsFtn->lnn = pgd.lnn;
		plbsFtn->pgn = pgd.pgn;
		/* continue flag is stored in previous pgd */
		if (ipgd == 0)
			plbsFtn->fContinue = fFalse;
		else
			{
			GetPlc(hplcpgd, ipgd - 1, &pgd);
			plbsFtn->fContinue = pgd.fContinue;
			}
		}

/* text lbs */
	pdod = PdodDoc(plbsText->doc);
	hplcpgd = pdod->hplcpgd;
	plbsText->cp = CpPlc(hplcpgd, ipgd);
	GetPlc(hplcpgd, ipgd, &pgd);
	plbsText->cl = pgd.cl;
	plbsText->lnn = pgd.lnn;
	plbsText->pgn = pgd.pgn;
	vfmtss.pgn = plbsText->pgn;
	plbsText->fRight = pgd.fRight;
	plbsText->fPgnRestart = pgd.fPgnRestart;
	/* lbc is set primarily for printing code */
	if (plbsText->cp >= CpMacDoc(plbsText->doc))
		plbsText->lbc = (plbsFtn->fContinue) ? lbcYlLim : lbcEndOfDoc;
	else
		{
		CacheSect(plbsText->doc, plbsText->cp);
		Assert(ipgd > 0);
		if (plbsText->cp == caSect.cpFirst && plbsText->cl == 0 && 
				CpPlc(hplcpgd, ipgd-1) < plbsText->cp && !plbsFtn->fContinue)
			{
			plbsText->lbc = lbcEndOfSection;
			NewSectLayout(plbsText, fFalse);
			}
		else
			plbsText->lbc = lbcYlLim; /* not always true, but adequate */
		}
}


/************************/
/* F  N e  L b s  P g d */
/* %%Function:FNeLbsPgd %%Owner:chrism */
FNeLbsPgd(plbs, ppgd, fFtn)
struct LBS *plbs;
struct PGD *ppgd;
int fFtn;
{
	if (ppgd->cl != plbs->cl)
		return(fTrue);
	if (fFtn)
		return(fFalse);
	return(ppgd->pgn != plbs->pgn || ppgd->lnn != plbs->lnn ||
			ppgd->fPgnRestart != plbs->fPgnRestart ||
			ppgd->fRight != plbs->fRight);
}


/*************************/
/* F  P r i n t  D o n e */
/* %%Function:FPrintDone %%Owner:chrism */
FPrintDone(plbs, scn, pgn)
struct LBS *plbs;
uns scn, pgn;
/* returns fTrue if plbs->cp is past scn, or in scn and past pgn.
	if there is only 1 section, it is assumed that the sections match.
	section numbers and (typically) page numbers start with one.

	section numbers of zero match if the page number matches
*/
{
	struct PLC **hplcsed = PdodDoc(plbs->doc)->hplcsed;
	int isedMac, scnCur;

	Assert(hplcsed != hNil);
	isedMac = IMacPlc(hplcsed);
	if (isedMac > 2)
		{
		if (scn > 0)
			{
			/* a one-section document has IMacPlc of 2 */
			scn = min(scn, isedMac - 1);
			scnCur = IInPlc(hplcsed, plbs->cp) + 1;
			if (scnCur > scn)
				return(fTrue);
			if (scnCur < scn)
				return(fFalse);
			}
		}
	return(plbs->pgn > pgn);
}


#ifdef WIN
#ifdef DEBUG
/********************************/
/* W w   F r o m   L b s  D o c */
/* %%Function:WwFromLbsDoc %%Owner:chrism */
int WwFromLbsDoc(plbs, doc)
struct LBS *plbs;
int doc;
{
	if (doc == plbs->doc)
		return(plbs->ww);
	LinkDocToWw(doc, WinMac(wwLayout, wwTemp), wwNil);
	return(WinMac(wwLayout, wwTemp));
}
#endif /* DEBUG */
#endif /* WIN */

#ifdef MAC
/********************************/
/* W w   F r o m   L b s  D o c */
/* %%Function:WwFromLbsDoc %%Owner:chrism */
int WwFromLbsDoc(plbs, doc)
struct LBS *plbs;
int doc;
{
	if (doc == plbs->doc)
		return(plbs->ww);
	LinkDocToWw(doc, WinMac(wwLayout, wwTemp), wwNil);
	return(WinMac(wwLayout, wwTemp));
}
#endif

/***********************/
/* R a w   L r   C p s */
/* %%Function:RawLrCps %%Owner:chrism */
RawLrCps(plbs, plr, ww)
struct LBS *plbs;
struct LR *plr;         /* must be on frame, not in heap */
int ww;
{
/* convert cp,cl pairs in lr to cp,0 pairs */
#ifdef WIN
	int flmSave, lmSave;
#endif
	struct LBS lbs;

	if (plr->clFirst != 0 || plr->clLim != 0)
		{
		lbs = *plbs;
		lbs.ww = ww;
		lbs.doc = plr->doc;

#ifdef WIN
		flmSave = vflm;
		if (vflm != flmRepaginate && vflm != flmPrint)
			{
			lmSave = vlm;
			FResetWwLayout(DocMotherLayout(lbs.doc), vlm == lmPrint ? flmPrint : flmRepaginate, 
				(vlm != lmNil) ? vlm : lmPagevw /* most forgiving */);
			lbs.ww = wwLayout;
			}
		Assert(lbs.ww == wwLayout);
#endif

		if (lbs.cl = plr->clFirst)
			{
			lbs.cp = plr->cp;
			plr->cp = CpFromCpCl(&lbs, fTrue);
			}
		if (lbs.cl = plr->clLim)
			{
			lbs.cp = plr->cpLim;
			plr->cpLim = CpFromCpCl(&lbs, fTrue);
			}

#ifdef WIN
		if (vflm != flmSave)
			FResetWwLayout(DocMotherLayout(lbs.doc), flmSave, lmSave);
#endif

		plr->clFirst = plr->clLim = 0;
		}
}


/***************************************/
/* D o c   C p   W i t h i n   P a g e */
/* %%Function:DocCpWithinPage %%Owner:chrism */
int DocCpWithinPage(pcp)
CP *pcp;
{
/* returns mother doc and a cp within the "current" page; for the purpose
	of showing the current page in preview or page view */
	int docMom = DocMother(selCur.doc);
	CP cp;

	Assert(!(*hwwdCur)->fPageView);
	if (selCur.doc == docMom)
		cp = PdrGalley(*hwwdCur)->cpFirst;
	else  if (PdodDoc(selCur.doc)->fFtn)
		cp = CpRefFromCpSub(selCur.doc, PdrGalley(*hwwdCur)->cpFirst, edcDrpFtn);
	else  if (PdodDoc(selCur.doc)->fAtn)
		cp = CpRefFromCpSub(selCur.doc, PdrGalley(*hwwdCur)->cpFirst, edcDrpAtn);
	else
		{
		/* header window */
		Assert(PdodDoc(selCur.doc)->fDispHdr);
		Mac(Assert(wwCur == vwwHdr));
		cp = CpMomFromHdr(selCur.doc);
		}
	*pcp = cp;
	return(docMom);
}


/*******************************/
/* T  C O M P A R E  P L D R S */
/* returns: 
	tPos	dr layout differs
	tZero	equal
	tNeg	dr layout is same, contents differ.
*/
/* %%Function:TComparePldrs %%Owner:chrism */
TComparePldrs(hpldrOld, hpldrNew)
struct PLDR **hpldrOld, **hpldrNew;
{
	int idrMacOld = IMacPlc(hpldrOld);
	int idr;
	int t = tZero;
	struct DR *pdrOld, *pdrNew;
	struct DRF drfFetchOld, drfFetchNew;

	if (IMacPlc(hpldrNew) != idrMacOld) return tPos;
	for (idr = 0; idr < idrMacOld; idr++)
		{
		pdrOld = PdrFetchAndFree(hpldrOld, idr, &drfFetchOld);
		pdrNew = PdrFetchAndFree(hpldrNew, idr, &drfFetchNew);
		if (pdrOld->doc != pdrNew->doc ||
				pdrOld->lrk != pdrNew->lrk ||
				pdrOld->idrFlow != pdrNew->idrFlow ||
				FNeRgw(&pdrOld->drcl, &pdrNew->drcl, cwRC))
			return tPos;
		if (pdrOld->cpFirst != pdrNew->cpFirst ||
				pdrOld->cpLim != pdrNew->cpLim)
			t = tNeg;
		}
	return t;
}


/********************/
/* H A V E  P L L R */
/* before a pllr is disposed, any window which displays the page in question will
be updated.
*/
/* %%Function:HavePllr %%Owner:chrism */
HavePllr(doc, plbs, ipgd)
int doc;
struct LBS *plbs;
int ipgd;
{
	BOOL fSetup = fTrue;
	int ww;
	int idr, idrMac;
	int t;
	struct PLDR **hpldrNear;
	struct DR *pdrNew, *pdrOld;
	struct WWD *pwwd, **hwwd;
	BOOL f;

/* enumerate windows which display the document */
	for (ww = WwDisp(doc, wwNil, fTrue);
			ww != wwNil; ww = WwDisp(doc, ww, fTrue))
		{
		hwwd = HwwdWw(ww);
		pwwd = *hwwd;
		if (!pwwd->fPageView)
			continue;
		if (pwwd->ipgd == ipgd)
			{
			if (fSetup)
				{
				if ((hpldrNear = HplInit(cwDR * sizeof(int), (*plbs->hpllr)->ilrMac)) == hNil)
					return;
				fSetup = fFalse;
/* setting the fPageView for WIN case is so that vfli.dypLine gets the correct unit */
				Win(PwwdWw(wwLayout)->fPageView = fTrue);
				f = FPldrFromPllr(doc, hpldrNear, WinMac(wwLayout, ww), plbs);
				Win(PwwdWw(wwLayout)->fPageView = fFalse);
				if (!f)
					break;
				}
/* ww displays the page in question and we have the hpldr to compare with */
			t = TComparePldrs(hpldrNear, hwwd);
			idrMac = (*hpldrNear)->idrMac;
			if (t == tPos)
				{
/* set the rgdr structure in the window to the same in hpldr: free old stuff,
make sure there is enough space, then blt stuff */
				FreeDrs(hwwd, 0);
				if (!FSetIdrMac(hwwd, idrMac))
					break;
				blt(PInPl(hpldrNear, 0), PInPl(hwwd, 0), cwDR * idrMac);
				(*hwwd)->fDrDirty = fFalse;
				(*hwwd)->fDirty = fTrue;
				WinMac(DrawBlankPage(ww, NULL), DrawBlankPage(ww));
				plbs->fBkRepagStop = fTrue;
				}
			else  if (t == tNeg)
				{
				pdrNew = PInPl(hwwd, 0);
				pdrOld = PInPl(hpldrNear, 0);
				for (idr = 0; idr < idrMac; idr++, pdrNew++, pdrOld++)
					{
					if (pdrOld->cpFirst != pdrNew->cpFirst ||
							pdrOld->cpLim != pdrNew->cpLim)
						{
						pdrNew->cpFirst = pdrOld->cpFirst;
						pdrNew->cpLim = pdrOld->cpLim;
						pdrNew->fLimSuspect = fFalse;
						pdrNew->fDirty = fTrue;
						}
					}
				(*hwwd)->fDirty = fTrue;
				plbs->fBkRepagStop = fTrue;
				}
			}
		}
	if (!fSetup)
		FreeH(hpldrNear);
}


#ifdef WIN

#ifdef JBL
/* %%Function:DumpHplcpgd %%Owner:chrism */
DumpHplcpgd(hplcpgd,sz)
struct PLC **hplcpgd;
char *sz;
{
	extern char szEmpty[];
	int ipgd, ipgdMac;
	struct PGD pgd;
	int rgfValid[50];

	CommSzSz(SzFrame("------------------"),szEmpty);
	CommSzSz(sz,SzFrame(":"));
	if (hplcpgd == hNil)
		{
		CommSzSz(SzFrame("hplcpgd is nil"),szEmpty);
		return;
		}
	CommSzNum(SzFrame("ipgdMac is: "),ipgdMac = IMacPlc(hplcpgd));
	Assert( ipgdMac < 50 );

	for ( ipgd = 0 ; ipgd < ipgdMac ; ipgd++ )
		{
		GetPlc( hplcpgd, ipgd, &pgd );
		rgfValid [ipgd] = !pgd.fUnks;
		}
	CommSzRgNum(SzFrame("fValid bits: "),rgfValid, ipgdMac);
}


#endif
#endif /* WIN */


#ifdef MAC

/*******************************/
/* P a r s e  P a g e  S e c t */
/* %%Function:ParsePageSect %%Owner:NOTUSED */
ParsePageSect(msg, pst, ppval, tmc)
int msg;
char *pst;
int **ppval;
int tmc;
/* parse a page/section number
	valid forms are: ppp, dsss, pppdsss, pppd, and null
	where: ppp = page number, d = 'S' or 's', sss = section number
	the number may be preceded with 'P' or 'p', which is ignored
	a single space may be used between the page number and 's': "1 s2"
*/
{
	int cch, cchMac;
	char ch, *pch;
	char st[cchMaxSz];

	switch (msg)
		{
	case msgParse:
		CopySt(pst, *ppval);
		pch = (char *) *ppval + 1;
		cch = 0;
		if (ChUpper(*pch) == vitr.chPageNo)  /* 'P' */
			{
			cch++;
			pch++;
			}
		for (cchMac = *pst; cch < cchMac; cch++, pch++)
			{
			ch = ChUpper(*pch);
			if (ch == ' ' || ch == vitr.chSectNo) /* 'S' */
				{
				cch++;
				pch++;
				if (ch == ' ')
					{
					if (ChUpper(*pch) != vitr.chSectNo)
						goto LBadPageSect;
					cch++;
					pch++;
					}
				if (cch == cchMac)
					goto LBadPageSect;
				break;
				}
			if (ch < '0' || ch > '9')
				goto LBadPageSect;
			}
		for (; cch < cchMac; cch++, pch++)
			if (*pch < '0' || *pch > '9')
				goto LBadPageSect;
		return(fTrue);
LBadPageSect:
		SelectTextTmc(tmc, 0, 0x7fff);
		SystemBeep(1);
		return(fFalse);
	case msgFormat:
		CopySt(*ppval, pst);
		return;
	case msgCwVal:
		if (pst == 0)
			StFromTmc(pst = st, tmc);
		return(max(sizeof(int) + 1, CwFromCch(*pst + 1)));
		}
}


/**********************************/
/* P a g e  S e c t  F r o m  S t */
/* %%Function:PageSectFromSt %%Owner:NOTUSED */
PageSectFromSt(st, ppgn, pscn)
char *st;
int *ppgn, *pscn;
	/* convert a page/section string to the corresponding page/section numbers--
		valid forms same as above
		default for an omitted number is zero
	*/
{
	int pgn = 0, scn = 0;
	int cch = 0, cchMac = st[0];
	char ch, *pch = &st[1];

	if (ChUpper(*pch) == vitr.chPageNo)  /* 'P */
		{
		cch++;
		pch++;
		}
	for (; cch < cchMac; cch++, pch++)
		{
		ch = ChUpper(*pch);
		if (ch == vitr.chSectNo || ch == ' ')
			break;
		pgn = pgn * 10 + ch - '0';
		}
	for (; cch < cchMac; cch++, pch++)
		{
		ch = ChUpper(*pch);
		if (ch != vitr.chSectNo && ch != ' ')
			break;
		}
	for (; cch < cchMac; cch++, pch++)
		scn = scn * 10 + *pch - '0';
	*ppgn = pgn;
	*pscn = scn;
}


/******************************/
/* P r i n t e r  C h a n g e */
/* %%Function:PrinterChange %%Owner:NOTUSED */
PrinterChange()
{
/* the user has changed either the printer or the way the printer effects
	the screen - recreate vprsu and its effect on vfli */
	int fChanged;

	FEnsurePrintInited(fFalse, &fChanged);
/* note that all this will be changed for ImageWriter the first time that
	DxsSetDxsInch() is called */
	if (fChanged)
		{
		vfli.dxsInch = vfli.dxuInch;
		TrashAllWws();
		TrashRuler();
		if (wwCur != wwNil && FRulerUp(wwCur))
			UpdateRuler();
		ReDrawAllRulers(fFalse);
		InvalCaFierce();
		}
}


/***************/
/* F  L r  G t */
/* %%Function:FLrGt %%Owner:NOTUSED */
FLrGt(ilr0, ilr1)
{
	LRP lrp0 = LrpInPl(vhpllr, ilr0);
	LRP lrp1 = LrpInPl(vhpllr, ilr1);

	return(lrp0->yl > lrp1->yl ||
			(lrp0->yl == lrp1->yl && lrp0->xl > lrp1->xl));
}


/*****************************************/
/* I n i t   F l i   F o r   L a y o u t */
/* %%Function:InitFliForLayout %%Owner:NOTUSED */
InitFliForLayout(lm, pfVisi)
int lm, *pfVisi;
{
	InvalFli();
	*pfVisi = vfVisiSave = FFormatVisi();
	ftcLast = -1;   /* reset font */
	vlm = lm;
	SetFormatVisi(fFalse);
}


/*******************************/
/* E n d   F l i   L a y o u t */
/* %%Function:EndFliLayout %%Owner:NOTUSED */
EndFliLayout(lm, fVisi)
int lm, fVisi;
{
	InvalFli();      /* force line format */
	SetFormatVisi(fVisi);
	ftcLast = -1;   /* reset font */
	vlm = lm;
}


#endif /* MAC */


/* F   F a c i n g   P a g e s */
/* %%Function:FFacingPages %%Owner:chrism */
EXPORT FFacingPages(doc)
int doc;
{
	struct DOP *pdop = &PdodDoc(doc)->dop;

	return(pdop->fFacingPages || pdop->dxaGutter != 0 ||
			pdop->fMirrorMargins);
}


/*************************/
/* F   M a t c h   A b s */
/* %%Function:FMatchAbs %%Owner:chrism */
EXPORT int FMatchAbs(doc, ppap1, ppap2)
int doc;
struct PAP *ppap1, *ppap2;
{
/* returns true if two paps describe identical APOs */
	if (ppap1->dxaAbs != ppap2->dxaAbs ||
			ppap1->dyaAbs != ppap2->dyaAbs ||
			ppap1->pc != ppap2->pc ||
			ppap1->dxaWidth != ppap2->dxaWidth)
		{
		return(fFalse);
		}
	return(!FAbsPap(doc, ppap1) || ppap1->dxaFromText == ppap2->dxaFromText);
}



/* P R E P A R E  D O C  F I E L D S  F O R  P R I N T */
/* Updates fields in doc prior to printing.  if pfpPrinting will guarantee
   page table up to date if needed for correct fields (esp pageref).  if
   pfpRefreshAll, updates all fields in doc.  may return fFalse iff
   pfpPrinting.
*/
/* %%Function:FPrepareDocFieldsForPrint %%Owner:peterj */
FPrepareDocFieldsForPrint (doc, pfp)
int doc;
int pfp;

{
	int cT;
	int docT;
	int fNoAbort;
	int frm = frmPrint | ((pfp & pfpRefreshAll) ? frmUser : 0);
	int frmHdr = vppmd == NULL ? frmHdr1st : 0;

	/*  fRefreshAll should never be set during PrintMerge */
	Assert (vppmd == NULL || !(pfp & pfpRefreshAll));
	Assert (!PdodDoc(doc)->fShort);

	ClearFDifferFltg(doc, fltgAll);
	AssureAllFieldsParsed(doc);

	if (pfp & pfpPrinting)
		/* printing specific */
		{
		/*  repaginate if necessary so that fields are up to date */
		if (FMustPrePaginate (doc))
			{
			/* assure page table is up to date */
			struct LBS lbsText, lbsFtn;
			struct RPL rpl;

			InvalFli();
			lbsText.hpllr = lbsFtn.hpllr = hNil;
			lbsText.hpllnbc = lbsFtn.hpllnbc = hNil;
			lbsText.fOutline = lbsFtn.fOutline = fFalse;
			SetWords (&rpl, pgnMax, cwRPL);
			rpl.cp = cpMax;
			LinkDocToWw(doc, wwLayout, wwNil);
			fNoAbort = FUpdateHplcpgd(wwLayout, doc, &lbsText, &lbsFtn,&rpl,patAbort);
			EndLayout (&lbsText, &lbsFtn);
			if (!fNoAbort)
				return fFalse;
			}

		/*  set print date */
		PdodDoc(doc)->dop.dttmLastPrint = DttmCur();
		}

	/* take care of fields */
	AcquireCaAdjust ();

	Assert(!PdodDoc(doc)->fShort);

	/*  header/footer */
	if (FHasFields(docT = PdodDoc (doc)->docHdr))
		if (FSetPcaForCalc (&caAdjust, docT, cp0, CpMacDoc (docT), &cT))
			FCalcFields (&caAdjust, frmHdr | frm, fFalse, fTrue);

	/*  footnotes */
	if (FHasFields(docT = PdodDoc (doc)->docFtn))
		if (FSetPcaForCalc (&caAdjust, docT, cp0, CpMacDoc (docT), &cT))
			FCalcFields (&caAdjust, frmPaginate | frm, fFalse, fTrue);

	/*  annotations */
	if (FHasFields(docT = PdodDoc (doc)->docAtn))
		if (FSetPcaForCalc (&caAdjust, docT, cp0, CpMacDoc (docT), &cT))
			FCalcFields (&caAdjust, frmPaginate|frm, fFalse, fTrue);

	/*  main doc */
	if (FHasFields(doc))
		if (FSetPcaForCalc (&caAdjust, doc, cp0, CpMacDoc (doc), &cT))
			FCalcFields (&caAdjust, frmPaginate | frm, fFalse, fTrue);

	ReleaseCaAdjust ();
	return fTrue;
}


