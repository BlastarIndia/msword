/* F E T C H 1 . C */
#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "doc.h"
#include "file.h"
#include "props.h"
#include "border.h"
#include "fkp.h"
#include "ch.h"
#include "format.h"
#include "layout.h"
#include "prm.h"
#include "sel.h"
#include "disp.h"
#include "inter.h"
#include "cmd.h"
#include "debug.h"
#include "screen.h"
#include "ruler.h"
#include "pic.h"
#include "outline.h"
#include "print.h"
#ifdef MAC
#include "mac.h"
#include "toolbox.h"
#endif

#ifdef WIN
#ifdef PROTOTYPE
#include "fetch1.cpt"
#endif /* PROTOTYPE */
#endif  /* WIN */

/* E X T E R N A L S */

extern BOOL             vfEndFetch;
extern struct DOD       **mpdochdod[];

extern struct CHP       vchpFetch;
extern struct CHP       vchpStc;
extern struct FKPD      vfkpdChp;

extern char HUGE        *vhpchFetch;
extern int              vdocFetch;
extern CP               vcpFetch;
extern int              vccpFetch;
extern int              vdocTemp;

extern char             rgchInsert[];
extern int              ichInsert;
extern struct SELS      rgselsInsert[];

extern struct FCB       **mpfnhfcb[];

extern struct CA        caParaL;
extern struct CA        caTap;

extern struct FKPD      vfkpdPap;

extern struct CA        caSect;
extern int              vised;
extern struct CA        caPage;
extern int              vipgd;
extern CP               vcpFirstLayout;
extern int              vifnd;
extern struct CHP       vchpStc;
extern struct PAP       vpapFetch;
extern struct SEP       vsepFetch;
extern int              prmFetch;
extern struct LLC       vllc;
extern struct TAP       vtapStc;
extern struct TAP       vtapFetch;
extern struct TCC       vtcc;
extern CP               vmpitccp[];
extern int              vitcMic;

extern char             (**vhgrpchr)[];
extern int              vbchrMax;
extern int              vbchrMac;

extern struct FLI       vfli;
extern struct FTI       vfti;
extern struct PREF      vpref;
extern struct SEL       selCur;

extern struct FLS       vfls;
extern struct MERR      vmerr;
extern struct DBS       vdbs;

extern int              wwCur;
extern struct WWD       **hwwdCur;

extern struct ITR       vitr;

extern BOOL             vfShowAllStd;
extern struct SCI       vsci;

extern int              vjnRare;
extern int              vjcRare;
extern int              vlm;
extern BOOL		vfFliMunged; /* returned from CachePage */
extern int		docGlobalDot;

extern struct TCXS		vtcxs;
#ifdef WIN
extern int              vflm;
extern struct PRI	vpri;
#ifdef DEBUG
extern int		vfDypsInScreenUnits;
#endif
#endif

#ifdef MAC
extern int		vftcHelvetica;
#endif

#ifdef JR
extern int              vfJRProps;
#endif /* JR */


/*  For swap tuning, make prl.c part of fetch1 */
#ifdef WIN
#include "prl.c"
#endif /* WIN */


/* G L O B A L S */
/* list of approved font sizes, in half points */
#ifdef MAC
/*              7   9  10  12  14  18  24  36  48  60   72 */
	int rghps[] = {
	14, 18, 20, 24, 28, 36, 48, 72, 96, 120, 144	};


#else
/*              6   8  10  12  14  18  24  36  48  60   72 */
	int rghps[] = {
	12, 16, 20, 24, 28, 36, 48, 72, 96, 120, 144	};


#endif
#define ihpsApprovedMax	(sizeof(rghps)/sizeof(int))


extern CP CpFromIpgd();


/* H P S  A L T E R */
/* Return the hps of the approved font size that is ialter steps away from
the given size.  I.e.: if ialter is -1, then return the next smaller size.
If ialter is 0, return hps. */
/* %%Function:HpsAlter %%Owner:davidlu */
EXPORT int HpsAlter(hps, ialter)
int hps, ialter;
{
	int ihps;

	if (ialter == 0)
		return hps;

	/* Find the size just larger than the given size. */
	if (ialter > 0)
		{
		for (ihps = 0; ihps < ihpsApprovedMax - 1; ++ihps)
			if (rghps[ihps] > hps) break;
		ihps = min(ihpsApprovedMax - 1, ihps + ialter - 1);
		return max(hps, rghps[ihps]);
		}
	else
		{
		for (ihps = 0; ihps < ihpsApprovedMax; ++ihps)
			if (rghps[ihps] >= hps) break;
		ihps = max(0, ihps + ialter);
		return min(hps, rghps[ihps]);
		}
}


/* C A C H E  S E C T  S E D  M A C  */
/* CacheSect call that allows caSect.cpLim of last section to be reported
	as CpMac1Doc(). Useful when CacheSect is used to determine the location
	of the end of section so a section sprm may be applied. */
/* %%Function:CacheSectSedMac %%Owner:davidlu */
CacheSectSedMac(doc, cp)
int doc;
CP cp;
{
	struct PLC **hplcsed;
	CacheSect(doc, cp);
	if (caSect.cpLim == CpMacDoc(doc))
		{
		if (caSect.cpFirst == caSect.cpLim)
			caSect.cpLim = CpMac1Doc(doc);
		else  if (!FMultipleSect(doc, &hplcsed) ||
				CpPlc(hplcsed, IMacPlc(hplcsed) - 2) < CpMacDoc(doc))
			caSect.cpLim = CpMac1Doc(doc);
		}
}


/* C A C H E  S E C T  C A */
/* alternate entry point */
/* %%Function:CacheSectCa %%Owner:davidlu */
CacheSectCa(pca)
struct CA *pca;
{
	CacheSect(pca->doc, pca->cpFirst);
}


