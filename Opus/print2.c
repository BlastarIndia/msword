#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"

#include "idd.h"
#include "sdmdefs.h"
#include "sdmver.h"
#include "sdm.h"
#include "sdmtmpl.h"

#include "dlbenum.h"
#include "doc.h"
#include "disp.h"
#include "props.h"
#include "border.h"
#include "screen.h"
#include "sel.h"
#include "field.h"
#include "file.h"
#include "format.h"
#include "layout.h"
#include "print.h"
#include "style.h"
#include "debug.h"
#include "prm.h"
#include "pic.h"
#include "ch.h"
#include "prompt.h"
#define REVMARKING
#include "compare.h"
#undef REVMARKING
/* #include "docsum.h" */
#include "doslib.h"
#include "error.h"
#include "message.h"
#include "rerr.h"
#include "resource.h"

#include "chgpr.hs"
#include "chgpr.sdm"

#include "sumstat.h"

#include "docsum.hs"
#include "docstat.hs"


#define cchBufMax       25

extern int              vdocStsh;
extern int              docGlsy;
extern int              vpgnFirst, vpgnLast, vsectFirst, vsectLast;
extern int              vfShowAllStd;
extern int              vistLbMac;
extern BOOL		vfRecording;
extern int              fElActive;
extern int              vccpFetch;
extern CHAR HUGE        *vhpchFetch;
extern char             (**vhmpiststcp)[];
extern char             (**vhgrpchr)[];
extern KMP              **vhkmpUser;
extern struct FLI       vfli;
extern struct FTI       vfti;
extern struct TCC       vtcc;
extern struct CHP       vchpStc;
extern struct CHP       vchpFetch;
extern struct PAP       vpapFetch;
extern struct SEP       vsepFetch;
extern struct TAP       vtapFetch;
extern struct PRI       vpri;
extern struct CA        caPara;
extern struct CA        caParaL;
extern struct CA        caSect;
extern struct MERR      vmerr;
extern struct PRSU      vprsu;
extern struct FMTSS     vfmtss;
extern CHAR             szNone[];
extern HWND vhwndApp;

extern CP CpFromIpgd();

csconst char stOn[] = StKey(" on ",On);

csconst char stNextStyle[] = StKey("NextStyle:\t",NextStyle);
csconst char stGlsyTemplate[] = StKey("Glossaries for Document Template:  ",GlsyTemplate);
csconst char stGlsyGlobal[] = StKey("Global Glossaries",GlsyGlobal);
csconst char stDotKeys[] = StKey("Key Assignments for Document Template:  ",DotKeys);
csconst char stGlobalKeys[] = StKey("Global Key Assignments",GlobalKeys);


/* *****  Fill Undo with various things  ***** */

/* %%Function:FFillUndoStsh %%Owner:davidbo */
FFillUndoStsh(docMother)
int docMother;
{
	int ist, istMac, cch, cchNS;
	int stcBase, stcNext, stc, stcp, stcpNext;
	char ch;
	struct STTB **hsttbChpe, **hsttbPape;
	CP cp = cp0;
	struct CA caT;
	struct STSH stsh;
	struct CHP chpName, chpText;
	struct PAP papName, papText;
	char rgch[cchMaxSz];
	char rgchNS[cchMaxSz];

	SetUndoNil();
	CachePara(docUndo, cp0);
	chpName = vchpStc;
	chpText = vchpStc;
	chpName.fBold = fTrue;
	StandardPap(&papName);
	papName.fKeepFollow = fTrue;
	papName.dyaBefore = 12 * dyaPoint;
	papName.dxaLeft = dxaInch;
	StandardPap(&papText);
	papText.dxaLeft = (3 * dxaInch) >> 1;
	papText.fKeep = fTrue;

	vistLbMac = 0;
	if ((vhmpiststcp = HAllocateCw(cwMpiststcp)) == hNil)
		return fFalse;

	RecordStshForDoc(vdocStsh = docMother, &stsh, &hsttbChpe, &hsttbPape);
	GenLbStyleMapping();
	istMac = vistLbMac;

	CopyCsSt(stNextStyle, rgchNS);
	cchNS = *rgchNS;

	for (ist = 0; ist < istMac && !vmerr.fMemFail && !vmerr.fDiskFail; ist++)
		{
		stcp = (**vhmpiststcp)[ist];
		stc = StcFromStcp(stcp, stsh.cstcStd);
		if ((vfShowAllStd && stcp < stsh.cstcStd) ||
				(stc >= stcLev3 && stc <= stcLev1) ||
				!FStcpEntryIsNull(stsh.hsttbName, stcp))
			GenStyleNameForStcp(rgch, stsh.hsttbName, stsh.cstcStd, stcp);
		else
			continue;

		cch = min(rgch[0], cchMaxSz - (int)ccpEop - 1);
		rgch[++cch] = chReturn;
		rgch[++cch] = chEop;
		if (!FInsertRgch(docUndo, cp, &rgch[1], cch, &chpName, &papName))
			return fFalse;
		cp += (CP) cch;

		GetStcBaseNextForStcp(&stsh, stcp, &stcBase, &stcNext);
		stcpNext = StcpFromStc(stcNext, stsh.cstcStd);
		if (stcpNext != stcp)
			{
			GenStyleNameForStcp(rgch, stsh.hsttbName, stsh.cstcStd, stcpNext);
			rgch[0] = min(rgch[0], cchMaxSz - (int)ccpEop - 1 - cchNS);
			StStAppend(rgchNS, rgch);
			cch = rgchNS[0];
			rgchNS[++cch] = chReturn;
			rgchNS[++cch] = chEop;
			if (!FInsertRgch(docUndo, cp, &rgchNS[1], cch, &chpText, &papName))
				return fFalse;
			rgchNS[0] = cchNS;
			cp += (CP) cch;
			}

		SetIbstFontDSFromStc(stc);
		GenStyleBanter(rgch, cchMaxSz - (int)ccpEop - 1, stc, &stsh, hsttbChpe, hsttbPape);
		cch = rgch[0];
		Assert(cch <= (cchMaxSz - (int)ccpEop - 1));
		rgch[++cch]= chReturn;
		rgch[++cch]= chEop;
		if (!FInsertRgch(docUndo, cp, &rgch[1], cch, &chpText, &papText))
			return fFalse;
		cp += (CP) cch;
		}

	FreePh(&vhmpiststcp);
	return fTrue;
}


/* %%Function:FFillUndoGlsy %%Owner:davidbo */
FFillUndoGlsy(docMother)
int docMother;
{
	int ist, istMac, doc, cch, cchStatic, iLoop;
	char ch;
	struct STTB **hsttbGlsy;
	CP dcp, cp;
	struct CA ca, caSprm;
	struct CHP chpName, chpText;
	struct PAP papName, papText;
	char rgb[4];
	char rgchStatic[cchMaxSz];
	char rgch[cchMaxSz];

	SetUndoNil();
	CachePara(docUndo, cp0);
	chpName = vchpStc;
	chpText = vchpStc;
	chpName.fBold = fTrue;
	StandardPap(&papName);
	papName.fKeepFollow = fTrue;
	StandardPap(&papText);
	papText.dxaLeft = dxaInch / 2;
	papText.fKeep = fTrue;

	for (iLoop = 0; iLoop < 2; iLoop++)
		{
		doc = (iLoop == 0) ? DocGlsyLocal() : docGlsy;
		cp = (iLoop == 0) ? cp0 : CpMacDocEdit(docUndo);
		Assert(cp >= cp0);
		if (doc == docNil || ((hsttbGlsy = PdodDoc(doc)->hsttbGlsy) == hNil))
			continue;

		CopyCsSt((iLoop == 0 ? stGlsyTemplate : stGlsyGlobal), rgchStatic);
		cchStatic = *rgchStatic;

		if (iLoop == 0)
			{
			int docDot = DocDotMother(docMother);
			Assert(docDot != docNil);
			GetDocSt(docDot, rgch, gdsoFullPath);
			rgch[0] = min(rgch[0], cchMaxSz - (int)ccpEop - 1 - cchStatic);
			StStAppend(rgchStatic, rgch);
			}
		cch = rgchStatic[0];
		rgchStatic[++cch] = chReturn;
		rgchStatic[++cch] = chEop;
		papName.dxaLeft = 0;
		if (!FInsertRgch(docUndo, cp, &rgchStatic[1], cch, &chpName, &papName))
			return fFalse;
		cp += (CP) cch;

		papName.dyaBefore = 10 * dyaPoint;
		papName.dxaLeft = dxaInch / 2;
		istMac = (*hsttbGlsy)->ibstMac;
		for (ist = 0; ist < istMac && !vmerr.fMemFail &&!vmerr.fDiskFail; ist++)
			{
			GetStFromSttb(hsttbGlsy, ist, rgch);

			cch = min(rgch[0], cchMaxSz - (int)ccpEop - 1);
			rgch[++cch] = chReturn;
			rgch[++cch] = chEop;
			if (!FInsertRgch(docUndo, cp, &rgch[1], cch, &chpName, &papName))
				return fFalse;
			cp += (CP) cch;

			PcaPoint( &caSprm, docUndo, cp );
			CaFromIhdd(doc, ist, &ca);
			if ((dcp = DcpCa(&ca)) != cp0 )
				{
				if (!FReplaceCps( &caSprm, &ca ))
					return fFalse;
				CachePara(ca.doc, ca.cpLim - ccpEop);
				}
			cp += dcp;
			if (dcp == cp0 || ca.cpLim != caPara.cpLim)
				{
				if (!FReplace(PcaPoint(&ca, docUndo, cp), fnSpec, (FC)fcSpecEop,
						(FC)ccpEop))
					return fFalse;
				cp += ccpEop;
				}
			caSprm.cpLim = cp;

			/* make all paras keep/nest, all but last keep follow */
			rgb[0] = sprmPFKeep;
			rgb[2] = sprmPFKeepFollow;
			rgb[1] = rgb[3] = 1;
			ApplyGrpprlCa(rgb, 4, &caSprm);
			caSprm.cpFirst = caSprm.cpLim - ccpEop;
			rgb[0] = sprmPFKeepFollow;
			rgb[1] = 0;
			ApplyGrpprlCa(rgb, 2, &caSprm);
			}
		}

	return (CpMacDoc(docUndo) > ccpEop);
}


/* %%Function:FFillUndoKeys %%Owner:davidbo */
FFillUndoKeys(docMother)
int docMother;
{
	char *pch;
	KMP **hkmp;
	int cch, docDot, iLoop;
	CP cp;
	struct CHP chp;
	char rgch[cchMaxSz];
	char rgchIns[cchMaxSz];
	struct PAP pap;
	BOOL fPrinted;

	fPrinted = fFalse;
	cp = cp0;
	SetUndoNil();
	CachePara(docUndo, cp0);
	StandardPap(&pap);
	StandardChp(&chp);
	chp.fBold = fTrue;
	for (iLoop = 0; iLoop < 2; iLoop++)
		{
		CopyCsSt((iLoop == 0 ? stDotKeys : stGlobalKeys), rgchIns);
		cch = rgchIns[0];

		if (iLoop == 0)
			{
			if ((docDot = DocDotMother(docMother)) == docNil)
				continue;
			hkmp = PdodDoc(docDot)->hkmpUser;

			GetDocSt(docDot, rgch, gdsoFullPath);
			rgch[0] = min(rgch[0], cchMaxSz - (int)ccpEop - 1 - cch);
			StStAppend(rgchIns, rgch);
			cch = rgchIns[0];
			}
		else
			{
			pap.dyaBefore = 10 * dyaPoint;
			hkmp = vhkmpUser;
			}

		if ((*hkmp)->ikmeMac == 0)
			continue;

		fPrinted = fTrue;

		rgchIns[++cch] = chReturn;
		rgchIns[++cch] = chEop;
		if (!FInsertRgch(docUndo, cp, &rgchIns[1], cch, &chp, &pap))
			return fFalse;
		cp += (CP) cch;
		if (!FInsKmpInUndo(hkmp, &cp, rgch))
			return fFalse;
		}

	return fPrinted;
}


/* %%Function:FInsKmpInUndo %%Owner:davidbo */
FInsKmpInUndo(hkmp, pcp, szBuf)
KMP **hkmp;
CP *pcp;
char szBuf[];
{
	int cch, ikme, ikmeMac;
	uns bsy;
	CP cp;
	char rgb[4];
	struct CA caSprm;
	struct CHP chp;
	struct PAP pap;
	char szBufBind[cchMaxSz];

	StandardPap(&pap);
	StandardChp(&chp);
	pap.itbdMac = 1;
	pap.rgdxaTab[0] = 2 * dxaInch;
	pap.rgtbd[0] = Tbd(jcLeft, tlcNone);

	cp = *pcp;
	ikmeMac = (*hkmp)->ikmeMac;
	for (ikme = 0; ikme < ikmeMac; ++ikme)
		{
		PcaPoint(&caSprm, docUndo, cp);
		pap.dxaLeft = dxaInch / 4;
		pap.dyaBefore = 10 * dyaPoint;

		GetNameFromBsy(szBufBind, bsy = (*hkmp)->rgkme[ikme].bcm);
		if (szBufBind[0] == 0)
			continue;

		FKcToSz((*hkmp)->rgkme[ikme].kc, szBuf);
		cch = CchSz(szBuf) - 1;
		szBuf[cch++] = '\t';
		if (!FInsertRgch(docUndo, cp, szBuf, cch, &chp, NULL))
			return fFalse;
		cp += (CP) cch;

		StToSzInPlace(szBufBind);
		cch = CchSz(szBufBind) - 1;
		szBufBind[cch++] = chReturn;
		szBufBind[cch++] = chEop;
		if (!FInsertRgch(docUndo, cp, szBufBind, cch, &chp, &pap))
			return fFalse;
		cp += (CP) cch;

		/* get menu help text */
		GetMenuHelpSz(bsy, szBuf);
		cch = CchSz(szBuf) - 1;
		szBuf[cch++] = chReturn;
		szBuf[cch++] = chEop;
		pap.dxaLeft = dxaInch / 2;
		pap.dyaBefore = 0;
		if (!FInsertRgch(docUndo, cp, szBuf, cch, &chp, &pap))
			return fFalse;
		cp += (CP) cch;

		/* make all paras keep/nest, all but last keep follow */
		caSprm.cpLim = cp;
		rgb[0] = sprmPFKeep;
		rgb[2] = sprmPFKeepFollow;
		rgb[1] = rgb[3] = 1;
		ApplyGrpprlCa(rgb, 4, &caSprm);
		caSprm.cpFirst = caSprm.cpLim - ccpEop;
		rgb[0] = sprmPFKeepFollow;
		rgb[1] = 0;
		ApplyGrpprlCa(rgb, 2, &caSprm);
		}

	*pcp = cp;
	return fTrue;
}


#define iNix 0
#define iSum 1
#define iStat 2

struct PRSD        /* Print Summary Descriptor */
	{
	char stStr[];
	int i;    /* If iNix, ignore iag */
	int iag;    /* If i is iSum, use hcabSum; if i is iStat, use hcabStat. */
};

csconst struct PRSD rgprsdDef[] =
	{
		{ StKey("Filename:\t", Filename),        iSum, Iag(CABDOCSUM, hszFNameSum) },
	
	{ StKey("Directory:\t", Directory),      iSum, Iag(CABDOCSUM, hszDirSum) },
	
	{ StKey("Template:\t", TemplateTab),     iStat, Iag(CABDOCSTAT, hszDOT) },
	
	{ StKey("Title:\t", Title),              iSum, Iag(CABDOCSUM, hszTitle) },
	
	{ StKey("Author:\t", Author),            iSum, Iag(CABDOCSUM, hszAuthor) },
	
	{ StKey("Subject:\t", Subject),          iSum, Iag(CABDOCSUM, hszSubject) },
	
	{ StKey("Keywords:\t", Keywords),        iSum, Iag(CABDOCSUM, hszKeyWords) },
	
	{ StKey("Comments:\t", Comments),        iSum, Iag(CABDOCSUM, hszComments) },
	
	{ StKey("Create Date:\t", Create),       iStat, Iag(CABDOCSTAT, hszCDate) },
	
	{ StKey("Revision Number:\t", Revision), iStat, Iag(CABDOCSTAT, hszRevNum) },
	
	{ StKey("Last Saved Date:\t", SavedDate),iStat, Iag(CABDOCSTAT, hszRDate) },
	
	{ StKey("Last Saved By:\t", SavedBy),    iStat, Iag(CABDOCSTAT, hszLRevBy) },
	
	{ StKey("Total Editing Time:\t", Time),  iStat, Iag(CABDOCSTAT, hszEdMin) },
	
	{ StKey("Last Printed:\t", LastPrinted), iStat, Iag(CABDOCSTAT, hszLPDate) },
	
	{ StKey("As of Last Complete Printing", AsOfPrint), iNix,    0 },
	
	{ StKey("\tNumber of Pages:\t", CPages), iStat, Iag(CABDOCSTAT, hszCPg) },
	
	{ StKey("\tNumber of Words:\t", CWords), iStat, Iag(CABDOCSTAT, hszCWords) },
	
	{ StKey("\tNumber of Characters:\t", CCh),iStat, Iag(CABDOCSTAT, hszCch) }
	};	