/* C A C H E  S E C T */
/* returns section props (normal + sepx in file + prm in piece table) in
	vsepFetch
and section boundaries in
	caSect
*/
/* %%Function:CacheSectProc %%Owner:davidlu */
EXPORT CacheSectProc(doc, cp)
int doc;
CP cp;
{
/* note: care is needed when recursing, because endnotes can cause the cp
	of the main doc to be CpMacDoc */
	char HUGE *hpchSepx;
	struct DOD *pdod;
	struct DOP *pdop;
	struct PLC **hplcsed;
	int fHdr, fAtn = fFalse, nfcPgn, ww;
	int fpc;
	struct PLC **hplcpcd;
	CP cpMac;
	struct CA ca;
	struct SED sed;
	struct PCD pcd;

#ifdef JR
	if (vfJRProps)
		cp = cp0;
#endif /* JR */
#ifdef WIN
	/* All callers should be checking FInCa before calling CacheSect. */
	Assert(!FInCa(doc, cp, &caSect));
#else /* !WIN */
	if (FInCa(doc, cp, &caSect)) return;
#endif /* !WIN */
	pdod = PdodDoc(doc);
	FreezeHp();
	Assert(cp < pdod->cpMac);
	if (pdod->fFtn || (fAtn = pdod->fAtn))
		{
/* if footnote text: section properties will be taken from the sect of
the reference. caSect will be set up to envelope the footnote text. */
		struct PLC **hplcfnd = pdod->hplcfnd;
		struct PLC **hplcRef;
		int docMom = pdod->doc;

		int ifnd = IInPlc(hplcfnd, cp);
		PcaSet(&ca, doc, CpPlc(hplcfnd, ifnd),
				CpPlc(hplcfnd, ifnd + 1));
		hplcRef = WinMac((pdod->fFtn) ? PdodDoc(docMom)->hplcfrd : PdodDoc(docMom)->hplcatrd,
				PdodDoc(docMom)->hplcfrd);
		Win(AssertH(hplcRef));
		fpc = (fAtn) ? fpcBottomPage : PdodDoc(docMom)->dop.fpc;
		cpMac = CpMacDoc(docMom);
		if (fpc == fpcEndDoc)
			CacheSect(docMom, cpMac);
		else
			{
			CacheSect(docMom, CpMin(CpPlc(hplcRef, ifnd), cpMac));
			/* for endnotes, find the section where they will be included */
			if (fpc == fpcEndnote)
				while (caSect.cpLim < cpMac && !vsepFetch.fEndnote)
					CacheSect(docMom, caSect.cpLim);
			}

LReturnCa:      
		caSect = ca;
		caSect.doc = doc;
		MeltHp();
		return;
		}
/* for temp file during page layout, use mother doc's props */
	if (doc == vdocTemp && vcpFirstLayout != cpNil)
		{
		PcaSet(&ca, doc, cp0, CpMacDoc(doc));
		CacheSect(pdod->doc,
				CpMin(vcpFirstLayout, CpMacDoc(pdod->doc)));
		goto LReturnCa;
		}

/* for headers, get and save props of mother doc's section */
	caSect.doc = doc;
	if (fHdr = (pdod->fHdr || pdod->dk == dkDispHdr))
		{
		CacheSect(pdod->doc, (pdod->dk == dkDispHdr) ? CpMomFromHdr(doc) :
				vcpFirstLayout == cpNil ? cp0 : 
				CpMin(CpMacDoc(pdod->doc), vcpFirstLayout));
		nfcPgn = vsepFetch.nfcPgn;
		caSect.doc = doc;       /* reset doc to hdr doc */
		}

	StandardSep(&vsepFetch);
#ifdef MAC
	/* compatibility with Word 3, which had zero space between as default */
	if (pdod->fn != fnNil && PfcbFn(pdod->fn)->fWord3File)
		vsepFetch.dxaColumns = 0;
#endif

	if (pdod->fShort || (hplcsed = pdod->hplcsed) == 0)
		{
		vised = -1;
/* last chEop in a file will act as a section break */
		caSect.cpFirst = cp0;
		caSect.cpLim = CpMac1Doc(doc);
		}
	else
		{
		vised = IInPlc(hplcsed, cp);

		caSect.cpFirst = CpPlc(hplcsed, vised);
		caSect.cpLim = CpPlc(hplcsed, vised + 1);
		GetPlc(hplcsed, vised, &sed);
		if (sed.fcSepx != fcNil)
			{
			hpchSepx = HpchFromFc(sed.fn, sed.fcSepx);
			Assert(*hpchSepx != 0);
#ifdef MAC
			ApplyPrlSgc(hpchSepx + 1, *hpchSepx,
					&vsepFetch, sgcSep, 
					WinMac(fFalse, PfcbFn(sed.fn)->fWord3File));
#else
			ApplyPrlSgc(hpchSepx + 1, *hpchSepx,
					&vsepFetch, sgcSep);
#endif /* MAC */
			}
		}
/* apply prm's if any from piece table */
	hplcpcd = pdod->hplcpcd;

	GetPlc(hplcpcd,
			IInPlc(hplcpcd, caSect.cpLim - 1), &pcd);
	if (pcd.prm)
		DoPrmSgc(pcd.prm, &vsepFetch, sgcSep);
	if (fHdr)
		vsepFetch.nfcPgn = nfcPgn;

	caSect.cpLim = CpMin(CpMacDoc(doc), caSect.cpLim);
/* calculate dependent quantities in the sep */
	pdop = &PdodMother(doc)->dop;
	/* bullet-proofing */
	if (vsepFetch.ccolM1 < 0)
		vsepFetch.ccolM1 = 0;
	vsepFetch.dxaColumnWidth = max(16, 
			(pdop->xaPage - pdop->dxaLeft - pdop->dxaRight -
			pdop->dxaGutter -
			vsepFetch.ccolM1 * vsepFetch.dxaColumns) / (vsepFetch.ccolM1 + 1));
#ifdef JR
	if (vfJRProps)
		{
		caSect.cpLim = CpMacDoc(doc);
		vsepFetch.bkc = bkcNewPage;
		vsepFetch.fPgnRestart = vsepFetch.fEndnote = fFalse;
		vsepFetch.nfcPgn = nfcArabic;
		vsepFetch.lnc = lncPerPage;
		vsepFetch.nLnnMod = 0;
		vsepFetch.dxaLnn = 0;
		}
#endif /* JR */
	MeltHp();
}



/* F  B O R D E R  P A P
/* Returns true iff there are any borders turned on in the pap */
/* %%Function:FBorderPap %%Owner:davidlu */
FBorderPap(ppap)
struct PAP *ppap;
{
	return CchNonZeroPrefix(&ppap->brcTop,cbrcParaBorders*sizeof(int)) != 0;
}


/* M A P  S T C  S T A N D A R D */
/* supplies chp and pap preset values. To be called only if there is
no user definition for the stc.
chp and pap must be pre-loaded with Normal.
*/
/* %%Function:MapStcStandard %%Owner:davidlu */
EXPORT MapStcStandard(stc, pchp, ppap)
int stc; 
struct CHP *pchp; 
struct PAP *ppap;
{
	int fMetric = vitr.fMetric;
	int dxa;
	struct CHP chpT;
	struct PAP papT;
#define dxaCm 567

	if (pchp == 0) pchp = &chpT;
	if (ppap == 0) ppap = &papT;