/* Change this number if the size of rgprsdDef changes! */
#define iprsdMac 18

/* %%Function:FFillUndoSummaryInfo %%Owner:davidbo */
FFillUndoSummaryInfo(docMother)
int docMother;
{
	HCABDOCSTAT hcabdocstat;
	HCABDOCSUM hcabdocsum;
	int doc, cch, cchStatic, iprsd, fRet;
	int dxa = dxaInch >> 1;
	char ch;
	char *pcabSum, *pcabStat;
	char **hcab, **hcabSum, **hcabStat;
	CP dcp, cp;
	struct CA caT;
	struct CA ca, caSprm;
	struct PAP pap;
	char rgb[7];
	char rgchStatic[cchMaxSz];
	char rgch[cchMaxSz];
	NDD  ndd;

	/* stuff relevant info into CAB structures.  Why use CABs?  So we
		* can share code
		*/
	if ((hcabdocstat = HcabAlloc(cabiCABDOCSTAT)) == hNil)
		return fFalse;

	if ((hcabdocsum = HcabAlloc(cabiCABDOCSUM)) == hNil)
		{
		FreeCab(hcabdocstat);
		return fFalse;
		}

	ndd.doc = docMother;
	GetDocSt(ndd.doc, rgch, gdsoFullPath);
	ndd.stFName = rgch;
	ndd.hsttbAssoc = HsttbAssocEnsure(ndd.doc);
	if (ndd.hsttbAssoc == hNil)
		{
		fRet = fFalse;
		goto LRet;
		}
	ndd.pdop = NULL; /* Protect ourselves from heap movement. */
	ndd.fAppendApprox = fTrue;
	ndd.fUseSt = fFalse;

	if (!FGetDocSumStatHcab(hcabdocsum, hcabdocstat, &ndd, tmcNull))
		{
		fRet = fFalse;
		goto LRet;
		}

	SetUndoNil();
	CachePara(docUndo, cp0);
	StandardPap(&pap);
	pap.dxaLeft = dxaInch/4;
	pap.dxaLeft1 = -dxaInch/4;
	pap.fKeep = fTrue;
	pap.fKeepFollow = fTrue;
	pap.itbdMac = 2;
	pap.rgdxaTab[0] = 2 * dxaInch / 5;
	pap.rgdxaTab[1] = 2 * dxaInch;
	pap.rgtbd[0] = pap.rgtbd[1] = Tbd(jcLeft, tlcNone);

	cp = cp0;
	for (iprsd = 0; iprsd < iprsdMac; iprsd++)
		{
		int i;

		CopyCsSt(rgprsdDef[iprsd].stStr, rgchStatic);
		cchStatic = *rgchStatic;

		i = rgprsdDef[iprsd].i;
		if (i != iNix)
			{
			hcab = (i == iSum ? hcabdocsum : hcabdocstat);
			GetCabSt(hcab, rgch, cchMaxSz, rgprsdDef[iprsd].iag);
			rgch[0] = min(rgch[0], cchMaxSz - (int)ccpEop - 1 - cchStatic);
			StStAppend(rgchStatic, rgch);
			}

		cch = rgchStatic[0];
		rgchStatic[++cch] = chReturn;
		rgchStatic[++cch] = chEop;
		if (!FInsertRgch(docUndo, cp, &rgchStatic[1], cch, &vchpStc, &pap))
			{
			fRet = fFalse;
			goto LRet;
			}
		cp += (CP) cch;
		}

	/* make all paras keep/nest, all but last keep follow */
	caSprm.doc = docUndo;
	caSprm.cpFirst = cp0;
	caSprm.cpLim = cp;
	rgb[0] = sprmPFKeep;
	rgb[2] = sprmPFKeepFollow;
	rgb[4] = sprmPNest;
	rgb[1] = rgb[3] = 1;
	bltb(&dxa, &rgb[5], sizeof(int));
	ApplyGrpprlCa(rgb, 7, &caSprm);
	caSprm.cpFirst = caSprm.cpLim - ccpEop;
	rgb[0] = sprmPFKeepFollow;
	rgb[1] = 0;
	ApplyGrpprlCa(rgb, 2, &caSprm);
	fRet = CpMacDoc(docUndo) > ccpEop;

LRet:
	FreeCab(hcabdocsum);
	FreeCab(hcabdocstat);

	return (fRet);
}


/* %%Function:FPrintSummaryInfo %%Owner:davidbo */
FPrintSummaryInfo(doc, cpFirst, cpLim)
int doc;
CP cpFirst, cpLim;  /* only used if printing selection from main doc */
{
	int fRet = fFalse;
	int pgnFirst = vpgnFirst;
	int pgnLast = vpgnLast;
	int sectFirst = vsectFirst;
	int sectLast = sectLast;
	int docMother = DocMother(doc);
	struct CA caT;
	struct DOD *pdod;
	struct DOP dop;
	struct PPR **hppr;

	SetUpPrintPrompt(docMother, istSummary, 0, &hppr, fFalse);
	if (!FFillUndoSummaryInfo(docMother))
		goto LRet;

	vpgnFirst = vsectFirst = 1;
	vpgnLast = vsectLast = 0x7FFF;

	/* set dop of derivative doc to docMother's dop */
	pdod = PdodDoc(docUndo);
	dop = pdod->dop;
	pdod->dop = PdodDoc(docMother)->dop;
	pdod->dop.grpfIhdt = 0; /* no special footnote separators */
	pdod->fMotherStsh = fTrue;      /* use docMother's stsh */
	pdod->doc = docMother;

	if (!FPrintDoc(docUndo, hppr))
		goto LRet;

	pdod = PdodDoc(docUndo);
	pdod->dop = dop;
	pdod->fMotherStsh = fFalse;

	/* if printing a selection more than once, put it back into Undo doc */
	if (vprsu.prng == prngSel && vprsu.cCopies > 1)
		SetWholeDoc(docUndo, PcaSet( &caT, doc, cpFirst, cpLim));

	vpgnFirst = pgnFirst;
	vpgnLast = pgnLast;
	vsectFirst = sectFirst;
	vsectLast = sectLast;
	fRet = fTrue;
LRet:
	return fRet;
}


/* %%Function:FFillUndoAtn %%Owner:davidbo */
FFillUndoAtn(docMother, cpFirst, cpLim)
int docMother;
CP cpFirst, cpLim;
{
	int docAtn = PdodDoc(docMother)->docAtn;
	struct CA caAtn, caT;
	CP cpAtnFirst, cpAtnLim, cp;

	Assert (docAtn != docNil);
	Assert (cHpFreeze == 0);
	SetUndoNil();
	PcaSet( &caAtn, docAtn, cp0, CpMacDocEdit(docAtn));

	if (vprsu.prng == prngSel)
		{
		/* Only print annotations with references within desired range */
		caAtn.cpFirst = CpFirstRef(docMother, cpFirst, &cp, edcDrpAtn);
		caAtn.cpLim = CpFirstRef(docMother, cpLim, &cp, edcDrpAtn);
		}
	else  if (vprsu.prng == prngPages)
		{
		int ipgd = vpgnFirst - 1;
		struct PLCPGD **hplcpgd;

		/*  If printing Annotations only, ensure page table is current */
		if (vprsu.istPrSrc == istAnnotation)
			{
			struct RPL rpl;
			struct PPR **hppr;

			rpl.cp = CpMacDocEdit(docMother);
			/* do one extra page so we know what cpMax of vpgnLast */
			rpl.pgn = vpgnLast + 1;
			rpl.ipgd = pgnMax;
			rpl.ised = pgnMax;
			if (!FRepaginateDoc(docMother, &rpl, patAbort))
				return fFalse;

			SetUndoNil();
			SetUpPrintPrompt(docMother, vprsu.istPrSrc, 0, &hppr, fFalse);
			}

		hplcpgd = PdodDoc(docMother)->hplcpgd;
		Assert(hplcpgd != hNil);
		if (ipgd < 0 || ipgd >= IMacPlc(hplcpgd))
			ipgd = 0;
		cp = CpFromIpgd(docMother, vpri.wwInit, ipgd); /* HM */
		caAtn.cpFirst = CpFirstRef(docMother, cp, &cp, edcDrpAtn);

		/*
		*  CpFromIpgd returns first cp on page ipgd.  We want last
		*  cp on page, therefore, use ipgd+1 (vpgnLast!).  If on
		*  last page, use cpMacDoc docAtn for cpAtnLim.
		*/
		if (vpgnLast > 1 && vpgnLast < (*PdodDoc(docMother)->hplcpgd)->iMac)
			{
			cp = CpFromIpgd(docMother, vpri.wwInit, vpgnLast); /* HM */
			caAtn.cpLim = CpFirstRef(docMother, cp, &cp, edcDrpAtn);
			}
		}
	vpri.cpAdjust = caAtn.cpFirst;
	SetWholeDoc(docUndo, &caAtn);
	vpri.fPrintingAtn = fTrue;
	return (CpMacDoc(docUndo) > ccpEop);
}


/* %%Function:FPrintAtn %%Owner:davidbo */
FPrintAtn(doc, cpFirst, cpLim)
int doc;
CP cpFirst, cpLim;   /* only used when printing a selection from main doc */
{
	int fRet = fFalse;
	int pgnFirst = vpgnFirst;
	int pgnLast = vpgnLast;
	int sectFirst = vsectFirst;
	int sectLast = sectLast;
	int docMother = DocMother(doc);
	/* if printing selection from main doc, save vpri.cpAdjust...printing
		annotations uses it too ! */
	CP cpAdjustSave = vpri.cpAdjust;
	struct CA caT;
	struct DOD *pdod;
	struct DOP dop;
	struct PPR **hppr;

	SetUpPrintPrompt(docMother, istAnnotation, 0, &hppr, fFalse);
	if (!FFillUndoAtn(docMother, cpFirst, cpLim))
		goto LRet;

	vpgnFirst = vsectFirst = 1;
	vpgnLast = vsectLast = 0x7FFF;

	/* set dop of derivative doc to docMother's dop */
	pdod = PdodDoc(docUndo);
	dop = pdod->dop;
	pdod->dop = PdodDoc(docMother)->dop;
	pdod->dop.grpfIhdt = 0; /* no special footnote separators */
	pdod->fMotherStsh = fTrue;      /* use docMother's stsh */
	pdod->doc = docMother;

	if (!FPrintDoc(docUndo, hppr))
		goto LRet;

	pdod = PdodDoc(docUndo);
	pdod->dop = dop;
	pdod->fMotherStsh = fFalse;
	pdod->doc = docNil;

	/* if printing a selection more than once, put it back into Undo doc */
	if (vprsu.prng == prngSel && vprsu.cCopies > 1)
		{
		SetWholeDoc( docUndo, PcaSet( &caT, doc, cpFirst, cpLim ));
		vpri.cpAdjust = cpAdjustSave;
		}

	vpgnFirst = pgnFirst;
	vpgnLast = pgnLast;
	vsectFirst = sectFirst;
	vsectLast = sectLast;
	fRet = fTrue;
LRet:
	vpri.fPrintingAtn = fFalse;
	return fRet;
}



/* *****  Change Printer/Printer Setup dialogs, etc.  ***** */

/* Change Printer Info */
typedef struct _cpi
	{
	char ** hszPrinter;
	char ** hszPort;
	char ** hszDriver;
	struct STTB **hsttb;
} CPI;

/*
*  Fill hsttb with printer list...list box is filled using this hsttb.
*  Returns fTrue if succeeds, false if no printers or runs out of memory.
*  If OOM, frees hsttb.
*/
/* %%Function:FFillChgPrLb %%Owner:davidbo */
FFillChgPrLb(szDevSpec, szListEntry, phsttb)
char szDevSpec[], szListEntry[];
struct STTB ***phsttb;
{
	int cPort, iPort, fRet = fFalse;
	char chNull = '\0', *pchPort, *pchDriver, *pchPrinters;
	char szPrinters[ichMaxProfileSz];

	/* Get a string that holds all of the printer names. */
	GetProfileString((LPSTR)SzShared("devices"), (LPSTR)NULL, (LPSTR)&chNull,
			(LPSTR)szPrinters, ichMaxProfileSz);

	if (szPrinters[0] == 0)
		return fFalse;

	/* There must be two nulls at the end of the list. */
	szPrinters[ichMaxProfileSz - 1] = szPrinters[ichMaxProfileSz - 2] = '\0';

	pchPrinters = &szPrinters[0];

	/* Parse out the names of the printers. */
	while (*pchPrinters != '\0')
		{
		/* Get the corresponding printer driver and port. */
		GetProfileString((LPSTR) SzShared("devices"), 
				(LPSTR) pchPrinters, (LPSTR) &chNull, 
				(LPSTR) szDevSpec, ichMaxProfileSz);
		szDevSpec[ichMaxProfileSz - 1] = '\0';

		/* If there is no driver for this printer, then it 
			cannot be added to the list. */
		if (szDevSpec[0] != '\0')
			{
			fRet = fTrue;

			/* Parse the ports and the driver. */
			cPort = ParseDeviceSz(szDevSpec, &pchPort, &pchDriver);
			for (iPort = 0; iPort < cPort; iPort++)
				{
				/* Contruct the list box entry. */
				BuildPrSetupSz(szListEntry, pchPrinters, pchPort);

				/* Put the string in hsttb */
				SzToStInPlace(szListEntry);
				if (IbstAddStToSttb(*phsttb, szListEntry) == ibstNil)
					{
					FreePhsttb(phsttb);
					return fFalse;
					}

				/* Bump the pointer to the next port in the list. */
				pchPort += CchSz(pchPort);
				}
			}

		/* Skip to the next printer in the list. */
		while (*pchPrinters++)
			;
		}

	return fRet;
}


/* %%Function:CmdChangePrinter %%Owner:davidbo */
CMD CmdChangePrinter(pcmb)
CMB * pcmb;
{
	CABCHGPR * pcabChgPr;
	CPI cpi;
	char szDevSpec[ichMaxProfileSz];
	char szListEntry[ichMaxProfileSz];

	SetBytes(&cpi, 0, sizeof(CPI));

	if (pcmb->fDefaults)
		{
		((CABCHGPR *) *pcmb->hcab)->hszLb = ppvZero;
		if (vpri.hszPrinter != NULL && vpri.hszPrPort != NULL)
			{
			BuildPrSetupSz(szListEntry, 
					&(**vpri.hszPrinter)[0], 
					&(**vpri.hszPrPort)[0]);

			FSetCabSz(pcmb->hcab, szListEntry, Iag(CABCHGPR, hszLb));
			if (vmerr.fMemFail)
				return cmdNoMemory;
			}
		}

	if (pcmb->fDialog)
		{
		TMC tmc;
		char dlt [sizeof (dltChgPr)];

		pcmb->pv = &cpi; /* so dialog proc can get at this */

		if ((cpi.hsttb = HsttbInit(ichMaxProfileSz/2, fFalse)) == hNil)
			return cmdNoMemory;

		/* if no printers, no need to bring up dialog */
		if (!FFillChgPrLb(szDevSpec, szListEntry, &cpi.hsttb))
			{
#ifdef NOTUSED
			/* cmdNoMemory and cmdError are the same value.  Old code: */
			CMD cmd = (cpi.hsttb != hNil) ? cmdError : cmdNoMemory;
			if (cmd == cmdError)
				ErrorEid(eidNoPrinters, "");
			return cmd;
#endif /* NOTUSED */
			if (cpi.hsttb != hNil)
				ErrorEid(eidNoPrinters, "");
			return cmdError;
			}

		BltDlt(dltChgPr, dlt);
		tmc = TmcOurDoDlg(dlt, pcmb);
		FreePhsttb(&cpi.hsttb);
		switch (tmc)
			{
#ifdef DEBUG
		default:
			Assert(fFalse);
			break;
#endif
		case tmcError:
			return cmdNoMemory;

		case tmcCancel:
			return cmdCancelled;

		case tmcOK:
			break;
			}
		}


	if (pcmb->fAction)
		{
		CPI * pcpi;
		char *pchPort, *pchDriver;
		char (**hszPrinter) [];
		char (**hszPort) [];
		char (**hszDriver) [];
		char szListEntry [ichMaxProfileSz];

		if (pcmb->tmc == tmcChgPrSetup)
			{
			char szPrinter [ichMaxProfileSz];

			GetCabSz(pcmb->hcab, szPrinter, sizeof (szPrinter),
					Iag(CABCHGPR, hszLb));
			/* may happen if OOM during dialog so no printer names */
			if (szPrinter[0] == 0)
				goto LAbort;
			SetupPrinter(szPrinter);
			}

		StartLongOp();

		hszPrinter = hszPort = hszDriver = hNil;

		GetCabSz(pcmb->hcab, szListEntry, sizeof(szListEntry), Iag(CABCHGPR, hszLb));
		/* may happen if OOM during dialog so no printer names */
		if (szListEntry[0] == 0)
			goto LAbort;
		GetPrinterFromListEntry(szListEntry, &pchPort, szDevSpec, &pchDriver);

		if (FNoHeap(hszPrinter = HszCreate(szListEntry)))
			{
LAbort:
			EndLongOp(fFalse);
			FreePh(&hszPrinter);
			FreePh(&hszPort);
			FreePh(&hszDriver);
			pcpi = &cpi;
			pcpi->hszPrinter = pcpi->hszPort = pcpi->hszDriver = hNil;
			return cmdError;
			}

		if (FNoHeap(hszDriver = HszCreate(pchDriver)))
			goto LAbort;

		if (FNoHeap(hszPort = HszCreate(pchPort)))
			goto LAbort;

		pcabChgPr = *pcmb->hcab;
		pcpi = &cpi;
		pcpi->hszPrinter = hszPrinter;
		pcpi->hszPort = hszPort;
		pcpi->hszDriver = hszDriver;

		if (!FCheckPrinter(hszPrinter, hszPort, hszDriver))
			{
			ErrorEid(eidBadPrinter, "");
			goto LAbort;
			}

		EndLongOp(fFalse);

		ChgPr(cpi.hszPrinter, cpi.hszPort, cpi.hszDriver);
		}

	return cmdOK;
}