	ppap->stc = stc;
	for (;;)
		switch (stc)
			{
		case stcStdMin:
LStcMin:
			StandardPap(ppap);
			StandardChp(pchp);
			return;
#ifdef WIN
		case stcNormIndent:
			{
			if (docGlobalDot == docNil)
				goto LStcMin;

			if ((ppap->dxaLeft = PdopDoc(docGlobalDot)->dxaTab) == 0)
				ppap->dxaLeft = fMetric ? 709 : dxaInch/2;
			}
			return;
#endif
		case stcLev1:
			ppap->dyaBefore = 12 * dyaPoint;
			pchp->kul = kulSingle;
			pchp->fBold = ~pchp->fBold;
			pchp->hps = 24;
			pchp->ftc = ftcHelv;
			return;
		case stcLev2:
			ppap->dyaBefore = 6 * dyaPoint;
			pchp->fBold = ~pchp->fBold;
			pchp->hps = 24;
			pchp->ftc = ftcHelv;
			return;
		case stcLev3:
			{
			struct DOP *pdop;
			
			if (docGlobalDot == docNil)
					goto LStcMin;
			pdop = PdopDoc(docGlobalDot);
			if (pdop->dxaTab)
				ppap->dxaLeft = pdop->dxaTab/2;
			else
				ppap->dxaLeft = fMetric ? 355 : dxaInch/4;
			}
			pchp->fBold = ~pchp->fBold;
			pchp->hps = 24;
			return;
		case stcLev4:
			{
			struct DOP *pdop;

			if (docGlobalDot == docNil)
				goto LStcMin;

			pdop = PdopDoc(docGlobalDot);
			if (pdop->dxaTab)
				ppap->dxaLeft = pdop->dxaTab/2;
			else
				ppap->dxaLeft = fMetric ? 355 : dxaInch/4;
			}
			pchp->kul = kulSingle;
			pchp->hps = 24;
			return;
		case stcLev5:
			{
			if (docGlobalDot == docNil)
				goto LStcMin;

			if ((ppap->dxaLeft = PdopDoc(docGlobalDot)->dxaTab) == 0)
				ppap->dxaLeft = fMetric ? 709 : dxaInch/2;
			}
			pchp->fBold = ~pchp->fBold;
			pchp->hps = 20;
			return;
		case stcLev6:
			{
			if (docGlobalDot == docNil)
				goto LStcMin;

			if ((ppap->dxaLeft = PdopDoc(docGlobalDot)->dxaTab) == 0)
				ppap->dxaLeft = fMetric ? 709 : dxaInch/2;
			}
			pchp->kul = kulSingle;
			pchp->hps = 20;
			return;
		case stcLev7:
		case stcLev8:
		case stcLev9:
			{
			if (docGlobalDot == docNil)
				goto LStcMin;

			if ((ppap->dxaLeft = PdopDoc(docGlobalDot)->dxaTab) == 0)
				ppap->dxaLeft = fMetric ? 709 : dxaInch/2;
			}
			pchp->fItalic = ~pchp->fItalic;
			pchp->hps = 20;
			return;
		case stcIndex1:
		case stcIndex2:
		case stcIndex3:
		case stcIndex4:
		case stcIndex5:
		case stcIndex6:
		case stcIndex7:
/* (level - 1) * .5 cm indent */
/* (level - 1) * 1/4" indent */
			ppap->dxaLeft = fMetric ? (stcIndexLast-stc)*dxaCm/2
					        : (stcIndexLast-stc)*dxaInch/4;
			return;
		case stcFtnText:
#ifdef WIN
		case stcAtnText:
#endif /* WIN */
			pchp->hps = 10 * 2;
			return;
		case stcFtnRef:
			pchp->hps = WinMac ( 8 * 2, 9 * 2 );
			pchp->hpsPos = 3 * 2;
			return;
#ifdef WIN
		case stcAtnRef:
			pchp->hps = 8 * 2;
/*			pchp->hpsPos = 256 - (3 * 2); */
			return;
#endif /* WIN */
		case stcHeader:
		case stcFooter:
			if (docGlobalDot == docNil)
				goto LStcMin;

			SetBytes((char *)ppap + (cwPAPBase * sizeof(int)),
					0, cbPAP - (cwPAPBase * sizeof(int)));
			{
			struct DOP *pdop = PdopDoc(docGlobalDot);

			ppap->rgdxaTab[0]=(pdop->xaPage-pdop->dxaLeft-pdop->dxaRight)/2; 
			ppap->rgdxaTab[1]=pdop->xaPage-pdop->dxaLeft-pdop->dxaRight;
			}
			ppap->itbdMac = 2;
			ppap->rgtbd[0] = Tbd(jcCenter, tlcNone);
			ppap->rgtbd[1] = Tbd(jcRight, tlcNone);
			return;
#ifdef MAC
		case stcPostScript:
			pchp->fVanish = pchp->fBold = fTrue;
			pchp->hps = 10 * 2;
			return;
#endif	/* MAC */
		default:
			if (stc >= stcTocMin && stc <= stcTocLast)
				{
				struct DOP *pdop;

				if (docGlobalDot == docNil)
					goto LStcMin;

				pdop = PdopDoc(docGlobalDot);
				
				if (fMetric)
					{
					ppap->dxaLeft = 
						NMultDiv(stcTocLast - stc,
							2835, 4);
					ppap->dxaRight = 850; 
					ppap->rgdxaTab[0] = pdop->xaPage-pdop->dxaLeft-pdop->dxaRight-425; 
					ppap->rgdxaTab[1] = pdop->xaPage-pdop->dxaLeft-pdop->dxaRight; 
					ppap->itbdMac = 2;
					ppap->rgtbd[0] = Tbd(jcLeft,tlcDot);
					ppap->rgtbd[1] = Tbd(jcRight,tlcNone);
					}
				else
					{
					ppap->dxaLeft = (stcTocLast - stc) *
							dxaInch / 2;
					ppap->dxaRight = dxaInch / 2;
					ppap->rgdxaTab[0] = pdop->xaPage-pdop->dxaLeft-pdop->dxaRight-dxaInch/4; 
					ppap->rgdxaTab[1] = pdop->xaPage-pdop->dxaLeft-pdop->dxaRight;
					ppap->itbdMac = 2;
					ppap->rgtbd[0] = Tbd(jcLeft,tlcDot);
					ppap->rgtbd[1] = Tbd(jcRight,tlcNone);
					}
				}
			return;
		/* else normal properties are returned */
			}
}