/* %%Function:ElPrinterSetup %%Owner:davidbo */
ElPrinterSetup(pstPrinter, pstPort, pstDriver)
char ** pstPrinter, ** pstPort, ** pstDriver;
{
	CHAR **hsz1, **hsz2, **hsz3;

	StToSzInPlace(*pstPrinter);
	StToSzInPlace(*pstPort);
	StToSzInPlace(*pstDriver);

	if ((hsz1 = HCopyHeapBlock(pstPrinter, strcPLAIN)) == hNil || 
			(hsz2 = HCopyHeapBlock(pstPort, strcPLAIN)) == hNil || 
			(hsz3 = HCopyHeapBlock(pstDriver, strcPLAIN)) == hNil)
		RtError(rerrOutOfMemory);
	ChgPr(hsz1, hsz2, hsz3);
}


/* %%Function:SetupPrinter %%Owner:davidbo */
SetupPrinter(szPrinter)
char * szPrinter;
{
	char * szPort, * szDriver;
	char szDevSpec [ichMaxProfileSz];

	Assert (*szPrinter != 0); /* callers should prevent this */
	GetPrinterFromListEntry(szPrinter, &szPort, szDevSpec, &szDriver);
	RunDriverDlg(szPrinter, szPort, szDriver);
}


/* %%Function:FDlgChgPr %%Owner:davidbo */
int FDlgChgPr(dlm, tmc, wNew, wOld, wParam)
DLM dlm;
TMC tmc;
WORD wOld, wNew, wParam;
{
	CMB * pcmb;
	int fKill = fFalse;
	char *pch, *pchPrinters, *pchPort, *pchDriver;
	char (**hszPrinter) [];
	char (**hszPort) [];
	char (**hszDriver) [];
	HANDLE hDriver;
	CABCHGPR *pcabChgPr;
	HCAB hcab;
	CPI * pcpi;
	char szDevSpec[ichMaxProfileSz];
	char szListEntry [ichMaxProfileSz];

	pcmb = PcmbDlgCur();
	hszPrinter = hszPort = hszDriver = hNil;

	switch (dlm)
		{
	case dlmInit:
		/* Select the current printer. */
		if (vpri.hszPrinter != NULL && vpri.hszPrPort != NULL)
			{
			BuildPrSetupSz(szListEntry, 
					&(**vpri.hszPrinter)[0], 
					&(**vpri.hszPrPort)[0]);

			SetTmcText(tmcChgPrLb, szListEntry);
			}

		if (ValGetTmc(tmcChgPrLb) == uNinchList)
			{
			EnableTmc(tmcOK, fFalse);
			EnableTmc(tmcChgPrSetup, fFalse);
			}

		if (fElActive)
			EnableTmc(tmcChgPrSetup, fFalse);

		break;

	case dlmClick:
		switch (tmc)
			{
			HDLG hdlg;
			BOOL fEnable;

		case tmcChgPrLb:
			fEnable = ValGetTmc(tmcChgPrLb) != uNinchList;
			EnableTmc(tmcOK, fEnable);
			if (!fElActive)
				EnableTmc(tmcChgPrSetup, fEnable);
			break;

		case tmcChgPrSetup:
			GetTmcText(tmcChgPrLb, szListEntry, ichMaxProfileSz);
			if (*szListEntry == 0)
				goto LAbortErr;
/*			SetFocusTmc(tmcChgPrLb); REDUNDANT */
			hdlg = HdlgGetCur();
			SetupPrinter(szListEntry);
			/* We must restore sdm's idea of current dialog
				because the ribbon may have been made current
				by the WININICHANGE message we might have 
				recieved. */
			HdlgSetCurDlg(hdlg);
			SetFocusTmc(tmcChgPrLb);
			break;
			}
		break;

	case dlmIdle:
		/* returns true, so will call FDoSdmIdle automatically */
		if (wNew == 0)
			if (ValGetTmc(tmcChgPrLb) == uNinchList)
				{
				EnableTmc(tmcOK, fFalse);
				EnableTmc(tmcChgPrSetup, fFalse);
				}
		break;

	case dlmTerm:
		if (tmc == tmcCancel)
			break;

/* REVIEW bradch: can't do this at dlmTerm for some reason! (dsb)

No wonder we're almost a megabyte big, what with all this code here AND
back at the cmd level!  How many other places do we do this foolishness?

		if (CmdDlgAction(CmdChangePrinter, tmc) != cmdOK)
			return fFalse;
*/

		/* Let the user know that this may take a while. */
		StartLongOp();

		GetTmcText(tmcChgPrLb, szListEntry, ichMaxProfileSz);
		if (szListEntry[0] == 0)
			goto LAbortErr;
		GetPrinterFromListEntry(szListEntry, &pchPort, szDevSpec, &pchDriver);

		if ((hcab = HcabFromDlg(fFalse /* fNew */ )) == hcabNotFilled)
			goto LAbort; /* stay in dialog */

		/* Update the heap strings describing the printer. */
		if (FNoHeap(hszPrinter = HszCreate(szListEntry)) ||
				FNoHeap(hszDriver = HszCreate(pchDriver)) ||
				FNoHeap(hszPort = HszCreate(pchPort)) ||
				hcab == hcabNull)
			{
			fKill = fTrue;
LAbort:
			EndLongOp(fFalse);
			FreePh(&hszPrinter);
			FreePh(&hszPort);
			FreePh(&hszDriver);

			pcpi = pcmb->pv;
			pcpi->hszPrinter = pcpi->hszPort = 
					pcpi->hszDriver = hNil;

			return fKill;   /* kills dialog if true */
			}

		pcabChgPr = *hcab;
		pcpi = pcmb->pv;
		pcpi->hszPrinter = hszPrinter;
		pcpi->hszPort = hszPort;
		pcpi->hszDriver = hszDriver;

		if (!FCheckPrinter(hszPrinter, hszPort, hszDriver))
			{
LAbortErr:
			ErrorEid(eidBadPrinter, "");
			goto LAbort;
			}

		EndLongOp(fFalse);
		break;
		}

	return fTrue;
}


/* %%Function:WListChgPr %%Owner:davidbo */
EXPORT WORD WListChgPr(tmm, sz, isz, filler, tmc, wParam)
TMM tmm;
char * sz;
int isz;
WORD filler;
TMC tmc;
WORD wParam;
{
	struct STTB **hsttb;

	hsttb = ((CPI *)PcmbDlgCur()->pv)->hsttb;
	switch (tmm)
		{
	case tmmCount:
		return (hsttb != hNil ? -1 : 0);

	case tmmText:
		if (hsttb == hNil || isz >= (*hsttb)->ibstMac)
			return fFalse;
		GetSzFromSttb(hsttb, isz, sz);
		return fTrue;
		}
	return 0;
}


/*
*  
*  szListEntry contains printer and port name (right out of LB).  Zero
*  terminate printer name, point to port name, and get driver name.
*/
/* %%Function:GetPrinterFromListEntry %%Owner:davidbo */
GetPrinterFromListEntry(szListEntry, ppchPort, szDevSpec, ppchDriver)
char szListEntry[], szDevSpec[];
char **ppchPort, **ppchDriver;
{
	char *pch, *pchPort, *pchDriver;
	char chNull = '\0';
	char stBuf[cchBufMax];

	/* Get the printer's name, its port, and its driver. */
	if (szListEntry[0] == 0)
		{
		*ppchDriver = NULL;
		*ppchPort = NULL;
		ReportSz("Warning - no printer name found - set values to null");
		return;
		}
	/* Parse the port name out of the list entry. */
	pchPort = &szListEntry[0] + CchSz(szListEntry) - 1;
	while (*(pchPort - 1) != ' ')
		{
		pchPort--;
		}

	/* zero terminate the printer name in the list entry. */
	pch = &szListEntry[0];
	CopyCsSt(stOn, stBuf);
	stBuf[stBuf[0]+1] = 0;
	for ( ; ; )
		{
		if ((pch = index(pch, ' ')) != 0 && FEqNcRgch(pch, &stBuf[1], stBuf[0]))
			{
			*pch = '\0';
			break;
			}
		pch++;
		}

	/* Get the driver name for this printer. */
	GetProfileString((LPSTR)SzShared("devices"), (LPSTR)szListEntry,
			(LPSTR)&chNull, (LPSTR)szDevSpec, ichMaxProfileSz);
	ParseDeviceSz(szDevSpec, &pch, &pchDriver);

	*ppchDriver = pchDriver;
	*ppchPort = pchPort;
}


BuildPrSetupSz(szPrSetup, szPrinter, szPort)
char *szPrSetup;
char *szPrinter;
char *szPort;
{
	/* This routine pieces together the string for the Change Printers list box.
	szPrinter is the name of the printer, and szPort, the name of the port.  It
	is assumed that the setup string, szPrSetup, is large enough to hold the
	string created by this routine. */

	char *pch, stBuf[cchBufMax];

	pch = bltbyte(szPrinter, szPrSetup, CchSz(szPrinter) - 1);
	CopyCsSt(stOn, stBuf);
	stBuf[stBuf[0]+1] = 0;
	pch = bltbyte(&stBuf[1], pch, stBuf[0]);

	/* If the port name is not "None", then raise the port name to all capitals.
	*/
	bltbyte(szPort, pch, CchSz(szPort));
	if (FNeNcSz(pch, szNone))
		{
		while (*pch != '\0')
			{
			*pch++ = ChUpper(*pch);
			}
		}
}


/*
*  move printer info to vpri structure.
*/
/* %%Function:ChgPr %%Owner:davidbo */
ChgPr(hszPrinter, hszPort, hszDriver)
char (**hszPrinter)[];
char (**hszPort)[];
char (**hszDriver)[];
{
	char *pch;
	char szWinDev[ichMaxProfileSz];

	/* if not in macro, test was done in dialog msgTerm processing */
	if (fElActive)
		{
		if (!FCheckPrinter(hszPrinter, hszPort, hszDriver))
			{
			ErrorEid(eidBadPrinter, "");
			goto LCleanUp;
			}
		}
	else  if (hszPrinter == hNil || hszPort == hNil || hszDriver == hNil)
		{
LCleanUp:
		FreePh(&hszPrinter);
		FreePh(&hszPort);
		FreePh(&hszDriver);
		return;
		}

	FreeH(vpri.hszPrinter);
	FreeH(vpri.hszPrPort);
	FreeH(vpri.hszPrDriver);
	vpri.hszPrinter = hszPrinter;
	vpri.hszPrPort = hszPort;
	vpri.hszPrDriver = hszDriver;
	/* build Windows device entry from printer, port, and file */
	/* guaranteed not to run out of room since FCheckPrinter didn't fail */
	pch = szWinDev;
	pch += CchCopySz(*hszPrinter, pch);
	*pch++ = ',';
	pch += CchCopySz(*hszDriver, pch);
	*pch++ = ',';
	CchCopySz(*hszPort, pch);

	/* change Windows' default printer */
	WriteProfileString((LPSTR) SzFrame("windows"), (LPSTR) SzFrame("Device"),
			(LPSTR) szWinDev);
	/* notify the world of said change */
	SendMessage(0xffff, WM_WININICHANGE, 0, (LPSTR) SzShared("windows"));
}


/*
*  Returns fTrue if hszPrinter, hszDriver, and hszPort form a valid
*  combination, i.e. said combination exists in win.ini.  Otherwise,
*  returns fFalse.
*/
/* %%Function:FCheckPrinter %%Owner:davidbo */
FCheckPrinter(hszPrinter, hszPort, hszDriver)
char (**hszPrinter)[];
char (**hszPort)[];
char (**hszDriver)[];
{
	int cch, iPort, cPort;
	char chNull = '\0';
	char *pchPort, *pchDriver;
	char szDevSpec[ichMaxProfileSz];

	/*
	*  Ensure printer, port, and driver names will fit in Windows default
	*  device entry.
	*/
	cch = CchSz(*hszPrinter) + CchSz(*hszPort) + CchSz(*hszDriver);
	if (cch > ichMaxProfileSz)
		return fFalse;

	/* Get the driver name for this printer. */
	GetProfileString((LPSTR)SzShared("devices"), (LPSTR)*hszPrinter,
			(LPSTR)&chNull, (LPSTR)szDevSpec, ichMaxProfileSz);
	if (szDevSpec[0] == chNull)
		return fFalse;
	cPort = ParseDeviceSz(szDevSpec, &pchPort, &pchDriver);

	if (FNeNcSz(pchDriver, *hszDriver))
		return fFalse;

	for (iPort = 0; iPort < cPort; iPort++)
		{
		if (FEqNcSz(pchPort, *hszPort))
			return fTrue;
		pchPort += CchSz(pchPort);
		}
	return fFalse;
}


/* %%Function:RunDriverDlg %%Owner:davidbo */
RunDriverDlg(pchPrinter, pchPort, pchDriver)
char *pchPrinter, *pchPort, *pchDriver;
{
	char *pch;
	HWND hwndDlg = (HWND)NULL;
	HANDLE hDriver;
	FARPROC lpfnDevMode;
	char szDriver[ichMaxProfileSz];

	/* Get the name of the driver, complete with extension. */
	pch = bltbyte(pchDriver, szDriver, CchSz(pchDriver));
	bltbyte(SzFrameKey(".DRV",DRV), pch - 1, 5);

	/* The driver is not resident; attempt to load it. */
	if ((hDriver = LoadLibrary((LPSTR)szDriver)) <= 32)
		{
#ifdef BZ
		{ 
		char szBuf[256];
		CchPchIntToPpch( SzFrame("Load lib failure value: "),hDriver,&szBuf);
		IdOurMessageBox(szBuf, 
				SzFrame("RunDriverDlg"), 
				MB_OK | MB_ICONEXCLAMATION);
		}
#endif /* BZ */

		if (hDriver != 2)
			{
			/* If hDriver is 2, then the user has cancelled a dialog
			box; there's no need to put up another. */
			ErrorEid(eidBadPrinter, "");
			}
		return;
		}


	/* Find the driver's DeviceMode() entry. */
	if ((lpfnDevMode = GetProcAddress(hDriver, 
			MAKEINTRESOURCE(idoDeviceMode))) == NULL)
		{
		ErrorEid(eidBadPrinter, "");
		FreeLibrary(hDriver);
		return;
		}


	/* Actual call to the device modes setup. */
	if (HdlgGetCur() != hdlgNull)
		hwndDlg = HwndFromDlg(HdlgGetCur()); /* make current dlg parent of new one */
	else if (fElActive)
		hwndDlg = vhwndApp;

	if (hwndDlg != (HWND)NULL)
		(*lpfnDevMode)(hwndDlg, hDriver, (LPSTR)pchPrinter, (LPSTR)pchPort);
	else
		Beep();

	FreeLibrary(hDriver);
}


/* ***** Miscellaneous PostScript Generating stuff ***** */