/* C A C H E  P A G E */
/* %%Function:CachePage %%Owner:chrism */
EXPORT CachePage(doc, cp)
int doc;
CP cp;
{
/* Find the boundaries of the page containing cp. If either boundary is
	represented by (cp,cl), reduce to a simple cp for speed on later calls.
	Restores vfli to its previous state because this routine is called
	from DisplayFli; also maintains the port (which may be a dialog box) */
	struct PLC **hplcpgd;
	CP *pcp;
	int ipgd, ipgdMac;
	int fWrongPage = fFalse;
	int fUnkFirst;
	CP cpFirst, cpLim;
	struct PGD pgd, pgdPrev;
	struct CA caT;


	Assert(cHpFreeze == 0);
	if (FInCa(doc, cp, &caPage))
		return;
	hplcpgd = PdodDoc(doc)->hplcpgd;

	if (hplcpgd == hNil || (ipgdMac = IMacPlc(hplcpgd)) == 0)
		{
		/* page table is empty */
		vipgd = 0;
		cpFirst = cp0;
		cpLim = cpMax;
		goto LRet;
		}

	vfFliMunged = fFalse;

/* set cp's */
	cpFirst = CpPlc(hplcpgd, vipgd = IInPlcMult(hplcpgd, cp));
/* if there are empty pages, we want the one that actually contains cpFirst */
	for ( ; ; vipgd++)
		{
		GetPlc(hplcpgd, vipgd, &pgd);
		if (!(pgd.fGhost && vipgd < ipgdMac - 1 &&
				cpFirst == CpPlc(hplcpgd, vipgd + 1)))
			break;
		}
	cpLim = CpPlc(hplcpgd, vipgd + 1);

/* if page break is not at para bounds, get a raw cp from (cp, cl) */
/* NOTE: this doesn't bother to adjust dcpDepend in the affected pgd's,
	which will make editing near the page edge a little more sensitive */
	fUnkFirst = pgd.fUnk;
	if (pgd.cl != 0)
		{
		Assert(vipgd > 0);
		GetPlc(hplcpgd, vipgd - 1, &pgdPrev);
		/* note that the PREVIOUS page's fUnk indicates whether we can
		   trust this page's starting point */
		if (!pgdPrev.fUnk)
			{
			cpFirst = CpFromIpgd(doc, wwCur, vipgd); /* heap movement */
			vfFliMunged = fTrue;
			if (cpFirst > cp)
				{
				/* very tricky: cp is in the part of the previous
					page described by cl; vipgd is 1 too big */
				cpLim = cpFirst;
				pgd.cl = 0;
				PutPlc(hplcpgd, vipgd, &pgd);
				PutCpPlc(hplcpgd, vipgd, cpLim);
				cpFirst = CpFromIpgd(doc, wwCur, --vipgd);/* HM */
				pgd = pgdPrev;
				fUnkFirst = fFalse;
				fWrongPage = fTrue;
				}
			pgd.cl = 0;
			PutPlc(hplcpgd, vipgd, &pgd);
			PutCpPlc(hplcpgd, vipgd, cpFirst);
			}
		}

	if (!fWrongPage && vipgd < ipgdMac - 1)
		{
		GetPlc(hplcpgd, vipgd + 1, &pgd);
		if (pgd.cl != 0 && !fUnkFirst)
			{
			cpLim = CpFromIpgd(doc, wwCur, vipgd + 1);/* HM */
			vfFliMunged = fTrue;
			pgd.cl = 0;
			PutPlc(hplcpgd, vipgd + 1, &pgd);
			PutCpPlc(hplcpgd, vipgd + 1, cpLim);
			}
		}

/* make sure no garbage entries got in the way */
	if (cpLim == cpFirst && vipgd < ipgdMac - 1)
		{
		for (ipgd = vipgd + 1;
				ipgd <= ipgdMac && (cpLim = CpPlc(hplcpgd, ipgd)) == cpFirst;
				ipgd++)
			;
		}
LRet:
	caPage.cpFirst = cpFirst;
	caPage.cpLim = cpLim;
	caPage.doc = doc;
	Assert(FInCa(doc, cp, &caPage));
}


/*************************/
/* C p  F r o m  I p g d */
/* %%Function:CpFromIpgd %%Owner:chrism */
CP CpFromIpgd(doc, ww, ipgd)
int doc, ww, ipgd;
{
/* return the true CP at which a page starts; this routine will return a
	best approximation if the document has been edited */
	struct PLC **hplcpgd = PdodDoc(doc)->hplcpgd;
	CP cp;
	struct LBS lbs;
#ifdef WIN
	struct LBS lbsT;
	int lmSave, flmSave;
#endif /* WIN */
#ifdef MAC
	struct WDS wds;
#endif /* MAC */
	struct PGD pgd;

/* not much to do if no cl */
	Assert(cHpFreeze == 0);
	Assert(hplcpgd != hNil);
	if (ipgd == ipgdNil || ipgd >= IMacPlc(hplcpgd))
		{
		Assert(fFalse);
		return cp0;
		}
	cp = CpPlc(hplcpgd, ipgd);
	GetPlc(hplcpgd, ipgd, &pgd);
	if (pgd.cl == 0)
		return(cp);

/* otherwise get the raw cp */
	SetWords(&lbs, 0, cwLBS);
#ifdef WIN
	flmSave = vflm;
	lmSave = vlm;
	if (!FInitWwLbsForRepag(ww, doc, vlm == lmNil ? lmBRepag : vlm, &lbs, &lbsT))
		{
		EndFliLayout(lmSave, flmSave);
		return(cp);	/* cp is not critical, do best guess */
		}
#else
	SaveWds(&wds);
	/* done in FInitLbsForRepag for WIN */
	lbs.ww = ww;
	lbs.doc = doc;
	lbs.fOutline = PwwdWw(ww)->fOutline;
#endif

	lbs.cl = pgd.cl;
	lbs.cp = cp;
	PwwdSetWw(wwCur, cptSame);
	cp = CpFromCpCl(&lbs, fFalse);
#ifdef WIN
	EndFliLayout(lmSave, flmSave);
#else
	RestoreWds(&wds);
#endif
	return(cp);
}


/**************************/
/* C p  F r o m  C p  C l */
#ifndef WIN
NATIVE /* WINIGNORE - MAC only */
#else
EXPORT /* CpFromCpCl */
#endif
/* %%Function:CpFromCpCl %%Owner:chrism */
CP CpFromCpCl(plbs, fCritical)
struct LBS *plbs;
int fCritical;  /* cp absolutely must be correct */
{
/* convert the cp,cl pair in plbs to a cp,0 pair */
	int dxa;
	int cl;
	BOOL fLayout;
	struct PLC **hplclnh;

	if (plbs->cl == 0)
		return(plbs->cp);
	CacheSect(plbs->doc, plbs->cp);
	fLayout = vfli.fLayout;
	vfli.fLayout = fTrue; /* correct splats */

/* WIN wants printer unit here not screen unit, not even XlFromXa */
	Debug(CheckFlmState());
	caParaL.doc = docNil;
	CacheParaL(plbs->doc, plbs->cp, plbs->fOutline);
	ClFormatLines(plbs, cpMax, ypLarge, ypLarge, clMax,
			NMultDiv(vsepFetch.dxaColumnWidth, WinMac(vfli.dxuInch, vfli.dxsInch), dxaInch),
			fFalse, fFalse);
	/* verbose for native complier */
	vfli.fLayout = fLayout;
	if (vmerr.fMemFail)
		return(plbs->cp + fCritical);
	hplclnh = vfls.hplclnh;
	cl = IInPlcCheck(hplclnh, plbs->cp);
	Assert(cl >= 0);
	if (cl < 0 || cl + plbs->cl >= IMacPlc(hplclnh))
		{
		Assert(!fCritical);
		/* adding fCritical is a failsafe which gives back SOMETHING
			and still advances */
		return(plbs->cp + fCritical);
		}
	return(CpPlc(hplclnh, cl + plbs->cl));
}


#ifdef DEBUG
/* C h e c k   F l m   S t a t e */
/* %%Function:CheckFlmState %%Owner:chrism */
EXPORT CheckFlmState()
{
/* verifies that the current flm/lm state is correct for layout */
#ifdef WIN
	Assert(vflm == flmRepaginate || vflm == flmPrint ||
		vfDypsInScreenUnits || (vlm == lmPagevw && vpri.hdc == NULL));
#endif /* WIN */
}
#endif /* DEBUG */


/* F  E X P A N D  G R P C H R */
/* expands grpchr table by 25%.
vbchrMac must be incremented by the caller beforehand by cbChr.
*/
/* %%Function:FExpandGrpchr %%Owner:davidlu */
EXPORT FExpandGrpchr(cbChr)
{
	int bchr = max(vbchrMax + vbchrMax / 4, vbchrMac);
/* space for one CHRE entry is always reserved */
	if (FRareF(reChngSizeH, FChngSizeHCw(vhgrpchr, CwFromCch(bchr + cbCHRE), fFalse)))
		{
		vbchrMax = bchr;
		return (fTrue);
		}
	else
		{
		vbchrMac -= cbChr;
		vfli.fError = fTrue;
		return (fFalse);
		}
}


#ifdef MAC	/* Not fields-sensitive like Opus version */
/* F E T C H  C P  V I S I B L E */
/* %%Function:FetchCpVisible %%Owner:NOTUSED */
FetchCpVisible(doc, cp, cpLim)
int doc;
CP cp, cpLim;
{
/* Fetch only visible characters between cp and cpLim.
	Use cp=cpNil for sequential access
	Assumes cpLim < CpMacDoc(doc)
*/
	if (cp == cpNil)
		CachePara(vdocFetch, vcpFetch + vccpFetch);
	else
		CachePara(doc, cp);
	FetchCp(doc, cp, fcmChars + fcmProps);
	while (vchpFetch.fVanish && vcpFetch < cpLim &&
			(vchpFetch.fSysVanish || !vpref.fSeeHidden))
		{
		CachePara(vdocFetch, vcpFetch + vccpFetch);
		FetchCp(docNil, cpNil, fcmChars + fcmProps);
		}
}


#endif	/* MAC */

/* P U S H  I N S E R T  S E L  C U R */
/* pushes selCur onto rgselsInsert, remembering it for
the Go Back command.
*/
/* %%Function:PushInsSelCur %%Owner:davidlu */
PushInsSelCur()
{
	PushInsSels(&selCur);
}


/* P U S H  I N S E R T  S E L S */
/* %%Function:PushInsSels %%Owner:davidlu */
PushInsSels(pselsPush)
struct SELS *pselsPush;
{
	int isels;
	struct SELS selsT, *pselsT;

	/* check if sels being pushed is already in the go back cache */
	selsT = *(struct SELS *)pselsPush;
	NormalizeSels(&selsT);
	for (isels = 0, pselsT = &rgselsInsert[0]; isels < iselsInsertMax; isels++)
		{
		/* if already cached, don't cache it again */
		if (!FNeRgw(&((pselsT++)->ca), &(selsT.ca), cwCA))
			return;
		}

	/* if not, add it to the end of the cache, pushing everything else up */
	blt(&rgselsInsert[1], &rgselsInsert[0], cwSELS * (iselsInsertMax - 1));
	*(pselsT = &rgselsInsert[iselsInsertMax - 1]) = *pselsPush;

	NormalizeSels(pselsT);
}


/* %%Function:NormalizeSels %%Owner:davidlu */
NormalizeSels(psels)
struct SELS *psels;
{
	int sk = psels->sk;
	if (sk != skColumn && sk != skRows && sk != skBlock)
		psels->xpFirst = psels->xpLim = xpNil;
	if (psels->sty == styChar)
		psels->sty = styNil;
}


#ifdef MAC

/* %%Function:StStAppend %%Owner:NOTUSED */
NATIVE StStAppend(st1, st2) /* WINIGNORE - MAC only */
char *st1, *st2;
	{/*
		Description:    Appends st2 to st1.
		Returns:        nothin'
	*/
	int cch;
	if ((cch = *st2) + *st1 > 255)
		cch = 255 - *st1;

	bltb(&st2[1], &st1[*st1 + 1], cch);
	*st1 += cch;
}


/* S T  T O  S Z  */
/* %%Function:StToSz %%Owner:NOTUSED */
StToSz(st, sz)
char *st, *sz;
{
	int cch = *st;

	bltb(st + 1, sz, cch);
	sz[cch] = 0;
}


#endif /* MAC */


#ifdef MAC
/* C C H  I N T  T O  R G C H  N F C */
/* Format the unsigned integer w according to nfc.
Result to pch, return cch <= cchMax, 0 if cchMax is reached.
*/
/* %%Function:CchIntToRgchNfc %%Owner:NOTUSED */
int CchIntToRgchNfc(n, pch, nfc, cchMax)
char *pch;
unsigned n, cchMax;
int nfc;
{
	int chA = 'a';
	int cch, ich;

	switch (nfc)
		{
	case nfcArabic:
		if (cchMax < 5)
			return 0;
		return CchUnsToPpch(n, &pch);
	case nfcUCRoman:
		return CchStuffRoman(&pch, n, fTrue, cchMax);
	case nfcLCRoman:
		return CchStuffRoman(&pch, n, fFalse, cchMax);
	case nfcUCLetter:
		chA = 'A';
		/* FALL THROUGH */
	case nfcLCLetter:
		if ((cch = (n - 1) / 26 + 1) > cchMax)
			{
			*pch = '*';	/* show something */
			return 1;
			}
		for (ich = 0; ich < cch; ich++)
			*pch++ = (n - 1) % 26 + chA;
		return cch;
	default:
		Assert(fFalse);
		return 0;
		}
}


/* C C H  U N S  T O  P P C H */
/* %%Function:CchUnsToPpch %%Owner:NOTUSED */
int CchUnsToPpch(n, ppch)
uns n;
char **ppch;
{
	int cch = 0;
	long ln;

	if (n >= 10)
		{
		ln = n;
		cch += CchUnsToPpch((uns) (ln / 10), ppch);
		n = (uns) (ln % 10);
		}

	*(*ppch)++ = '0' + n;
	return cch + 1;
}