/*************************/
/* E m i t  P S */
/* %%Function:EmitPs %%Owner:davidbo */
EmitPs(idPrintPs, cp)
int idPrintPs;
CP cp;
{
	/* emit the PostScript variables which Word provides to embedded
		PostScript programs */
	int xlLeft, xlRight;
	int xp, xpT, yp, dxp, dxpLeft, dyp, ypPage, stcp;
	struct DOD *pdod;
	struct DTTM dttm;
	struct RC rc;
	int rgw[2];
	CP cpPara, cpLimPara, cpLine;
	struct LR lr;
	char rgch[cchMaxPic];
	char szPic[cchMaxPic];
	struct DOP dop;
	struct TCX tcx;

	/* calculate drawing rectangle size and position */
	lr = *((struct LR *)vpri.plr);
	dop = PdodMother(lr.doc)->dop;
	CachePara(lr.doc, lr.cp);
	Assert(cp >= lr.cp);
	Assert(cp < lr.cpLim);
	/* Note: all postscript vars in points (1/72 inch) */
	ypPage = dop.yaPage / 20;
	switch (idPrintPs)
		{
	case idPrintPsPage:
LPage:
		xp = yp = 0;
		dxp = dop.xaPage / 20;
		dyp = ypPage;
		break;

	case idPrintPsPara:
		GetCpFirstCpLimDisplayPara(wwLayout, lr.doc, cp, &cpPara, &cpLimPara);
		CachePara(lr.doc, cpPara);
		dxpLeft = NMultDiv(vpapFetch.dxaLeft + min(0, vpapFetch.dxaLeft1), vfli.dxuInch, dxaInch);
		if (vpapFetch.fInTable)
			{
			dxp = DxpDypToPara(lr.doc, cp, &dyp);
			xp = lr.xl + dxp - dypLeftRightSpace + dxpLeft;
			yp = lr.yl + dyp;
			CachePara(lr.doc, cpLine = cpPara);
			dxp = vtcc.xpDrRight - vtcc.xpDrLeft + 
					dypLeftRightSpace + dypLeftRightSpace -
					dxpLeft - NMultDiv(vpapFetch.dxaRight, vfli.dxuInch, dxaInch);
			cpLimPara = caPara.cpLim;
			Mac(vfli.fStopAtPara =) vfli.fLayout = fTrue;
			for (dyp = 0; cpLine < cpLimPara; cpLine = vfli.cpMac, dyp += vfli.dypLine)
				{
				/* dxa 0 will be replaced by cell width */
				FormatLineDxa(wwLayout, lr.doc, cpLine, 0);
				}
			Mac(vfli.fStopAtPara = )vfli.fLayout = fFalse;
			}
		else
			{
			xp = lr.xl - dypLeftRightSpace + dxpLeft;
			dxp = lr.dxl + dypLeftRightSpace + dypLeftRightSpace -
					dxpLeft - NMultDiv(vpapFetch.dxaRight, vfli.dxuInch, dxaInch);
			yp = lr.yl;
			dyp = lr.dyl;
			}
		xp = NMultDiv(xp, 72, vfli.dxuInch);
		yp = NMultDiv(yp, 72, vfli.dyuInch);
		dxp = NMultDiv(dxp, 72, vfli.dxuInch);
		dyp = NMultDiv(dyp, 72, vfli.dyuInch);
		break;

	case idPrintPsPic:
		if (!FGetPsPicRc(lr.doc, cp, &lr, &rc))
			{
			idPrintPs = idPrintPsPage;
			goto LPage;
			}
		xp = NMultDiv(rc.xpLeft, 72, vfli.dxuInch);
		yp = NMultDiv(rc.ypTop, 72, vfli.dyuInch);
		dxp = NMultDiv(rc.xpRight - rc.xpLeft, 72, vfli.dxuInch);
		dyp = NMultDiv(rc.ypBottom - rc.ypTop, 72, vfli.dyuInch);
		break;

	case idPrintPsCell:
		CachePara(lr.doc, cp);
		if (!vpapFetch.fInTable)
			{
			idPrintPs = idPrintPsPage;
			goto LPage;
			}
		yp = lr.yl;
		dyp = lr.dyl;
		CacheTc(wwLayout, lr.doc, cp, fFalse, fFalse);
		xp = vtcc.xpDrLeft - vtcc.dxpOutLeft - DxFromBrc(vtcc.brcLeft, fTrue);
		lr.dxl = vtcc.xpDrLeft - xp;    /* save for Emit... */
		xpT = vtcc.xpDrRight + vtcc.dxpOutRight +DxFromBrc(vtcc.brcLeft, fTrue);
		dxp = xpT - xp;
		xp += lr.xl;
		lr.xl = xpT - vtcc.xpDrRight;
		/* now lr.dxl & lr.xl are the white space outside the table */
		goto LConvert;

	case idPrintPsRow:
		CachePara(lr.doc, cp);
		if (!vpapFetch.fInTable)
			{
			idPrintPs = idPrintPsPage;
			goto LPage;
			}
		CpFirstTap1(lr.doc, cp, fFalse);
		ItcGetTcx(wwLayout, &vtapFetch, 0, &tcx);
		xp = tcx.xpDrLeft - vtcc.dxpOutLeft - DxFromBrc(tcx.brcLeft, fTrue);
		lr.dxl = tcx.xpDrLeft - xp;    /* save for Emit... */
		ItcGetTcx(wwLayout, &vtapFetch, vtapFetch.itcMac, &tcx);
		xpT = tcx.xpDrLeft - tcx.dxpOutLeft;
		dxp = xpT - xp;
		xp += lr.xl;
		lr.xl = xpT - tcx.xpCellLeft +
				NMultDiv(vtapFetch.dxaGapHalf, vfli.dxuInch, dxaInch);
		/* now lr.dxl & lr.xl are the white space outside the table */
		yp = lr.yl;
		dyp = lr.dyl;
LConvert:
		xp = NMultDiv(xp, 72, vfli.dxuInch);
		yp = NMultDiv(yp, 72, vfli.dyuInch);
		dxp = NMultDiv(dxp, 72, vfli.dxuInch);
		dyp = NMultDiv(dyp, 72, vfli.dyuInch);
		lr.xl = NMultDiv(lr.xl, 72, vfli.dxuInch);
		lr.yl = NMultDiv(lr.yl, 72, vfli.dyuInch);
		break;

#ifdef DEBUG
	default:
		Assert(fFalse);
		return;
#endif
		}

/* the fillrect above has left the PS pen pattern white - restore black */
	EmitPsSz(SzFrame("initgraphics"));

/* translate coords to appropriate location on page. */
	rgw[0] = xp;
	rgw[1] = ypPage - yp - dyp;
	if (idPrintPs != idPrintPsPage)
		EmitComplexPs(SzFrame("%d %d translate"), rgw);

/* define origin, height and width variables */
	EmitComplexPs(SzFrame("/wp$xorig %d def"), rgw);
	EmitComplexPs(SzFrame("/wp$yorig %d def"), &rgw[1]);
	EmitComplexPs(SzFrame("/wp$y %d def"), &dyp);
	EmitComplexPs(SzFrame("/wp$x %d def"), &dxp);

/* define page number variables */
	rgw[0] = vfmtss.pgn;
	EmitComplexPs(SzFrame("/wp$page %d def"), rgw);
	rgch[CchLongToRgchNfc((LONG)vfmtss.pgn,rgch,vsepFetch.nfcPgn,31)] = 0;
	rgw[0] = rgch;
	EmitComplexPs(SzFrame("/wp$fpage (%s) def"), rgw);

/* define date ... */
	dttm = DttmCur();
	DefSzPicDTTmplt(fTrue /* fDate */, rgch, cchMaxPic);
	LocalSzPicDTTmplt(rgch, szPic);
	rgch[CchFormatDttmPic(&dttm, szPic, rgch, cchMaxPic)] = 0;
	rgw[0] = rgch;
	EmitComplexPs(SzFrame("/wp$date (%s) def"), rgw);

/* ...and time variables */
	DefSzPicDTTmplt(fFalse /* fDate */, rgch, cchMaxPic);
	LocalSzPicDTTmplt(rgch, szPic);
	rgch[CchFormatDttmPic(&dttm, szPic, rgch, cchMaxPic)] = 0;
	rgw[0] = rgch;
	EmitComplexPs(SzFrame("/wp$time (%s) def"), rgw);

/* define drawing box procedure */
	EmitPsSz(SzFrame("/wp$box { newpath 0 0 moveto wp$x 0 rlineto 0 wp$y rlineto wp$x neg 0 rlineto closepath } def"));

/* set clipping */
	EmitPsSz(SzFrame("wp$box clip newpath"));

	switch (idPrintPs)
		{
	case idPrintPsPic:
		break;

	case idPrintPsPara:
		CachePara(lr.doc, lr.cp);

		/* style name */
		pdod = PdodMother(lr.doc);
		stcp = StcpFromStc(vpapFetch.stc, pdod->stsh.cstcStd);
		GenStyleNameForStcp(rgch, pdod->stsh.hsttbName,pdod->stsh.cstcStd,stcp);
		rgch[rgch[0] + 1] = 0;
		rgw[0] = &rgch[1];
		EmitComplexPs(SzFrame("/wp$style (%s) def"), rgw);

		/* before/after */
		rgw[0] = lr.cp = cpPara ? vpapFetch.dyaBefore / 20 : 0;
		EmitComplexPs(SzFrame("/wp$top %d def"), rgw);
		rgw[0] = lr.cpLim == cpLimPara ? vpapFetch.dyaAfter / 20 : 0;
		EmitComplexPs(SzFrame("/wp$bottom %d def"), rgw);

		/* left/right */
		rgw[0] = vpapFetch.dxaLeft / 20;
		EmitComplexPs(SzFrame("/wp$left %d def"), rgw);
		rgw[0] = vpapFetch.dxaLeft1 / 20;
		EmitComplexPs(SzFrame("/wp$first %d def"), rgw);
		rgw[0] = vpapFetch.dxaRight / 20;
		EmitComplexPs(SzFrame("/wp$right %d def"), rgw);
		break;

	case idPrintPsCell:
	case idPrintPsRow:
		EmitPsSz(SzFrame("/wp$top 0 def"));
		EmitPsSz(SzFrame("/wp$bottom 0 def"));
		EmitComplexPs(SzFrame("/wp$left %d def"), &lr.dxl);
		EmitComplexPs(SzFrame("/wp$right %d def"), &lr.xl);
		break;

	case idPrintPsPage:
		/* top/bottom */
		rgw[0] = abs(dop.dyaTop) / 20;
		EmitComplexPs(SzFrame("/wp$top %d def"), rgw);
		rgw[0] = abs(dop.dyaBottom) / 20;
		EmitComplexPs(SzFrame("/wp$bottom %d def"), rgw);

		/* left/right */
		GetXlMargins(&dop, vfmtss.pgn & 1, 72, &xlLeft, &xlRight);
		EmitComplexPs(SzFrame("/wp$left %d def"), &xlLeft);
		dxp = dop.xaPage / 20;
		xlRight = dop.xaPage / 20 - xlRight;
		EmitComplexPs(SzFrame("/wp$right %d def"), &xlRight);

		/* column stuff */
		if ((pdod = PdodDoc(lr.doc))->fHdr)
			/* this leaves proper section cached */
			IhdtFromIhdd(pdod->doc, IInPlc(pdod->hplchdd, lr.cp), 0);
		else
			CacheSect(lr.doc, lr.cp);
		rgw[0] = vsepFetch.ccolM1 + 1;
		EmitComplexPs(SzFrame("/wp$col %d def"), rgw);
		rgw[0] = vsepFetch.dxaColumnWidth / 20;
		EmitComplexPs(SzFrame("/wp$colx %d def"), rgw);
		rgw[0] = (vsepFetch.ccolM1 == 0) ? 0 : vsepFetch.dxaColumns / 20;
		EmitComplexPs(SzFrame("/wp$colxb %d def"), rgw);
		break;
		}
}