#ifdef DEBUG
/* F   R A R E   P R O C */
/* this procedure returns true if the selected rare event has
occured the selected number of times
	vjnRare         identifier of event
	vjcRare         delay number of times
These variables will be typically set from the debugger or other debug
routines.
*/
/* %%Function:FRareProc %%Owner:NOTUSED */
NATIVE int FRareProc(n) /* WINIGNORE - MAC only */
int n;
{
	if (vjnRare == n)
		{
		if (--vjcRare < 0) return fTrue;
		}
	return fFalse;
}


#endif	/* DEBUG */

/* W  C O M P   S T */
/*
Compare an st1 to st2.  This routine is NOT case sensitive.
returns:    - if st1 < st2
		0 if st1 = st2
		+ if st1 > st2
*/
/* %%Function:WCompSt %%Owner:NOTUSED */
NATIVE int WCompSt(st1, st2) /* WINIGNORE - MAC only */
char *st1, *st2;
{
	int cchSt, cchSt1, cchSt2;
	int dcch = (cchSt1 = *st1++) - (cchSt2 = *st2++);
	for (cchSt = min(cchSt1, cchSt2); cchSt-- != 0; ++st1, ++st2)
		{
		int dch = ChLower(*st1) - ChLower(*st2);
		if (dch != 0) return dch;
		}
	return dcch;
}


#endif	/* MAC */

#ifdef JR
/* %%Function:JuniorChp %%Owner:NOTUSED */
JuniorChp()
{{ /* WINIGNORE - JR only */
	if (vfJRProps)
		{
		vchpFetch.fStrike = fFalse;
		vchpFetch.fSmallCaps = fFalse;
		vchpFetch.fCaps = fFalse;
		if (!vchpFetch.fSysVanish)
			vchpFetch.fVanish = fFalse;
		vchpFetch.qpsSpace = 0;
		if (vchpFetch.kul != kulNone)
			vchpFetch.kul = kulSingle;
		}
}}


/* %%Function:JuniorPap %%Owner:NOTUSED */
JuniorPap()
{{ /* WINIGNORE - JR only */
	if (vfJRProps)
		{
		vpapFetch.fTable = vpapFetch.fPageBreakBefore = vpapFetch.fNoLnn = fFalse;
		vpapFetch.brcp = brcpNone;
		vpapFetch.brcl = brclSingle;
		vpapFetch.nfcSeqNumb = nfcArabic;
		vpapFetch.nnSeqNumb = 0;
		vpapFetch.dyaAfter = 0;
		}
}}



/* here for now */
/* %%Function:CmdTitlePage %%Owner:NOTUSED */
CmdTitlePage(pcmb)
CMB *pcmb;
{
	char rgb[2];
	struct CA ca;

	if (wwCur == wwNil || PdodDoc(selCur.doc)->fShort)
		return cmdError;
	vfJRProps = fFalse;
	caSect.doc = docNil;
	CacheSectSedMac(selCur.doc, cp0);
	ca = caSect;
	rgb[0] = sprmSFTitlePage;
	rgb[1] = (vsepFetch.fTitlePage) ? 0 : 1;
	ApplyGrpprlCa(rgb, 2, &ca);
	vfJRProps = fTrue;
	caSect.doc = docNil;
	return cmdOK;
}


#endif /* JR */


/* F   M U L T I P L E   S E C T */
/* %%Function:FMultipleSect %%Owner:davidlu */
int FMultipleSect(doc, phplcsed)
int doc;
struct PLC ***phplcsed;
{
/* return fTrue if document has more than one section */
	struct PLC **hplcsed;

	doc = DocMother(doc);
	*phplcsed = hplcsed = PdodDoc(doc)->hplcsed;
	return(hplcsed != hNil && IMacPlc(hplcsed) > 2);
}


/* C P  R E F  F R O M  C P  S U B */
/* Return reference cp of footnote/annotation containing given cp. docSub is the
document with fFtn/Atn.
vifnd is also returned.
*/
/* %%Function:CpRefFromCpSub %%Owner:davidlu */
EXPORT CP CpRefFromCpSub(docSub, cp, edcDrp)
int docSub;
CP cp;
int edcDrp;
{
	struct DOD *pdodSub = PdodDoc(docSub);
	struct DRP *pdrp = ((int *)PdodDoc(pdodSub->doc)) + edcDrp;
	struct PLC **hplcRef = pdrp->hplcRef;

	Mac(Assert(edcDrp == edcDrpFtn));
	/* multiple values possible at FInsertRgch, before plcfnd is updated */
	vifnd = IInPlcMult(pdodSub->hplcfnd, cp);
	return (CpPlc(hplcRef, vifnd));
}


/* C P  F I R S T  R E F */
/* Return cpFirst of text of first footnote/annotation with ref >= cp.
If no footnotes, returns cp0 into docFtn.
Cp of said reference (or cpMac if none) is returned through pcpRef.
ifnd is available via vifnd.
*/
/* %%Function:CpFirstRef %%Owner:davidlu */
EXPORT CP CpFirstRef(doc, cp, pcpRef, edcDrp)
int doc; 
CP cp, *pcpRef; 
int edcDrp;
{
	int docSub = DocSubEnsure(doc,edcDrp);
	struct PLC **hplcRef;
	struct DRP *pdrp;

	if (docSub == docNil)
		{
		/* all callers currently check for docFtn before calling;
			this could be dangerous otherwise */
		Assert(fFalse);
		vifnd = 0;
		return(cp0);
		}
	Assert(PdodDoc(doc)->fMother);
	pdrp = ((int *)PdodDoc(doc)) + edcDrp;
	hplcRef = pdrp->hplcRef;
	vifnd = IInPlcRef(hplcRef, cp);
	*pcpRef = CpPlc(hplcRef, vifnd);
	return (CpPlc(PdodDoc(docSub)->hplcfnd, vifnd));
}


/* U P D A T E  H P L C P A D */
/* creates if necessary then scans plcpad. Inserts all missing entries
and updates all fUnk1 entries.
*/
/* %%Function:FUpdateHplcpad %%Owner:davidlu */
EXPORT BOOL FUpdateHplcpad(doc)
int doc;
{
	int ipad, dipad;
	struct DOD **hdod = mpdochdod[doc];
	struct PLC **hplcpad;
	struct PAD padNew, pad;
	CP cp, cpPlc, cpMac2Doc;
	extern CP vcpFirstTablePara;
#ifdef WIN
	extern int docSeqCache;

	docSeqCache = docNil;
#endif /* WIN */

	if ((*hdod)->fGlsy)
		return fFalse;

	cpMac2Doc = CpMac2Doc(doc);

	if ((hplcpad = (*hdod)->hplcpad) == 0)
		{
		if ((hplcpad = HplcCreateEdc(hdod, edcPad)) == hNil)
			return fFalse;
		Win((*hplcpad)->fMult = fTrue);
		(*hdod)->fOutlineDirty = fTrue; /* in case premature end */
		}
	else  if (!(*hdod)->fOutlineDirty) return fTrue;

	SetWords(&padNew, 0, sizeof(struct PAD)/sizeof(int));
	padNew.fUnk = fTrue;

	ipad = 0;
	for (ipad = 0, cp = cp0; cp < cpMac2Doc; ipad++)
		{
#ifdef WIN
/* check to see if we are in a segment of text whose paragraph mark is in a 
	table, but which is not itself in a table (because of OPUS fields).  we 
	do not make a pad entry for this piece, skip ahead to the table. */
		if (!FInTableDocCp(doc, cp) && vpapFetch.fInTable)
			cp = vcpFirstTablePara;
#endif /* WIN */

/* make sure entry at ipad starts at cp */
		while (cp != (cpPlc = CpPlc(hplcpad, ipad)))
			{
			if (cpPlc < cp)
				{
/* can happen, e.g. when paras are copied into tables, since table rows
are just one entries each. */
				FOpenPlc(hplcpad, ipad, -1);
				}
			else
				{
/* insert a new entry. Pad will be fully initialized below because its fUnk
bit is set. */
				if (ipad == 0)
					padNew.fShow = fTrue;
				else
					{
					GetPlc(hplcpad, ipad - 1, &pad);
					padNew.fShow = pad.fShow;
					}
				if (!FInsertInPlc(hplcpad, ipad, cp, &padNew))
					{
					/* Wimp out */
					(*hdod)->fOutlineDirty = fTrue;
					(*hdod)->hplcpad = hNil;
					FreeHplc(hplcpad);
					return fFalse;
					}
				}
			}

		GetPlc(hplcpad, ipad, &pad);
/* now we have an entry at ipad with correct cp, ready to be updated */
		if (pad.fUnk || pad.lvl == lvlUpdate)
/* in this entry the boundaries and properties are unknown */
			{
			CachePara(doc, cp);
			/* guaranteed above */
			Assert(!vpapFetch.fInTable == 
					!FInTableDocCp(doc, cp));
			if (vpapFetch.fInTable)
				{
				CpFirstTap(doc, cp);
				CachePara(doc, cp);
				cp = caTap.cpLim;
				}
			else
				cp = caPara.cpLim;
			UpdatePad(hplcpad, ipad, &pad);
			}
		else
/* the start of the next entry is correct */
			cp = CpPlc(hplcpad, ipad + 1);
		if (vmerr.fMemFail)
			{
			/*ppad->fUnk = fTrue;*/
			SetErrorMat( matLow );
			return fFalse;
			}
		}

/* make sure that no bogus entries got pushed to the end of the plc */
	if ((dipad = ipad - IMacPlc(hplcpad)) < 0)
		FOpenPlc(hplcpad, ipad, dipad);
	(*hdod)->fOutlineDirty = fFalse;
	return fTrue;
}


/* U P D A T E  P A D */
/* copy properties from vpapFetch to ppad.
*/
/* %%Function:UpdatePad %%Owner:davidlu */
UpdatePad(hplcpad, ipad, ppad)
struct PLCPAD **hplcpad;
int ipad;
struct PAD *ppad;
{
	int stc = vpapFetch.stc;
	if (stc >= stcLevMin && stc <= stcLevLast)
		{
		ppad->fBody = fFalse;
		ppad->lvl = stcLevLast - stc;
		}
	else
		{
		ppad->fBody = fTrue;
/* neutral lvl value is stored for body text. Effective lvl must
be calculated using LvlFromIpad. */
		ppad->lvl = lvlMax;
		}
	ppad->fUnk = 0;
	if ((ppad->fInTable = vpapFetch.fInTable))
		ppad->lvl = lvlMax;
	ppad->stc = vpapFetch.stc;
	PutPlc(hplcpad, ipad, ppad);
}


/* U P D A T E  H P L C P A D  S I N G L E */
/* new para has been inserted with cpLim. */
/* %%Function:UpdateHplcpadSingle %%Owner:davidlu */
UpdateHplcpadSingle(doc, cpLim)
int doc; 
CP cpLim;
{
	struct PLCPAD **hplcpad;
#ifdef WIN
	struct PLC ** hplcfld;
	extern int docSeqCache;
	docSeqCache = docNil;
#endif /* WIN */

	Assert(cpLim > cp0);
	if (hplcpad = PdodDoc(doc)->hplcpad)
		{
		int ipad = IInPlc(hplcpad, cpLim);
		BOOL fShow;
		struct PAD pad;
		CachePara(doc, cpLim - 1);
		GetPlc(hplcpad, ipad, &pad);
		if (pad.fInTable && cpLim < CpPlc(hplcpad, ipad + 1))
			{
			if (!vpapFetch.fInTable)
				{
				PdodDoc(doc)->fOutlineDirty = fTrue;
				FUpdateHplcpad(doc);
				}
			return;
			}
		FOpenPlc(hplcpad, ipad + 1, 1);

/* this is to help replace of para followed by supressed text */
		fShow = pad.fShow;
		pad.fShow = fTrue;
		PutPlc(hplcpad, ipad + 1, &pad);
		PutCpPlc(hplcpad, ipad + 1, cpLim);
		pad.fShow = fShow;
		UpdatePad(hplcpad, ipad, &pad);
		}
}



/* NOTE: it's usually totally harmless if this fails; style gets mapped to
	stcNormal */