/*******************************/
/* E m i t  C o m p l e x  P s */
/* %%Function:EmitComplexPs %%Owner:davidbo */
EmitComplexPs(psz, pw)
char *psz;
int *pw;
{
/* an incredibly trivial printf -- converts a string with optional control
	sequences into a PostScript command and emits it
		%d - use next value in *pw as signed decimal
		%s - used next value in *pw as psz
*/
	int cch;
	char *pchOut, *pchIn;
	char rgch[cchMaxSz];

	for (pchOut = rgch, pchIn = psz; *pchIn; )
		if (*pchIn != '%')
			*pchOut++ = *pchIn++;
		else
			{
			if (*++pchIn == 'd')
				CchIntToPpch(*pw++, &pchOut);
			else  if (*pchIn == 's')
				{
				cch = CchSz(*pw) - 1;
				bltb(*pw++, pchOut, cch);
				pchOut += cch;
				}
			pchIn++;
			}
	*pchOut = 0;
	EmitPsSz(rgch);
}


/*********************/
/* E m i t  P S  S z */
/* %%Function:EmitPsSz %%Owner:davidbo */
EmitPsSz(sz)
char *sz;
{
	/* output a PostScript command */
	int cch = CchSz(sz) - 1;
	char rgch[cchMaxSz];

	bltb(sz, &rgch[2], cch);
	cch += 2;
	rgch[cch] = chReturn;
	rgch[cch+1] = chEop;
	blt(&cch, (int *)rgch, 1);
	Escape(vpri.hdc, PASSTHROUGH, cch, (LPSTR) rgch, (LPSTR) NULL);
}


/*****************************/
/* F  G e t  P s  P i c  R c */
/* %%Function:FGetPsPicRc %%Owner:davidbo */
FGetPsPicRc(doc, cp, plr, prc)
int doc;
CP cp;
struct LR *plr; /* WARNING: do not pass a heap pointer into this routine! */
struct RC *prc;
{
	/* determine the bounding rectangle for the next picture to receive
		PostScript commands */
	int dyp = 0, dypMax = plr->dyl;
	int yp = plr->yl;
	int xp;
	int dxa, dxp;
	int *pdxp, ich, ichNext, chrm, fCrop;
	struct CHR *pchr;
	CP cpT, cpPic, cpLim;
	struct CHP chp;
	struct PICRC picrc;

	Assert(cp >= caPara.cpFirst);
	Assert(cp >= plr->cp);

/* find the picture in the para */
	cpPic = cp;
	CacheParaL(doc, cp, fFalse);
	if (vpapFetch.fInTable)
		{
		dxp = DxpDypToPara(doc, cp, &dyp);
		cpT = cp;
		}
	else
		{
		dxp = 0;
		cpT = plr->cp;
		}
	cpLim = caParaL.cpLim;
	do
		{
		if (cpPic >= cpLim)
			return fFalse;  /* no next picture */
		CachePara(doc, cpPic);
		FetchCp(doc, cpPic, fcmChars + fcmProps);
		cpPic += vccpFetch;
		}
	while (!(vchpFetch.fSpec && *vhpchFetch == chPicture));
	cpPic -= vccpFetch;

	if (cpPic < plr->cp)
		return fFalse;  /* picture on prev page */

/* determine if pic is in current LR...para might be split across page */
	dxa = NMultDiv(plr->dxl, dxaInch, vfli.dxuInch);
	for (;; cpT = vfli.cpMac)
		{
		FormatLineDxa(wwLayout, doc, cpT, dxa);
		if (cpPic < vfli.cpMac)
			break;

		dyp += vfli.dypLine;
		if (dyp >= dypMax)
			return fFalse; /* pic on next page */
		}

/* find the pic on the line */
	cpT = vfli.cpMin;
	xp = plr->xl + vfli.xpLeft + dxp;
	pdxp = &vfli.rgdxp[0];
	ich = 0;
	FreezeHp();
	for (pchr = &(**vhgrpchr)[0]; ; (char *)pchr += chrm)
		{
		for (ichNext = pchr->ich; cpT < cpPic && ich < ichNext; ich++, cpT++)
			xp += *pdxp++;
		if (cpT >= cpPic)
			break;

		if ((chrm = pchr->chrm) == chrmVanish)
			cpT += ((struct CHRV *)pchr)->dcp;
		else if (chrm == chrmFormula)
			{
			xp += ((struct CHRF *)pchr)->dxp;
			dyp += ((struct CHRF *)pchr)->dyp;
			}
		else if (chrm == chrmDisplayField)
			cpT += ((struct CHRDF *)pchr)->dcp-1;
		}
	MeltHp();
	if (cpT != cpPic)
		return(fFalse);         /* invisible picture */

	chp = pchr->chp;        /* beware of heap movement */
	FGetPicRc(&picrc, &fCrop, &chp, xp, yp + dyp + vfli.dypLine -
		vfli.dypBase, NULL, picgFrOut);
	*prc = picrc.frOut;
	return(fTrue);
}


/*********************************/
/* D x p   D y p   T o   P a r a */
/* %%Function:DxpDypToPara %%Owner:davidbo */
int DxpDypToPara(doc, cp, pdyp)
int doc;
CP cp;
int *pdyp;
{
	int dxp, dyp;
	CP cpLine;

	CacheTc(wwLayout, doc, cp, fFalse, fFalse);
	dxp = vtcc.xpDrLeft;
	dyp = vtcc.dylAbove;
	Mac(vfli.fStopAtPara =) vfli.fLayout = fTrue;
	for (cpLine = vtcc.ca.cpFirst; cpLine < cp; cpLine = vfli.cpMac, dyp += vfli.dypLine)
		{
		/* dxa 0 will be replaced by cell width */
		FormatLineDxa(wwLayout, doc, cpLine, 0);
		}
	Mac(vfli.fStopAtPara =) vfli.fLayout = fFalse;
	CachePara(doc, cp);
	*pdyp = dyp;
	return(dxp);
}