/* %%Function:FEnsureStcDefined %%Owner:davidbo */
FEnsureStcDefined(doc, stc, pcstcStd)
int doc, stc;
int *pcstcStd;
{
	int stcpNew;
	int stcp;
	int stcpMac;
	int fStshDirty = fFalse;
	int fEnsureIndexHeading, fEnsureNormIndent;
	char stNil[1], stZero[1];
	struct ESTCP estcp;
	struct STSH stsh;
	struct STTB **hsttbChpe, **hsttbPape;

	RecordStshForDoc(doc, &stsh, &hsttbChpe, &hsttbPape);
	*pcstcStd = stsh.cstcStd;

	stZero[0] = 0;
	if (stc > stcStdMin)
		{
		fEnsureNormIndent = WinMac((stc >= stcLev9 && stc <= stcLev3), fFalse);
		fEnsureIndexHeading = fFalse;
#ifdef WIN
		if (stc == stcIndexHeading)
			{
			fEnsureIndexHeading = fTrue;
			Assert(stcIndex1 < stcIndexHeading);
			/* stcIndex1 is lower in std stc list than stcIndexHeading.  Add
				std styles from stcIndex1 so recursive call on FEnsure does
				not grow stsh! */
			stc = stcIndex1;
			}
#endif
		if ((cstcMax - stc) > *pcstcStd)
			{
			if (!FAddStdStcsToStsh(doc, stc, pcstcStd))
				return (fFalse);
			PdodDoc(doc)->fStshDirty = fTrue;
			}
		else  if (FStcpEntryIsNull(stsh.hsttbName, stcp = StcpFromStc(stc, *pcstcStd)))
			{
			/* stZero is empty, therefore this is no larger then the string
			that was here already--won't fail.*/
			AssertDo(FChangeStInSttb(stsh.hsttbName, stcp, stZero));
			PdodDoc(doc)->fStshDirty = fTrue;
			}

		Mac( vspop.fSttbIncomplete = fTrue; )
#ifdef WIN
				/* again, changing a slot from 255 to 0 cannot fail */
		if (fEnsureNormIndent)
			{
			AssertDo(FChangeStInSttb(stsh.hsttbName,
					StcpFromStc(stcNormIndent, *pcstcStd), stZero));
			PdodDoc(doc)->fStshDirty = fTrue;
			}
		if (fEnsureIndexHeading)
			{
			AssertDo(FChangeStInSttb(stsh.hsttbName,
					StcpFromStc(stcIndexHeading, *pcstcStd), stZero));
			PdodDoc(doc)->fStshDirty = fTrue;
			}
#endif
		}
	else
		{
		stcpNew = StcpFromStc(stc, *pcstcStd);
		if (stcpNew >= (stcpMac = (*stsh.hsttbName)->ibstMac))
			{
			stNil[0] = 255;
			estcp.stcBase = stcNormal;
			for (stcp = stcpMac; stcp <= stcpNew; stcp++)
				{
				estcp.stcNext = StcFromStcp(stcp, *pcstcStd);
				if (!FInsertInPl(stsh.hplestcp, stcp, &estcp))
					return (fFalse);
				if (!FStretchSttb(stsh.hsttbName, 1, 1) ||
						!FStretchSttb(hsttbChpe, 1, 1) ||
						!FStretchSttb(hsttbPape, 1, 1) ||
						!FStretchSttb(stsh.hsttbChpx, 1, 1) ||
						!FStretchSttb(stsh.hsttbPapx, 1, 1))
					{
					((struct PL *)*stsh.hplestcp)->iMac--;
					return (fFalse);
					}
/* since everything is pre-stretched, the inserts shouldn't fail */
				AssertDo(FInsStInSttb(stsh.hsttbName, stcp, stNil));
				AssertDo(FInsStInSttb(stsh.hsttbChpx, stcp, stNil));
				AssertDo(FInsStInSttb(stsh.hsttbPapx, stcp, stNil));
				AssertDo(FInsStInSttb(hsttbChpe, stcp, stNil));
				AssertDo(FInsStInSttb(hsttbPape, stcp, stNil));
				}
			}
		if (FStcpEntryIsNull(stsh.hsttbName, stcpNew))
			{
			FChangeStInSttb(stsh.hsttbName, stcpNew, stZero);
LDirtyStsh:
			PdodDoc(doc)->fStshDirty = fTrue;
			}
		}
	return fTrue;
}


/* F  A D D  S T D  S T C S  T O  S T S H */
/* %%Function:FAddStdStcsToStsh %%Owner:davidbo */
FAddStdStcsToStsh(doc, stc, pcstcStd)
int doc;
int stc;
int *pcstcStd;
{
	int cstc;
	int cbIncr;
	int cb;
	int stcT;
	struct STSH stsh;
	struct STTB **hsttbChpe, **hsttbPape;
	struct PL *ppl;
	char stNil[1], stZero[1];
	struct ESTCP estcp;

	Assert(stc > stcStdMin && stc <= WinMac(stcNormIndent, stcLevLast));

	RecordStshForDoc(doc, &stsh, &hsttbChpe, &hsttbPape);
	cstc = (cstcMax - stc) - stsh.cstcStd;
	if (cstc <= 0)
		return fFalse;

	stNil[0] = 0377;
	stZero[0] = 0;
	estcp.stcBase = stcNormal;
	if (pcstcStd)
		*pcstcStd = (PdodDoc(doc))->stsh.cstcStd;

	for (stc = 0, stcT = (-stsh.cstcStd - 1) & 255; stc < cstc; stc++, stcT--)
		{
		switch (stcT)
			{
#ifdef WIN
		case stcAtnText:
#endif
		case stcFtnText:
		case stcHeader:
		case stcFooter:
			estcp.stcNext = stcT;
			break;
#ifdef WIN
		case stcIndexHeading:
			estcp.stcNext = stcIndex1;
			break;
#endif
		default:
#ifdef WIN
			if (stcT <= stcLev3 && stcT >= stcLev9 || stcT == stcNormIndent)
				estcp.stcNext = stcNormIndent;
			else
#endif
				estcp.stcNext = stcNormal;
			}

		if (!FInsertInPl(stsh.hplestcp, 0, &estcp))
			return (fFalse);
		if (!FStretchSttb(stsh.hsttbName, 1, 1) ||
				!FStretchSttb(hsttbChpe, 1, 1) ||
				!FStretchSttb(hsttbPape, 1, 1) ||
				!FStretchSttb(stsh.hsttbChpx, 1, 1) ||
				!FStretchSttb(stsh.hsttbPapx, 1, 1))
			{
			((struct PL *)*stsh.hplestcp)->iMac--;
			return (fFalse);
			}

/* since we've pre-stretched the STTBs we should not fail */
		FInsStInSttb(stsh.hsttbName, 0, (stc + 1 < cstc) ? stNil : stZero);
		FInsStInSttb(hsttbChpe, 0, stNil);
		FInsStInSttb(hsttbPape, 0, stNil);
		FInsStInSttb(stsh.hsttbChpx, 0, stNil);
		FInsStInSttb(stsh.hsttbPapx, 0, stNil);
		(PdodDoc(doc))->stsh.cstcStd += 1;
		if (pcstcStd)
			*pcstcStd += 1;
		}
	return fTrue;
}


/* %%Function:RecordStshForDoc %%Owner:davidbo */
RecordStshForDoc(doc, pstsh, phsttbChpe, phsttbPape)
int doc;
struct STSH *pstsh;
struct STTB ***phsttbChpe;
struct STTB ***phsttbPape;
{
	struct DOD *pdod = PdodDoc(doc);

	bltb(&pdod->stsh, pstsh, cbSTSH);
	*phsttbChpe = pdod->hsttbChpe;
	*phsttbPape = pdod->hsttbPape;
}



#ifdef WIN
#ifdef PROFILE
/*  this is here so that appended native code does not appear in previous
	function in pcode profiles. */
Fetch1_Last(){}
#endif /* PROFILE */
#endif /* WIN */

/* ADD NEW CODE *ABOVE* Fetch1_Last() */
