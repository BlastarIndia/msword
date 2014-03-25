/* F E T C H . C */

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "doc.h"
#include "file.h"
#include "props.h"
#include "fkp.h"
#include "ch.h"
#include "prm.h"
#include "disp.h"
#include "debug.h"
#include "border.h"
#include "error.h"
#ifdef WIN
#include "field.h"
#include "format.h"
#include "rareflag.h"
#endif /* WIN */

/* E X T E R N A L S */

extern BOOL             vfEndFetch;
extern struct DOD       **mpdochdod[];
extern struct CHP       vchpFetch;
extern struct CHP       vchpStc;
extern struct FKPD      vfkpdChp;
extern int		vdocTemp;
Mac(extern struct PLB	vplb);  /* page limit block */

#ifdef MAC
extern uns              *pfgrMac;
extern BYTE             mpUcLc[], mpLcUc[];
/* the value of ibUcLc depends on the array "mpLcUc" don't change one
/* without the other. This define must also be maintained in fetch.s
/**/
#define                 ibUcLcReverse	20
#endif

#ifdef WIN 
#undef FetchCp
#undef CachePara
#undef ApplyPrlSgc
#undef PnFromPlcbteFc
#undef BFromFc
#undef MapStc
#undef HpchFromFc
#undef HpchGetPn
#undef IbpCacheFilePage
#undef DoPrmSgc
#define FetchCp(doc,cp,fcm) \
		C_FetchCp(doc,cp,fcm)
#define CachePara(doc,cp) \
		C_CachePara(doc,cp)
#define ApplyPrlSgc(pprl, cch, prgbProps, sgc, fWord3File) \
		C_ApplyPrlSgc(pprl, cch, prgbProps, sgc)
#define PnFromPlcbteFc(hplcbte, fc) \
		C_PnFromPlcbteFc(hplcbte, fc)
#define BFromFc(hpfkp, fc, pfcFirst, pfcLim, pifc, fFkpWord3) \
		C_BFromFc(hpfkp, fc, pfcFirst, pfcLim, pifc)
#define MapStc(pdod, stc, pchp, ppap) \
		C_MapStc(pdod, stc, pchp, ppap)
#define HpchFromFc(fn, fc) \
		C_HpchFromFc(fn, fc)
#define HpchGetPn(fn, pn) \
		C_HpchGetPn(fn, pn)
#define IbpCacheFilePage(fn, pn) \
		C_IbpCacheFilePage(fn, pn)
#define DoPrmSgc(prm, prgbProps, sgc) \
		C_DoPrmSgc(prm, prgbProps, sgc)
#define ApplyChpxTransforms(pchp) \
		C_ApplyChpxTransforms(pchp)
PN		       C_PnFromPlcbteFc();
CHAR  HUGE	       *HpBaseForIbp();
extern uns	       *pfgrMac;
extern struct RF	vrf;
#ifdef DEBUG
extern BOOL vfInCacheParaVerify;
#endif /* DEBUG */
#endif /* WIN */

extern int              vdocFetch;
extern CP               vcpFetch;
extern int              vccpFetch;
extern int              vdocPapLast;
extern int              vfnPapLast;
extern int              vpnPapLast;
extern int              vbpapxPapLast;
extern int              vstcLast;
extern int              vcchPapxLast;
extern CHAR             rgchInsert[];
extern int              ichInsert;
extern CHAR 		rgchEop[];
extern struct FCB       **mpfnhfcb[];
extern struct CA        caPara;
extern struct CA	caTap;
extern struct FKPD      vfkpdPap;
extern struct CA        caSect;
extern int              vised;
extern struct CA        caPage;
extern int              vipgd;
extern struct SAB       vsab;
extern struct CHP       vchpStc;
extern struct PAP       vpapStc;
extern struct TAP       vtapStc;
extern struct PAP       vpapFetch;
extern struct SEP       vsepFetch;
extern struct TAP       vtapFetch;
extern struct TCC       vtcc;
extern CP               vmpitccp[];
extern int              vitcMic;
extern int              vibp;
extern int              vibpProtect;
extern struct BPTB      vbptbExt;
extern FC               fcFetch;
extern int              fcmFetch;
extern CHAR HUGE       *vhpchFetch;
extern CP               ccpChp;
extern CP               ccpPcd;
extern int              ccpFile;
extern int              ipcdFetch;
extern int              fnFetch;
extern int              prmFetch;
extern CHAR             rgchCaps[];
extern int              vstcpMapStc;
extern struct ESPRM     dnsprm[];
#ifdef DEBUG
extern struct DBS       vdbs;
struct FKPC
	{
	int pn;
	int ifc;
};

#ifdef NOTUSED
struct FKPC mpfnfkpcChp[fnMax];
struct FKPC mpfnfkpcPap1[fnMax];
struct FKPC mpfnfkpcPap2[fnMax];
#endif 

int vcFetchCp = 0;
int vcFetchChp = 0;
int vcFetchSameChp = 0;
#ifdef NOTUSED
int vcFetchChpPlus1 = 0;
#endif

int vcCachePara = 0;
int vcFetchPap1 = 0;
int vcPap1Minus1 = 0;
int vcPap1Same = 0;
int vcFetchPap2 = 0;
int vcPap2Same = 0;
int vcPap2Plus1 = 0;
#ifdef NOTUSED
int vcPap2EqPap1Plus1 = 0;
int vcPap1EqPap2Plus1 = 0;
#endif
int vcPapStcUnchanged = 0;
int vcStyleUnchanged = 0;

#endif


/* F E T C H  C P */
/*
Basic procedure for random or sequential access to documents

Inputs:
	doc, if docNil the call is sequential
	starting cp
	fcm tells whether to get chars, props, (both) and whether to parse
		caps/small caps and word underlining.
Outputs:
	vcpFetch        starting cp
	vdocFetch       doc
	vccpFetch       number of ch's fetched
	vhpchFetch      points to first character fetched, others follow
	vchpFetch       char prop of fetched chars
	vfEndFetch      fTrue iff end of document is reached
The output variables must not be disturbed.
*/

#ifdef DEBUGORNOTWIN
/* %%Function:FetchCp %%Owner:davidlu */
HANDNATIVE FetchCp(doc, cp, fcm)
int doc, fcm;
CP cp;
{
	CP ccpMin;
	struct DOD *pdod;

#ifdef WIN
	vrf.fInFetchCp = !vrf.fInFetchCp;
	Assert (vrf.fInFetchCp);
	if (!vrf.fInFetchCp)
		{
		vrf.fInFetchCp = !vrf.fInFetchCp;
		return;
		}
#endif /* WIN */
	Debug(vcFetchCp++);
	if (doc == docNil)
		goto LSequential;
	Assert(cp < CpMacDoc(doc));
	if (doc == vdocFetch && cp <= vcpFetch + vccpFetch &&
			cp >= vcpFetch && fcm == fcmFetch)
		{
		doc = docNil;
		vccpFetch = cp - vcpFetch;
/* Sequential call */
LSequential:
		Assert(vcpFetch != cpNil);

		vcpFetch += vccpFetch;  /* Go to where we left off */
		fcFetch += vccpFetch;
		}
	else
		{
/* Random-access call */
		vcpFetch = cp;
		vdocFetch = doc;
		ccpChp = ccpPcd = ccpFile = 0;
		fcmFetch = fcm;
		}

	pdod = *mpdochdod[vdocFetch];
	if (vcpFetch >= CpMacDoc(vdocFetch))
		{
		vfEndFetch = fTrue;
#ifdef WIN
		vrf.fInFetchCp = !vrf.fInFetchCp;
#endif /* WIN */
		return;
		}
	vfEndFetch = fFalse;

/* determine if current piece has more text in it or if new piece has to be
looked at */
	if (ccpPcd > vccpFetch)
		ccpPcd -= vccpFetch;
	else
		{
		struct PLC **hplcpcd = pdod->hplcpcd;
		struct PCD pcd;

		if (doc == docNil)
			++ipcdFetch; /* Save some work on sequential call */
		else
			/* Search for piece and remember index for next time */
			ipcdFetch = IInPlc(hplcpcd, vcpFetch);
		GetPlc(hplcpcd, ipcdFetch, &pcd);
		ccpPcd = CpPlc(hplcpcd, ipcdFetch + 1) - vcpFetch;
		Assert(ccpPcd > cp0);
/* Invalidate other ccp's since new piece */
		ccpChp = ccpFile = 0;
		fcFetch = pcd.fc + vcpFetch - CpPlc(hplcpcd, ipcdFetch);
		fnFetch = pcd.fn;
		prmFetch = pcd.prm;
		}

	ccpMin = ccpPcd; /* prepare for calculating next vccpFetch */
	if (fcm & fcmChars)
		{
		if (ccpFile > vccpFetch)
			{
			ccpFile -= vccpFetch;
			vhpchFetch += vccpFetch;
			}
		else
/* get hold of characters */
			{
			if (fnFetch == fnSpec)
				{
				int ich;
				if ((ich = (int)fcFetch) >= cchInsertMax)
					{
/* special pieces for out of memory fill and for an end of document Eop */
					if (ich == (int)fcSpecEop)
						{
						bltb(rgchEop, rgchCaps,
								(uns)(ccpFile=ccpEop));
						}
#ifdef CRLF
					else  if (ich == (int)fcSpecEop + 1)
						{
						rgchCaps [0] = chEop;
						ccpFile = 1;
						}
#endif	/* CRLF */
					else
						{
						SetBytes(rgchCaps, '*', ichCapsMax);
						ccpFile = (CP)ichCapsMax;
						}
					vhpchFetch = (CHAR HUGE *) &rgchCaps[0];
					}
				else
/* Special quick and dirty insert mode. */
					{
					vhpchFetch = (CHAR HUGE *)&rgchInsert[ich];
					ccpFile = cchInsertMax - ich;
					}
				}
			else
				{
#ifdef MAC
				if (vplb.fnDest != fnNil && ccpPcd >= cbSector)
					vplb.fNoTsNonDest = fTrue;
#endif
/* No monkeying with files after this statement, or we may page out */
				vhpchFetch = HpchFromFc(fnFetch, fcFetch); /* Read in buffer */
				Mac(vplb.fNoTsNonDest = fFalse);
				vibpProtect = vibp;
				ccpFile = cbSector - (fcFetch & maskSector);
				}
			}
		if ((CP)ccpFile < ccpMin) ccpMin = ccpFile;
		Assert( ccpMin <= cbSector );
		}

	if (fcm & fcmProps)
		{ /* There must be enough page buffers so that this will not
			page out vhpchFetch! */
		if (ccpChp > vccpFetch)
			ccpChp -= vccpFetch;
		else
			{
/* Fill vchpFetch with char props; length of run to ccpChp */
			struct FKP HUGE *hpfkp;
			struct FCB *pfcb;
			struct CHP *pchp;

			FreezeHp();

			blt(&vchpStc, &vchpFetch, cwCHP);
			if (fnFetch == fnSpec)
				{
				if ((int)fcFetch >= cchInsertMax)
					{
					ccpChp = cpMax;
					}
				else
					{
					ApplyChpxTransforms(&vfkpdChp.chp);
/* ichInsert points to first "vanished" character in the rgchInsert array */
					if (ichInsert <= (int)fcFetch)
						{
						/* in the vanished region */
						vchpFetch.fVanish = fTrue;
						vchpFetch.fSysVanish = fTrue;
						ccpChp = cpMax;
						}
					else
						ccpChp = ichInsert - (int)fcFetch;
					}
				}
			else  if (fnFetch == fnScratch
					&& fcFetch >= vfkpdChp.fcFirst)
				{
				ApplyChpxTransforms(&vfkpdChp.chp);
				ccpChp = cpMax;
				}
			else
				{
/* ensure current para */
				Assert(caPara.doc == vdocFetch &&
						caPara.cpFirst <= vcpFetch &&
						caPara.cpLim > vcpFetch);
/* initial estimate of size of CHP run. will be revised if piece not from
	text file. */
				ccpChp = ccpPcd;

				pfcb = *(mpfnhfcb[fnFetch]);
/* note that the scratch file FCB has the fHasFib flag on as an expedient lie
	which allows us to recognize that the scratch file has PLCBTEs also. */
				if (pfcb->fHasFib)
					{ /* Copy necessary amt of formatting info over std CHP */
					int pn;
					FC HUGE *hpfc;
					FC fcMin, fcMac;
#ifdef MAC
					int fWord3File = pfcb->fWord3File;
#endif /* MAC */
					int crun;
					int bchpx;
					struct CHP chpX;
					Debug(vcFetchChp++);
					pn = PnFromPlcbteFc(pfcb->hplcbteChp, fcFetch);
#ifdef MAC
					if (vplb.fnDest != fnNil && ccpPcd >= cbSector)
						vplb.fNoTsNonDest = fTrue;
					hpfkp = (struct FKP HUGE *)(!fWord3File ?
							HpchGetPn(fnFetch, pn) :
							HpchGetPo(fnFetch, (PO) pn));
#else /* WIN */
					hpfkp = (struct FKP HUGE *) HpchGetPn(fnFetch, pn);
#endif /* WIN */
					Mac(vplb.fNoTsNonDest = fFalse);
					if ((pn == pfcb->pnChpHint && pfcb->ifcChpHint != ifcFkpNil &&
							*(hpfc = &hpfkp->rgfc[pfcb->ifcChpHint]) <= fcFetch) &&
							fcFetch < *(hpfc + 1))
						{
						fcMac = *(hpfc + 1);
						crun = hpfkp->crun;
#ifdef MAC
						if (fWord3File)
							crun = ((struct FKPO HUGE *)hpfkp)->crun;
#endif /* MAC */
						bchpx = ((CHAR HUGE *)&((hpfkp->rgfc)[crun+1]))
								[pfcb->ifcChpHint];
#ifdef MAC
						if (!fWord3File)
#endif /* MAC */
							bchpx = bchpx << 1;
						Debug(vcFetchSameChp++);
						}
					else
						{
						/* fcMin not used */
						bchpx = BFromFc(hpfkp, fcFetch, &fcMin/*nil*/, &fcMac, &pfcb->ifcChpHint,
								 pfcb->fWord3File);
						pfcb->pnChpHint = pn;
						}
					ccpChp = fcMac - fcFetch;
					if (bchpx != 0/*nil*/)
						{
						CHAR HUGE *hpchpx = &((CHAR HUGE *)hpfkp)[bchpx];
						int cch = *hpchpx++;
						SetWords(&chpX, 0, cwCHP);
						bltbh(hpchpx, &chpX, cch);
						/* "apply" transformations expressed in the chpx */
						ApplyChpxTransforms(&chpX);
						}
					}
				}
/* limit scope of to min of paragraph end or run end */
			if (ccpChp > caPara.cpLim - vcpFetch)
				ccpChp = caPara.cpLim - vcpFetch;

			if (prmFetch != prmNil)
				DoPrmSgc(prmFetch, &vchpFetch, sgcChp);
			MeltHp();
			}

		if (ccpChp < ccpMin) ccpMin = ccpChp;
		}

/* convert vccpFetch to an integer and prevent larger than 15 bit reach.
(possible if fcmChar is not set!)
*/
	vccpFetch = (ccpMin >= 0x7FFF) ? 0x7FFF : (int)ccpMin;
	Assert(vccpFetch > 0);
	Assert( !(fcm & fcmChars) || vccpFetch <= cbSector );

#ifndef JR
	if ((fcm & fcmParseCaps) != 0)
		{
		CHAR HUGE *hpch;
		CHAR HUGE *hpchFetch;
		int cch = vccpFetch - 1;
		hpch = hpchFetch  = vhpchFetch;
/* first check for word underline */
		if (vchpFetch.kul == kulWord)
			{
			int ch = *hpch++;
			if (ch == chSpace || ch == chTab || ch == chNonBreakSpace)
				{
				while (cch-- != 0 &&
						(*hpch == chSpace ||
						*hpch == chTab || *hpch == chNonBreakSpace)) hpch++;
				vchpFetch.kul = 0;
				}
			else
				{
				while (cch-- != 0 &&
						(*hpch != chSpace &&
						*hpch != chTab && *hpch != chNonBreakSpace)) hpch++;
				vchpFetch.kul = kulSingle;
				}
			vccpFetch = hpch - hpchFetch;
			ccpChp = 0;
			}
		hpch = hpchFetch;
		cch = vccpFetch - 1;
		if (vchpFetch.fSmallCaps || vchpFetch.fCaps)
			{
/* run length will be limited to min(ichCapsMax, vccpFetch).
If small caps: if chars are already caps, further limit length to run of caps
and we are done. If LC chars need to change, change chp and copy UC versions
to rgchCaps. If fCaps, just copy UC versions.
*/
			if (vchpFetch.fCaps)
				{
				CHAR *pch;
LCopyCaps:
				vccpFetch = min(vccpFetch, ichCapsMax);
				for (cch = 0, pch = &rgchCaps[0]; cch < vccpFetch; cch++)
					*pch++ = ChUpper(*hpchFetch++);

				vhpchFetch = (CHAR HUGE *)&rgchCaps[0];
				ccpFile = 0; /* since hpch is changed */
				}
			else
				{
				if (!FLower(*hpch++))
					{
/* text is already in upper case */
					while (cch-- != 0 && !FLower(*hpch))
						hpch++;
					vccpFetch = hpch - hpchFetch;
					}
				else
					{
/* text is in lower case, change size and case */
					while (cch-- != 0
						&& (FLower(*hpch) || *hpch == chSpace))
						hpch++;
					vccpFetch = hpch - hpchFetch;
/* mapping of point size for small caps: */
					vchpFetch.hps = HpsAlter(vchpFetch.hps, -1);
					ccpChp = 0; /* since chp is changed */
					goto LCopyCaps;
					}
				}
			}
		}
#else /* #ifdef JR */
	JuniorChp();
#endif /* JR */
#ifdef WIN
	vrf.fInFetchCp = !vrf.fInFetchCp;
#endif /* WIN */
}


#endif /* DEBUGORNOTWIN */


#ifdef DEBUGORNOTWIN
/* A P P L Y  C H P X  T R A N S F O R M S */
/* %%Function:ApplyChpxTransforms %%Owner:davidlu */
HANDNATIVE ApplyChpxTransforms(pchpX)
struct CHP *pchpX;
{
	*(int *)&vchpFetch ^= *(int *)pchpX;
	if (*(int *)pchpX & (maskFs | maskFSpec))
		{
		if (pchpX->fsFtc) vchpFetch.ftc = pchpX->ftc;
		if (pchpX->fsHps) vchpFetch.hps = pchpX->hps;
		if (pchpX->fsKul) vchpFetch.kul = pchpX->kul;
		if (pchpX->fsPos) vchpFetch.hpsPos = pchpX->hpsPos;
		if (pchpX->fsSpace) vchpFetch.qpsSpace = pchpX->qpsSpace;
		if (pchpX->fsIco) vchpFetch.ico = pchpX->ico;
		if (pchpX->fSpec) vchpFetch.fcPic = pchpX->fcPic;
		}
}


#endif /* DEBUGORNOTWIN */


#ifdef DEBUGORNOTWIN
/* P N  F R O M  P L C B T E  F C */
/* looks up fc in plcbte. Does simply: PInPlc(plc, Iplc(plc, fc))->pn.
*/
/* %%Function:PnFromPlcbteFc %%Owner:davidlu */
HANDNATIVE PN PnFromPlcbteFc(hplcbte, fc)
struct PLC **hplcbte;
FC fc;
{
	struct BTE bte;

	Assert(IMacPlc(hplcbte) > 0);
	Assert(fc >= CpPlc(hplcbte, 0) &&
			fc < CpPlc(hplcbte, IMacPlc(hplcbte)));
	GetPlc(hplcbte, IInPlc(hplcbte, fc), &bte);
	return (bte.pn);
}


#endif /* DEBUGORNOTWIN */


#ifdef MAC  /* in fetchn2.asm if WIN */
/* C A C H E  P A R A  C A */
/* alternative entry point */
/* %%Function:CacheParaCa %%Owner:davidlu */
HANDNATIVE CacheParaCa(pca)
struct CA *pca;
{
	CachePara(pca->doc, pca->cpFirst);
}


#endif	/* MAC */


#ifdef DEBUGORNOTWIN
/* C A C H E  P A R A */
/* returns para props (style + papx in file + prm in piece table) in
	vpapFetch
and paragraph boundaries in
	caPara
*/
/* %%Function:CachePara %%Owner:davidlu */
HANDNATIVE CachePara(doc, cp)
int doc;
CP cp;
{
	int ipcdBase, ipcd,  prm;
	CP cpGuess;
	CP ccpPcd;
	struct DOD *pdod;
	struct PLC **hplcpcd;
	struct PCD pcd;

	Debug(vcCachePara++);
	if (FInCa(doc, cp, &caPara)) return;

	FreezeHp();
	pdod = *mpdochdod[doc];
	Assert(cp >= cp0 && cp < CpMac2Doc(doc));
	hplcpcd = pdod->hplcpcd;
	ipcdBase = IInPlc(hplcpcd, cpGuess = cp);

	if (caPara.doc == doc && cp == caPara.cpLim)
		/* looking for the beginning of the paragraph following
		the one in the cache */
		caPara.cpFirst = cp;
	else
		{
/* Search backward to find para start.
We ask each piece in turn: do you have a para end in the interval (cpMin, cpGuess)?
with cpGuess initially = cp, and = cpMin of the next piece for the others.
*/
		for (ipcd = ipcdBase; ; --ipcd)
			{ /* Beware heap movement! */
			int fn;
			CP cpMin = CpPlc(hplcpcd, ipcd);

			GetPlc(hplcpcd, ipcd, &pcd);
			ccpPcd = CpPlc(hplcpcd, ipcd + 1) - cpMin;
			fn = pcd.fn;
			if (!pcd.fNoParaLast)
				{ /* Don't check if we know there's no para end */
				FC fcMin = pcd.fc;
				FC fc, fcFirst;
				struct FCB *pfcb;

				fcFirst = fc = fcMin + cpGuess - cpMin;
/* calculate the fcFirst after the latest para end before fc.
If there is no Eop at fc' in [fcMin, fc), goto NoParaEnd1. */
				if (cpGuess == cpMin)
					goto LNoParaEnd1;
				if (fn == fnSpec/*fnInsert*/)
					{
					if ((int)fcMin == fcSpecEop)
						{
						Assert( CpPlc( hplcpcd, ipcd + 1 ) == cpMin + ccpEop );
#ifdef CRLF
						if (fc > fcSpecEop + 1)
#endif
							goto LParaStart;
						}
					goto LNoParaEnd1;
					}

				pfcb = *(mpfnhfcb[fn]);
/* note that the scratch file FCB has the fHasFib flag on as an expedient lie
	which allows us to recognize that the scratch file has PLCBTEs also. */
				if (!pfcb->fHasFib)
					{ /* Unformatted file; scan for an EOL */
					PN pn;
					FC fcFirstPage;

					fcFirstPage = ((fc - 1) >> shftSector) << shftSector;
					pn = fcFirstPage >> shftSector;

					while (fc > fcMin)
						{
						CHAR HUGE *hpch;

						hpch = HpchGetPn(fn, pn--) + (int)(fc - fcFirstPage);
						if (fcMin > fcFirstPage)
							fcFirstPage = fcMin;
						while (fc > fcFirstPage)
							{
							if (*(--hpch) == chEol)
								{
								fcFirst = fc;
#ifdef DEBUG
								pcd.fNoParaLastValid = fTrue;
#ifdef WIN
								if (!vfInCacheParaVerify)
#endif /* WIN */
									PutPlcLast(hplcpcd, ipcd, &pcd);
#endif /* DEBUG */
								goto LParaStart;
								}
							fc--;
							}
						fcFirstPage -= cbSector;
						}
					}
				else
					{ /* Formatted file; get info from para run */
					int pn;
					int ifc, ifcT;
					FC HUGE *hpfc;
					struct FKP HUGE *hpfkp;
					FC fcLim, fcFkpLim;
					int dpn;
					struct PLC **hplcbte = pfcb->hplcbtePap;
/* this if takes care of the case if fc is beyond the last Eop in the file.
	the case occurs when writing FKPs to the scratch file and during quicksave. */
					if (fc >= (fcFkpLim = (FC) CpMacPlc(hplcbte)))
						{
/* para bound at lim fc of the plcbte. */
						if (fcMin < (fcFirst = fcFkpLim))
							goto LParaStart;
						goto LNoParaEnd1;
						}
					Debug(vcFetchPap1++);
					pn = PnFromPlcbteFc(hplcbte, fc);
#ifdef MAC
					if (vplb.fnDest != fnNil && ccpPcd >= cbSector)
						vplb.fNoTsNonDest = fTrue;
					hpfkp = (struct FKP HUGE *)(!pfcb->fWord3File ?
							HpchGetPn(fn, pn) :
							HpchGetPo(fn, (PO) pn));
#else /* WIN */
					hpfkp = (struct FKP HUGE *) HpchGetPn(fn, pn);
#endif /* WIN */
					Mac(vplb.fNoTsNonDest = fFalse);
					ifc = pfcb->ifcPapHint1;
					if (ifc != ifcFkpNil && pn == pfcb->pnPapHint1 &&
							(((ifc > 1 && *(hpfc = &hpfkp->rgfc[ifcT = ifc - 1]) <= fc) &&
							fc < *(hpfc + 1)) ||
							((*(hpfc = &hpfkp->rgfc[ifcT = ifc]) <= fc) && fc < *(hpfc + 1))))
						{
						fcFirst = *hpfc;
#ifdef DEBUG
						if (ifc == ifcT)
							vcPap1Same++;
						else  if (ifc - 1 == ifcT)
							vcPap1Minus1++;
						else  
							Assert(fFalse);
#endif /* DEBUG */
#ifdef DEBUG
#ifdef WIN
						if (!vfInCacheParaVerify)
#endif /* WIN */
#endif /* DEBUG */
							pfcb->ifcPapHint1 = ifcT;
						}
					else
						{
#ifdef DEBUG
#ifdef WIN
						int ifcSav = pfcb->ifcPapHint1;

#endif /* WIN */
#endif /* DEBUG */
						BFromFc(hpfkp, fc, &fcFirst, &fcLim/*not used*/, &pfcb->ifcPapHint1, pfcb->fWord3File);
#ifdef DEBUG
#ifdef WIN
						if (vfInCacheParaVerify)
							pfcb->ifcPapHint1 = ifcSav;
						if (!vfInCacheParaVerify)
#endif /* WIN */
#endif /* DEBUG */
							pfcb->pnPapHint1 = pn;
						}
					if (fcMin >= fcFirst) goto LNoParaEnd1;
/* Found para start */
LParaStart:                             
					caPara.cpFirst = cpMin + (fcFirst - fcMin);
					Debug(pcd.fNoParaLastValid = fTrue);
#ifdef DEBUG
#ifdef WIN
					if (!vfInCacheParaVerify)
#endif /* WIN */
#endif /* DEBUG */
						PutPlc(hplcpcd, ipcd, &pcd);
					break;
					}
				}
LNoParaEnd1:
/* Now we know there's no para end from cpMin to cpGuess.
If original piece, there may be one after cp == cpGuess */
			if (ipcd != ipcdBase)
				{
				/* not original piece, save knowledge */
				Debug(pcd.fNoParaLastValid = fTrue);
				pcd.fNoParaLast = fTrue;
#ifdef DEBUG
#ifdef WIN
				if (!vfInCacheParaVerify)
#endif /* WIN */
#endif /* DEBUG */
					PutPlc(hplcpcd, ipcd, &pcd);
				}
			if (cpMin == cp0)
				{ /* Beginning of doc is beginning of para */
				caPara.cpFirst = cp0;
				break;
				}
			cpGuess = cpMin;
			}
		}

	caPara.doc = doc;
/* Now go forward to find the cpLim of the para */
	cpGuess = cp;

	for (ipcd = ipcdBase; ; ++ipcd)
		{
		CP cpMin = CpPlc(hplcpcd, ipcd);
		CP cpLim = CpPlc(hplcpcd, ipcd + 1);
		FC fc;
		int fn;

		GetPlc(hplcpcd, ipcd, &pcd);
		fn = pcd.fn;

		if (!pcd.fNoParaLast)
			{ /* Don't check if we know there's no para end */
			struct FCB *pfcb;
			struct PLC **hplcbtePap;
			FC fcMin = pcd.fc;
			FC fcMac, fcLim;
			if (fn == fnSpec && (int)fcMin == fcSpecEop)
				{ /* special eop piece */
				caPara.cpLim = cpMin + ccpEop;
				if (doc != vdocPapLast || fn != vfnPapLast ||
						vbpapxPapLast != cbSector ||
						vstcLast == stcNil)
					{
					if (doc != vdocPapLast ||
							vstcLast != stcNormal ||
							vcchPapxLast > 0)
						{
						MapStc(pdod, stcNormal, &vchpStc, &vpapStc);
						vstcLast = stcNormal;
						vcchPapxLast = 0;
						}
Debug(else  
	vcStyleUnchanged++);
SetWords(&vpapStc.phe, 0, cwPHE);
vdocPapLast = doc;
vfnPapLast = fn;
vbpapxPapLast = cbSector;
					}
#ifdef DEBUG
				else  
					vcPapStcUnchanged++;

				pcd.fNoParaLastValid = fTrue;
#ifdef WIN
				if (!vfInCacheParaVerify)
#endif /* WIN */
					PutPlcLast(hplcpcd, ipcd, &pcd);
#endif
				break;
				}

			fc = fcMin + cpGuess - cpMin;
			fcMac = fcMin + cpLim - cpMin;
/* Calculate the fcLim after the first para end after or at fc.  If there
is no para end in [fc, fcMac), goto LNoParaEnd2.  Also get paragraph
properties in vpapStc.  Leave normal chp's in vchpStc. */
			if (fn == fnInsert)
				goto LNoParaEnd2;

			pfcb = *mpfnhfcb[fn];
/* note that the scratch file FCB has the fHasFib flag on as an expedient lie
	which allows us to recognize that the scratch file has PLCBTEs also. */
			if (!pfcb->fHasFib)
				{ /* Unformatted file; scan for EOL */
				PN pn;
				FC fcFirstPage;
				if (vdocPapLast != doc || vfnPapLast != fn ||
						vbpapxPapLast != cbSector ||
						vstcLast == stcNil)
					{
					if (doc != vdocPapLast ||
							vstcLast != stcNormal ||
							vcchPapxLast > 0)
						{
						MapStc(pdod, stcNormal, &vchpStc, &vpapStc);
						vstcLast = stcNormal;
						vcchPapxLast = 0;
						}
Debug(else  
	vcStyleUnchanged++);

SetWords(&vpapStc.phe, 0, cwPHE);
vdocPapLast = doc;
vfnPapLast = fn;
vbpapxPapLast = cbSector;
					}
Debug(else  
	vcPapStcUnchanged++);
fcFirstPage = (fc >> shftSector) << shftSector;
pn = fcFirstPage >> shftSector;

while (fc < fcMac)
{
	CHAR HUGE *hpch;

	hpch = (CHAR HUGE *) (HpchGetPn(fn, pn++)) + (int)(fc - fcFirstPage);
	if ((fcFirstPage += cbSector) > fcMac)
		fcFirstPage = fcMac;
	while (fc < fcFirstPage)
		{
		fc++;
		if (*hpch++ == chEol)
			{
			fcLim = fc;
#ifdef DEBUG
			pcd.fNoParaLastValid = fTrue;
#ifdef WIN
			if (!vfInCacheParaVerify)
#endif /* WIN */
				PutPlcLast(hplcpcd, ipcd, &pcd);
#endif
			goto LParaEnd2;
			}
		}
}


				}
			else
/* Formatted file; get info from para run */
				{
				int pn;
				int ifc, ifcT;
				FC HUGE *hpfc;
				struct FKP HUGE *hpfkp;
				struct FPAP *pfpap;
				FC fcFirst;
				int bpapx;
#ifdef MAC
				int fWord3File = pfcb->fWord3File;
#endif /* MAC */
				int nFib = pfcb->nFib;
				int crun;

				hplcbtePap = pfcb->hplcbtePap;
				if (fc >= (FC) CpMacPlc(hplcbtePap))
					goto LNoParaEnd2;
#ifdef DEBUG
				vcFetchPap2++;
				pcd.fNoParaLastValid = fTrue;
#ifdef WIN
				if (!vfInCacheParaVerify)
#endif /* WIN */
					PutPlcLast(hplcpcd, ipcd, &pcd);
#endif

				pn = PnFromPlcbteFc(hplcbtePap, fc);
#ifdef MAC
				if (vplb.fnDest != fnNil && (cpLim - cpMin) >= cbSector)
					vplb.fNoTsNonDest = fTrue;
				hpfkp = (struct FKP HUGE *)(!fWord3File ?
						HpchGetPn(fn, pn) :
						HpchGetPo(fn, (PO) pn));
#else /* WIN */
				hpfkp = (struct FKP HUGE *) HpchGetPn(fn, pn);
#endif /* WIN */
				Mac(vplb.fNoTsNonDest = fFalse);
				ifc = pfcb->ifcPapHint2;
				crun = hpfkp->crun;
#ifdef MAC
				if (fWord3File)
					crun = ((struct FKPO HUGE *)hpfkp)->crun;
#endif /* MAC */
				if (ifc != ifcFkpNil && pn == pfcb->pnPapHint2 &&
						(((ifc < crun - 1 &&
						*(hpfc = &hpfkp->rgfc[ifcT = ifc + 1]) <= fc) &&
						fc < *(hpfc + 1)) ||
						((*(hpfc = &hpfkp->rgfc[ifcT = ifc]) <= fc) && fc < *(hpfc + 1))))
					{
					fcFirst = *hpfc;
					fcLim = *(hpfc + 1);
					bpapx = ((CHAR HUGE *)&((hpfkp->rgfc)[crun + 1]))[ifcT];
#ifdef MAC
					if (!fWord3File)
#endif /* MAC */
						bpapx = bpapx << 1;
#ifdef DEBUG
					if (ifc == ifcT)
						vcPap2Same++;
					else  if (ifc + 1 == ifcT)
						vcPap2Plus1++;
					else  
						Assert(fFalse);
#endif /* DEBUG */
#ifdef DEBUG
#ifdef WIN
					if (!vfInCacheParaVerify)
#endif /* WIN */
#endif /* DEBUG */
						pfcb->ifcPapHint2 = ifcT;
					}
				else
					{
#ifdef DEBUG
#ifdef WIN
					int ifcSav = pfcb->ifcPapHint2;

#endif /* WIN */
#endif /* DEBUG */
					bpapx = BFromFc((struct FKP HUGE *)hpfkp, fc, &fcFirst, &fcLim, &pfcb->ifcPapHint2,
							 fWord3File);
#ifdef DEBUG
#ifdef WIN
					if (vfInCacheParaVerify)
						pfcb->ifcPapHint2 = ifcSav;
					if (!vfInCacheParaVerify)
#endif /* WIN */
#endif /* DEBUG */
						pfcb->pnPapHint2 = pn;
					}
				if (fcLim <= fcMac)
					{
					CHAR HUGE *hpch;
					int fPaphInval;
					int cch, stc;
					if (bpapx != 0/*nil*/)
						{
						hpch = ((CHAR HUGE *)hpfkp) + bpapx;
						cch = *hpch++;
						if (WinMac(nFib >= 25, !fWord3File && nFib >= 9))
							cch <<= 1;
						stc = *hpch++;
/* we only need to check for sprmPStcPermute application, if the papx stc is
	not normal and there is a prm for this piece. */
						if (stc != stcNormal && pcd.prm != prmNil)
							{
							struct PRM *pprm;
/* sprmPStcPermute can't fit in a non-complex (ie. 2-byte) prm */
							if ((pprm = (struct PRM *)&pcd.prm)->fComplex)
								{
								char *pprl;

								struct PRC *pprc = *HprcFromPprmComplex(pprm);

								pprl = pprc->grpprl;
/* if the first sprm is sprmPStcPermute, we interpret it here */
								if (*pprl == sprmPStcPermute)
									{
									if (stc <= *(pprl + 1))
										stc = *(pprl + 1 + stc);
									}
								}
							}
						}
					if (vdocPapLast == doc &&
							vfnPapLast == fn &&
							vpnPapLast == pn &&
							vbpapxPapLast == bpapx &&
							vstcLast != stcNil &&
							(bpapx == 0 || stc == vstcLast))
						{
						Debug(vcPapStcUnchanged++);
						goto LParaEnd2;
						}
					if (bpapx != 0/*nil*/)
						{
						int cbPhe;
						struct PHE phe;

#ifdef WIN
						cbPhe = cbPHE;
						bltbh(hpch, &phe, cbPHE);
#else
						if (!fWord3File)
							{
							cbPhe = cbPHE;
							bltbh(hpch, &phe, cbPHE);
							}
						else
							cbPhe = cbPAPH;
#endif /* not WIN */

						if (doc != vdocPapLast ||
								vstcLast != stc ||
								vcchPapxLast > 0)
							{
							MapStc(pdod, stc, &vchpStc, &vpapStc);
							SetWords(&vtapStc, 0, cwTAP);
							vstcLast = stc;
							}
Debug(else  
	vcStyleUnchanged++);
Assert(cch >= cbPhe + 1);
vcchPapxLast = cch - (cbPhe + 1);

fPaphInval = vpapStc.phe.fStyleDirty || pdod->fStyDirtyBroken;
vpapStc.phe = phe;

if ((cch -= cbPhe + 1) > 0)
{
	ApplyPrlSgc(hpch + cbPhe, cch, &vpapStc, sgcPap, fWord3File);
	if (vpapStc.fTtp)
		{
		ApplyPrlSgc(hpch + cbPhe, cch, &vtapStc, sgcTap, fWord3File);
		/* blow away non-standard properties of ttp paragraph */
		StandardPap(&vpapStc);
		vpapStc.fInTable = vpapStc.fTtp = fTrue;
		}
}


#ifdef MAC
/* convert Word 3 borders */
if (vpapStc.brcp || vpapStc.brcl)
{
	BrcsFromBrclBrcp(&vpapStc.brcTop, vpapStc.brcl, vpapStc.brcp);
	vpapStc.brcp = vpapStc.brcl = 0;
}


#endif
						}
					else
						{
/* bpapx == 0 is a special state, which is used to encode a set
	of properties which are an agreement between this clause
	and CbGenPapxFromPap */
						if (doc != vdocPapLast ||
								vstcLast != stcNormal ||
								vcchPapxLast > 0)
							{
							MapStc(pdod, stcNormal, &vchpStc, &vpapStc);
							vstcLast = stcNormal;
							vcchPapxLast = 0;
							}
Debug(else  
	vcStyleUnchanged++);

fPaphInval = vpapStc.phe.fStyleDirty;
vpapStc.phe.fDiffLines = 0;
vpapStc.phe.clMac = 1;
vpapStc.phe.dxaCol = 7980;
/* constant in following line must match value in CbGenPapxFromPap */
vpapStc.phe.dylLine = WinMac(240, 15);
						}
/* if the style has been dirtied during the edit session, invalidate the paph*/
					if (fPaphInval Mac(|| fWord3File))
						SetWords(&vpapStc.phe, 0, cwPHE);
/* establish the vpapStc cache */
					vdocPapLast = doc;
					vfnPapLast = fn;
					vpnPapLast = pn;
					vbpapxPapLast = bpapx;

LParaEnd2:                              
					caPara.cpLim = cpMin + (fcLim - fcMin);
					break;
					}
				}
			}
LNoParaEnd2:
/* Now we know there's no para end. */
		if (ipcd != ipcdBase)
			{
			/* not original piece, save knowledge */
			Debug(pcd.fNoParaLastValid = fTrue);
			pcd.fNoParaLast = fTrue;
#ifdef DEBUG
#ifdef WIN
			if (!vfInCacheParaVerify)
#endif /* WIN */
#endif /* DEBUG */
				PutPlc(hplcpcd, ipcd, &pcd);
			}
		cpGuess = cpLim;
		}
	vpapFetch = vpapStc;
	prm = pcd.prm;
/* note: if the prm we're applying causes vchpStc to be altered, vstcLast will
	be set to stcNil on return from DoPrmSgc to ensure that vchpStc is
	recalculated when we fetch a new paragraph. */
	if (prm != prmNil && !vpapFetch.fTtp)
		DoPrmSgc(prm, &vpapFetch, sgcPap);
#ifdef MAC
	if (vpapFetch.brcp || vpapFetch.brcl)
		BrcsFromBrclBrcp(&vpapFetch.brcTop, vpapFetch.brcl, vpapFetch.brcp);
#else /* WIN */
	Assert (!vpapFetch.brcp && !vpapFetch.brcl);
#endif /* WIN */
#ifdef JR
	JuniorPap();
#endif /* JR */
	MeltHp();
}


#endif /* DEBUGORNOTWIN */

#ifdef DEBUGORNOTWIN
/* B   F R O M   F C */
/* Return the b, fcFirst & fcLim for the first run with fcLim > fc. */
/* %%Function:BFromFc %%Owner:davidlu */
HANDNATIVE int BFromFc(hpfkp, fc, pfcFirst, pfcLim, pifc, fFkpWord3)
struct FKP HUGE *hpfkp;
FC fc, *pfcFirst, *pfcLim;
int *pifc;
#ifdef MAC
int fFkpWord3;
#endif /* MAC */
{
	struct RUN *prun, *rgrun;
	int crun, ifc, b;

	crun = hpfkp->crun;
#ifdef MAC
	if (fFkpWord3)
		crun = ((struct FKPO HUGE *)hpfkp)->crun;
#endif /* MAC */
	*pifc = ifc = IcpInRgcp(LpFromHp(hpfkp->rgfc), crun, (CP) fc);
	*pfcFirst = (hpfkp->rgfc)[ifc];
	*pfcLim = (hpfkp->rgfc)[ifc + 1];
	b = ((CHAR HUGE *)&((hpfkp->rgfc)[crun + 1]))[ifc];
#ifdef MAC
	if (!fFkpWord3)
#endif /* MAC */
		b <<= 1;        /* possible compiler problem */
	return (b);
}


#endif /* DEBUGORNOTWIN */

#ifdef DEBUGORNOTWIN
/* M A P   S T C */
/* maps pdod, stc into
	*pchp
	*ppap
*/
/* %%Function:MapStc %%Owner:davidlu */
HANDNATIVE MapStc(pdod, stc, pchp, ppap)
struct DOD *pdod; 
int stc; 
struct CHP *pchp; 
struct PAP *ppap;
{
	int cch, stcpT;
	CHAR HUGE *hpchpe, HUGE *hppape;

#ifdef MAC
	if (pdod->fShort || pdod->fMotherStsh)
		pdod = PdodDoc(pdod->doc);
#else /* WIN */
#ifdef DEBUG
	int cMothers = 0;
#endif /* DEBUG */
	while (!pdod->fMother || pdod->fMotherStsh)
		{
		Assert (cMothers++ < 5);/* avoid a loop */
		Assert (pdod->doc != docNil);
		pdod = PdodDoc(pdod->doc);
		}
#endif /* WIN */

	if (pdod == PdodDoc(docScrap) && vsab.docStsh != docNil)
		pdod = PdodDoc(vsab.docStsh);
	AssertH(pdod->hsttbChpe);
	Assert(ppap != 0);

	vstcpMapStc = (stc + pdod->stsh.cstcStd) & 0377;
	if (vstcpMapStc >= (*pdod->hsttbChpe)->ibstMac)
		{
		vstcpMapStc = pdod->stsh.cstcStd;
		if (stc >= stcStdMin) goto LStcStandard;
		}

	Assert(pdod->hsttbPape);
	Assert((*pdod->hsttbPape)->ibstMac > vstcpMapStc);
	hppape = HpstFromSttb(pdod->hsttbPape,vstcpMapStc);
	if ((cch = *hppape++) == 0377) goto LStcStandard;
	bltbh(hppape, (char HUGE *)ppap, cch);
	SetBytes(((CHAR *)ppap) + cch, 0,
			((cch < cbPAPBase) ?
			cbPAPBase : cbPAP) - cch);


	ppap->stc = stc; /* return proper stc even if we faked the style */

	if (pchp)
		{
		Assert((*pdod->hsttbChpe)->ibstMac > vstcpMapStc);
		hpchpe = HpstFromSttb(pdod->hsttbChpe, vstcpMapStc);
		cch = *hpchpe++;
		bltbh(hpchpe, (char HUGE *)pchp, cch);
		SetBytes(((CHAR *)pchp) + cch, 0, cwCHP*sizeof(int)-cch);
		}
	return;
LStcStandard:
	stcpT = vstcpMapStc;
	MapStc(pdod, 0, pchp, ppap);
	MapStcStandard(stc, pchp, ppap);
	vstcpMapStc = stcpT;
}


#endif /* DEBUGORNOTWIN */


#ifdef MAC
/* C H  L O W E R */
/* returns lower case for upper-case characters, otherwise returns ch */
NATIVE int /* WINIGNORE - unused in WIN */
/* %%Function:ChLower %%Owner:NOTUSED */
ChLower(ch)
int		ch;
{
	CHAR *pch;

	if ( ch <= 'Z' && ch >= 'A' )
		ch |= 0x20;
	else  if (ch >= 0x80)
		{
		for ( pch = mpUcLc; *pch != 0; pch += 2 )
			if ( ch == *pch )
				{
				ch = *(pch + 1);
				break;
				}
		}
	return(ch);
}


#endif	/* MAC */

#ifdef MAC /* in resident.asm if WIN */
/* L O W E R  R G C H */
/* converts characters to lower case */
/* %%Function:LowerRgch %%Owner:davidlu */
HANDNATIVE LowerRgch(pch, cch)
CHAR *pch;
int cch;
{
	for (; cch-- > 0; pch++)
		*pch = ChLower(*pch);
}


#endif	/* MAC */

#ifdef MAC
/* F  L O W E R */
/* returns true iff ch is a lowercase letter */
NATIVE int FLower(ch) /* WINIGNORE - MAC version, not used in WIN */
unsigned int ch;
{
	CHAR *pch;

	if ( ch < 'a' )
		return fFalse;
	if ( ch <= 'z' )
		return fTrue;
	if (ch < 0x87)
		return fFalse; /* not a special international char */
	if (ch <= 0x9F )
		return fTrue; /* it is a special intl char */

	/* finally, check the non-contiguous special intl chars */
	/* pch is set up to look at the odd bytes of the table */
	for ( pch = mpUcLc + ibUcLcReverse + 1; *pch != 0; pch += 2  )
		if ( ch == *pch )
			return fTrue;
	return fFalse;
}


#endif	/* MAC */

#ifdef MAC
/* F  U P P E R */
/* returns true iff ch is an uppercase letter */
NATIVE int FUpper(ch) /* WINIGNORE - MAC version, not used in WIN */
unsigned int ch;
{
	CHAR *pch;

	if ( ch <= 'Z' )
		return (ch >= 'A');
	if ( ch < 0x80 )
		return fFalse;
	for ( pch = mpUcLc; *pch != 0; pch += 2 )
		if ( ch == *pch )
			return fTrue;
	return fFalse;
}


#endif	/* MAC */

#ifdef MAC
/* C H  U P P E R */
/* for lower case letters returns upper case letter, otherwise returns ch */
NATIVE int  /* WINIGNORE - MAC version, not used in WIN */
/* %%Function:ChUpper %%Owner:NOTUSED */
ChUpper(ch)
unsigned int ch;
{
	CHAR *pch;

	if ( ch < 'a' )
		return (ch);

	if ( ch <= 'z' )
		{
		ch &= ~0x20;
		return (ch);
		}

	if (ch < 0x87)
		return(ch);	/* not a l.c. intl character */

	if ( ch <= 0x9F )
		{
		ch = mpLcUc[ch - 0x87];
		return ch;	/* map contiguous intl chars to u.c. */
		}

	/* check special case table for other l.c. intl chars */
	/* use the inverse of the last part of the mpUcLc array */
	for ( pch = mpUcLc + ibUcLcReverse; *pch != 0; pch += 2 )
		if ( ch == *(pch+1) )	/* found match in table */
			return *pch;	/* map to equivalent l.c. */

	return(ch);
}


#endif	/* MAC */

#ifdef DEBUGORNOTWIN
/* H P C H   F R O M   F C */
/* Fetches contents of page containing character fc in file fn and returns
a HUGE pointer to the char in the page that represents fc.
For Mac, a HUGE is a far *. For Opus it is a huge *.
Buffer number is left in vibp so that the buffer may be set dirty by caller.
*/
/* %%Function:HpchFromFc %%Owner:davidlu */
HANDNATIVE CHAR HUGE *HpchFromFc(fn, fc)
int fn;
FC fc;
{
	CHAR HUGE *hpch;
	int bOffset;

	Assert(fn >= fnScratch && mpfnhfcb[fn] != hNil && fn < fnMax);
	Assert(fc < fcMax);
	vibp = IbpCacheFilePage(fn, (PN)(fc >> shftSector));
	bOffset = (int)fc & maskSector; /* beware native compiler bug */
	hpch = HpBaseForIbp(vibp) + bOffset;
	return (hpch);
}


#endif /* DEBUGORNOTWIN */

#ifdef DEBUGORNOTWIN
/*  H P C H  G E T   P N */
/* Fetches contents of page pn in file fn into an external file buffer and
returns a HUGE pointer to the beginning of the page.
For Mac, a HUGE is a far *. For Opus it is a huge *.
Buffer number is left in vibp so that the buffer may be set dirty by caller.
*/
/* %%Function:HpchGetPn %%Owner:davidlu */
HANDNATIVE CHAR HUGE *HpchGetPn(fn, pn)
int fn;
PN pn;
{
	CHAR HUGE *hpch;

	Assert(fn >= fnScratch && mpfnhfcb[fn] != hNil && fn < fnMax);
	Assert(pn >= 0 && pn <= 0x7fff);
	vibp = IbpCacheFilePage(fn, pn);
	hpch = HpBaseForIbp(vibp);        /* possible compiler problem */
	return(hpch);
}


#endif /* DEBUGORNOTWIN */


#ifdef MAC
/* H P C H   G E T   P O */
/* Returns bch in buffer vibp to start of Word 3.0 buffer page po in file fn.
Buffer number is left in vibp so that the buffer may be set dirty by caller.
Offset in buffer is stored in vbchFc.
*/
/* %%Function:HpchGetPo %%Owner:NOTUSED */
NATIVE CHAR HUGE *HpchGetPo(fn, po) /* WINIGNORE - unused in WIN */
int fn;
PO po;
{
	CHAR HUGE *hpch;
	int bOffset;

	Assert(fn >= fnScratch && mpfnhfcb[fn] != hNil);
	vibp = IbpCacheFilePage(fn, po >> shftPoPn);
	bOffset = (po & 0x3) << shftSectorPre35; /* beware native compiler bug */
	hpch = HpBaseForIbp(vibp) + bOffset;
	return (hpch);
}


#endif	/* MAC */


#ifdef MAC /* in fetchn3.asm if WIN */
/* S E T  D I R T Y */
/* ibp in internal cache */
/* %%Function:SetDirty %%Owner:davidlu */
HANDNATIVE SetDirty(ibp)
int ibp;
{
	struct BPS HUGE *hpbps;
	hpbps = &((struct BPS HUGE *)vbptbExt.hpmpibpbps)[ibp];
	hpbps->fDirty = fTrue;
	Assert(hpbps->fn != fnNil);
	PfcbFn(hpbps->fn)->fDirty = fTrue;
}


#endif	/* MAC */


#ifdef DEBUGORNOTWIN
/* I B P   C A C H E   F I L E   P A G E */
/* Get page pn of file fn into file cache pbptb.
Return ibp.
See w2.rules for disk emergencies.
*/
/* %%Function:IbpCacheFilePage %%Owner:davidlu */
HANDNATIVE int IbpCacheFilePage(fn, pn)
int fn;
PN pn;
{
	extern int vfnPreload;

	int ibp, iibp;
	struct BPS HUGE *hpbps;
/* NOTE: IibpHash macro has changed */
	Assert(fn >= fnScratch && mpfnhfcb[fn] != hNil && fn < fnMax);
	Assert(pn >= 0 && pn <= 0x7fff);
	iibp = IibpHash(fn, pn, vbptbExt.iibpHashMax);
	ibp = (vbptbExt.hprgibpHash)[iibp];
/* search list of buffers with the same hash code */
	while (ibp != ibpNil)
		{
		hpbps = &vbptbExt.hpmpibpbps[ibp];
		if (hpbps->pn == pn && hpbps->fn == fn)
			{
/* page found in the cache */
			hpbps->ts = ++(vbptbExt.tsMruBps);
#ifdef WIN
			vbptbExt.hpmpispnts[(ibp << 2) / vbptbExt.cqbpspn]
					= vbptbExt.tsMruBps;
#endif
			return(ibp);
			}
		ibp = hpbps->ibpHashNext;
		}
#ifdef WIN
	if (fn == vfnPreload)
		{	/* Read in big chunks! */
		if ((ibp = IbpLoadFn(fn,pn)) != ibpNil)
			return ibp;
		}
#endif
/* page not found, read page into cache */
	ibp = IbpReadFilePage(fn, pn, iibp);
	return(ibp);    /* possible compiler problem */
}


#endif /* DEBUGORNOTWIN */


#ifdef MAC /* in file2.c if WIN */
/* P N  W H O S E  ... */
/* %%Function:PnWhoseFcGEFc %%Owner:NOTUSED */
NATIVE PN PnWhoseFcGEFc(fc) /* WINIGNORE - MAC version */
FC      fc;
{
	PN pn = (fc + (cbSector - 1)) >> shftSector;
	Assert(fc < fcMax);

	return (pn);    /* possible compiler problem */
}


#endif /* MAC */


#ifdef MAC /* in resident.asm if WIN */
/* F C  F R O M  P N */
/* %%Function:FcFromPn %%Owner:davidlu */
HANDNATIVE FC FcFromPn(pn)
PN      pn;
{
	FC fc = ((FC) pn) << shftSector;

	return (fc);    /* possible compiler problem */
}


#endif /* MAC */


#ifdef MAC /* in resident.asm if WIN */
/* P N  F R O M  F C */
/* %%Function:PnFromFc %%Owner:davidlu */
HANDNATIVE PN PnFromFc(fc)
FC      fc;
{
	PN pn = (PN) (fc >> shftSector);
	Assert(fc < fcMax);

	return (pn);
}


#endif /* MAC */


#ifdef MAC
/* P O  W H O S E  ... */
/* %%Function:PoWhoseFcGEFc %%Owner:NOTUSED */
NATIVE PO PoWhoseFcGEFc(fc) /* WINIGNORE - unused in WIN */
FC      fc;
{
	PO po = (fc + (cbSectorPre35 - 1)) >> shftSectorPre35;
	Assert(fc < fcMax);

	return (po);
}


#endif /* MAC */


#ifdef MAC
/* F C  F R O M  P O */
/* %%Function:FcFromPo %%Owner:NOTUSED */
NATIVE FC FcFromPo(po) /* WINIGNORE - unused in WIN */
PO      po;
{
	FC fc = ((FC) po) << shftSectorPre35;

	return (fc);
}


#endif /* MAC */


#ifdef MAC
/* P O  F R O M  F C */
/* %%Function:PoFromFc %%Owner:NOTUSED */
NATIVE PO PoFromFc(fc) /* WINIGNORE - unused in WIN */
FC      fc;
{
	PO po = (PO)  (fc >> shftSectorPre35);

	return (po);
}


#endif /* MAC */


#ifdef MAC /* in resident.asm if WIN */
/* P F C B  F N */
/* quick return of pfcb for a fn */
/* %%Function:PfcbFn %%Owner:davidlu */
struct FCB *PfcbFn(fn)
int fn;
{
	Assert(fn >= fnScratch && mpfnhfcb[fn] != hNil);
	return *mpfnhfcb[fn];
}


#endif /* MAC */


#ifdef DEBUGORNOTWIN
/* D O  P R M  S G C */
/* apply prm to prgbProps that is of type sgc */
/* %%Function:DoPrmSgc %%Owner:davidlu */
HANDNATIVE DoPrmSgc(prm, prgbProps, sgc)
struct PRM prm; 
int sgc; 
CHAR *prgbProps;
{
	int cch;
	CHAR *pprl;
	CHAR grpprl[2];

	if (prm.fComplex)
		{
		struct PRC *pprc;

		pprc = *HprcFromPprmComplex(&prm);
		cch = pprc->bprlMac;
		pprl = pprc->grpprl;
		}
	else
		{
/* cch = 1 will pick up one sprm, no matter what its length */
		cch = 1;
		grpprl[0] = prm.sprm;
		grpprl[1] = prm.val;
		pprl = grpprl;
		}
	ApplyPrlSgc((CHAR HUGE *)pprl, cch, prgbProps, sgc, fFalse);
}


#endif /* DEBUGORNOTWIN */


#ifdef DEBUGORNOTWIN
/* A P P L Y  P R L  S G C */
/* apply sprms of type sgc in grpprl of length cch to prgbProps */
/* %%Function:ApplyPrlSgc %%Owner:davidlu */
HANDNATIVE ApplyPrlSgc(hpprlFirst, cch, prgbProps, sgc, fOldVersion)
CHAR HUGE *hpprlFirst;
struct CHP *prgbProps;
int cch, sgc;
#ifdef MAC
BOOL fOldVersion;
#endif /* MAC */
{
	int val;
	struct SIAP siap;
	CHAR HUGE *hpprl = hpprlFirst;

	while (cch > 0)
		{
		int cchSprm, dsprm;
		struct ESPRM esprm;
		int sprm = *hpprl;
/*	if we encounter a pad character at the end of a grpprl of a PAPX we
	set the length to 1 and continue. */
		if (sprm == 0)
			{
			cchSprm = 1;
			goto LNext;
			}
/* if reading properties from Word 3.0 files, must map sprms into their new
	assignments. */
#ifdef MAC
		if (fOldVersion)
			{
			dsprm = 0;
			if (sprm >= sprmSBkcW3)
				dsprm = dsprmSBkc;
			else  if (sprm >= sprmPicFScaleW3)
				dsprm = dsprmPicFScale;
			else  if (sprm == sprmCPlainW3)
				dsprm = dsprmCPlain;
			else  if (sprm >= sprmCDefaultW3)
				dsprm = dsprmCDefault;

			sprm += dsprm;
			}
#endif /* MAC */
		Assert(sprm < sprmMax);
		esprm = dnsprm[sprm];
		val = *(hpprl + 1);
		if ((cchSprm = esprm.cch) == 0)
			{
			if (sprm == sprmTDefTable)
				bltbh(hpprl + 1, &cchSprm, sizeof(int));
			else
				{
				cchSprm = val;
				if (cchSprm == 255 && sprm == sprmPChgTabs)
					{
					char HUGE *hpprlT;
					cchSprm = (*(hpprlT = hpprl + 2) * 4) + 1;
					cchSprm += (*(hpprlT + cchSprm) * 3) + 1;
					}
				}
			cchSprm += 2;
			}
		if (esprm.sgc == sgc)
			{
			int shftBit;
			int maskBit;
			switch (esprm.spra)
				{
			case spraWord:
/* sprm has a word parameter that is to be stored at b */
				bltbh(hpprl+1, (CHAR *)prgbProps + esprm.b, 2);
				break;
			case spraByte:
/* sprm has a byte parameter that is to be stored at b */
				*((CHAR *)prgbProps + esprm.b) = val;
				break;
			case spraBit:
/* sprm has a byte parameter that is to be stored at bit b in word 0 */
/* WARNING: FOLLOWING CONSTRUCTION DEPENDS ON BIT ALLOCATION ORDER WITHIN
	STRUCTURES; DIFFERENT FOR Mac Word AND Opus */
#ifdef MAC
				maskBit = (1<<(shftBit = 15 - esprm.b));
#else
				maskBit = (1<<(shftBit = esprm.b));
#endif
/* if the high bit of the operand is on and the low bit of the operand is off
	we will make prop to be same as vchpStc prop. if the high operand bit is on
	and the low operand bit is on, we will set the prop to the negation of the
	vchpStc prop. */
				if (val & 0x80)
					{
					*((int *)prgbProps) &= ~maskBit;
					*((int *)prgbProps) |=
							(((*(int *)&vchpStc) & maskBit) ^
							((val & 0x01)<<shftBit));
					}
				else  if (val == 0)
					*((int *)prgbProps) &= ~(maskBit);
				else
					*((int *)prgbProps) |= maskBit;
				break;
			case spraCPlain:
				/*  fSpec and fFldVanish are properties that
					the user is not allowed to modify! */
				val = (*(int *)prgbProps) & maskFNonUser;
				blt(&vchpStc, prgbProps, cwCHPBase);
				Assert((*(int *)&vchpStc & maskFNonUser) == 0);
				(*(int *)prgbProps) |= val;
				break;
			case spraCFtc:
				bltbh(hpprl+1, &val, 2);
				prgbProps->ftc = val;
				break;
			case spraCKul:
				Assert(val <= 7); /* hand native assumption */
				prgbProps->kul = val;
				break;
			case spraCSizePos:
				bltbh(hpprl + 1, &siap, cbSIAP);
				if ((val = siap.hpsSize) != 0)
					prgbProps->hps = val;
				if ((val = siap.cInc) != 0)
					prgbProps->hps = HpsAlter(prgbProps->hps,
							val >= 64 ? val - 128 : val);
				if ((val = siap.hpsPos) != hpsPosNil)
					{
					if (siap.fAdj)
						{
						if (val != 0)
							{ /* Setting pos to super/sub */
							if (prgbProps->hpsPos == 0)
								prgbProps->hps = HpsAlter(prgbProps->hps, -1);
							}
						else
							{ /* Restoring pos to normal */
							if (prgbProps->hpsPos != 0)
								prgbProps->hps = HpsAlter(prgbProps->hps, 1);
							}
						}
					prgbProps->hpsPos = val;
					}
				break;
			case spraCHpsInc:
				val &= 255;
				prgbProps->hps = HpsAlter(prgbProps->hps,
						val >= 128 ? val - 256 : val);
				break;
			case spraCHpsPosAdj:
				if (val != 0)
					{ /* Setting pos to super/sub */

					if (prgbProps->hpsPos == 0)
						prgbProps->hps = HpsAlter(prgbProps->hps, -1);
					}
				else
					{ /* Restoring pos to normal */
					if (prgbProps->hpsPos != 0)
						prgbProps->hps = HpsAlter(prgbProps->hps, 1);
					}
				prgbProps->hpsPos = val;
				break;
			case spraCIco:
				prgbProps->ico = val;
				break;
#ifdef DEBUG
			default:
				Assert(fFalse); /* hand native assumption */
#endif
/* other special actions */
			case spraSpec:
/* if sprmPStcPermute is the first sprm of a grpprl, it would have been
	interpreted in CachePara before the papx grpprl was applied. */
				if (hpprl != hpprlFirst || sprm != sprmPStcPermute)
					ApplySprm(hpprl, sprm, val, prgbProps);
				}
			}
LNext:
		cch -= cchSprm;
		hpprl += cchSprm;
		}
}


#endif /* DEBUGORNOTWIN */

#ifdef MAC /* in fetchn2.asm if WIN */
/* F E T C H  C P  A N D  P A R A */
/* %%Function:FetchCpAndPara %%Owner:davidlu */
HANDNATIVE FetchCpAndPara( doc, cp, fcm )
int doc;
CP cp;
int fcm;
{
	CachePara(doc, cp);
	FetchCp(doc, cp, fcm);
}


#endif /* MAC */


#ifdef MAC /* in fetchn2.asm if WIN */
/* F E T C H  C P  A N D  P A R A  C A */
/* %%Function:FetchCpAndParaCa %%Owner:davidlu */
HANDNATIVE FetchCpAndParaCa( pca, fcm )
struct CA *pca;
int fcm;
{
	CacheParaCa(pca);
	FetchCp(pca->doc, pca->cpFirst, fcm);
}


#endif /* MAC */


#ifdef MAC /* in fetchn3.asm if WIN */
/* C H	F E T C H  */
/* %%Function:ChFetch %%Owner:davidlu */
HANDNATIVE ChFetch(doc, cp, fcm)
int doc;
CP cp;
int fcm;
{
	int ch;

	if (fcm != fcmChars)
		CachePara(doc, cp);
	FetchCp(doc, cp, fcm);
	ch = *vhpchFetch;
	return (ch);
}


#endif /* MAC */


#ifdef MAC  /* in fetchn3.asm if WIN */
/*********************/
/* F   A b s   P a p */
/* %%Function:FAbsPap %%Owner:davidlu */
HANDNATIVE int FAbsPap(doc, ppap)
int doc;
struct PAP *ppap;
{
/* returns fTrue if pap describes an absolutely positioned object */
	struct DOD *pdod = PdodDoc(doc);

	int fAbs = doc != vdocTemp && !pdod->fFtn Win (&& !pdod->fAtn) &&
	(ppap->dxaAbs != 0 || ppap->dyaAbs != 0 || 
			ppap->pcHorz != pcHColumn || ppap->dxaWidth != 0);

	return(fAbs);
}


#endif /* MAC */


#ifdef WIN
#ifdef DEBUG
/* H P	B A S E  F R O M  I B P */
/* %%Function:HpBaseForIbp %%Owner:davidlu */
CHAR HUGE *HpBaseForIbp(ibp)
int ibp;
{
	long hpBase;

	*((int *)&hpBase) = (ibp % vbptbExt.cbpChunk) * cbSector;
	*(((int *)&hpBase)+1) = ibp / vbptbExt.cbpChunk
			+ *(((int *)&vbptbExt.hprgbpExt)+1);
	return hpBase;
}


#endif /* DEBUG */
#endif /* WIN */



extern CP vcpFirstTablePara;
extern CP vcpFirstTableCell;

#ifdef DEBUGORNOTWIN
/* F  I N  T A B L E  D O C  C P */
/* Returns whether a doc,cp is in a table according to field structures.
	vcpFirstTablePara is set to the cpFirst of the table in this paragraph.
	vcpFirstCellPara is set to the cpFirst of the table cell.
*/
#ifdef MAC
/* %%Function:FInTableDocCp %%Owner:NOTUSED */
NATIVE BOOL FInTableDocCp(doc, cp) /* WINIGNORE - "C_" in WIN */
#else /* WIN */
/* %%Function:C_FInTableDocCp %%Owner:davidlu */
HANDNATIVE BOOL C_FInTableDocCp(doc, cp)
#endif /* WIN */
int doc;
CP cp;
{
#ifdef MAC
	extern int vfInFormatPage;
	extern struct CA caPara;
	extern struct PAP vpapFetch;
	if (!FInCa(doc, cp, &caPara))
		CachePara(doc, cp);
	vcpFirstTablePara = caPara.cpFirst;
	Debug(vcpFirstTableCell = cpNil); /* MAC - not used */

	return vpapFetch.fInTable;

#else  /* WIN */
	extern struct CA caPara;
	extern struct PAP vpapFetch;
	extern CP vcpFirstTablePara;
	extern CP vcpFirstTableCell;
	extern CP vmpitccp[];
	int icp;
	CP cpFirstCell, cpLimCell;
	int docFetchSav;
	CP cpFetchSav;
	int fcmFetchSav;

	CachePara(doc, cp);
	vcpFirstTableCell = vcpFirstTablePara = caPara.cpFirst;
	if (!vpapFetch.fInTable)
		return fFalse;

	if (!vtapFetch.fCaFull || !FInCa(doc, caPara.cpLim - 1, &caTap))
		{
		docFetchSav = vdocFetch;
		cpFetchSav = vcpFetch;
		fcmFetchSav = fcmFetch;
		/* Optimization to reduce time spent by CpFirstTap looking for
		   the beginning of the row. */
		if (CpFirstTap(doc, caPara.cpFirst) == cpNil)
			CpFirstTap(doc, caPara.cpLim - 1);
		if (docFetchSav != docNil && cpFetchSav <= CpMacDocEdit(docFetchSav))
			FetchCpAndPara(docFetchSav, cpFetchSav, fcmFetchSav);
		else
			vdocFetch = docNil;
		CachePara(doc, cp);
		}
	Assert(FInCa(doc, caPara.cpLim - 1, &caTap));
	Assert(caTap.cpFirst == vmpitccp[0]);

	for (icp = 0; cp >= vmpitccp[icp+1]; icp++);
	vcpFirstTableCell = vmpitccp[icp];
	vcpFirstTablePara = FInCa(doc, vcpFirstTableCell, &caPara)
		? vcpFirstTableCell : caPara.cpFirst;

	return (cp >= caTap.cpFirst);
#endif
}


#endif /* DEBUGORNOTWIN */


#ifdef MAC /* in resident.asm if WIN */
/* S T A N D A R D  C H P */
/* creates most basic chp */
/* %%Function:StandardChp %%Owner:davidlu */
HANDNATIVE StandardChp(pchp)
struct CHP *pchp;
{
	SetWords(pchp, 0, cwCHP);
#ifdef MAC
	pchp->hps = 24;
	pchp->ftc = 2;
#else
	pchp->hps = hpsDefault;
#endif
}


#endif /* MAC */


#ifdef MAC /* in resident.asm if WIN */
/* S T A N D A R D  P A P */
/* creates most basic pap */
/* %%Function:StandardPap %%Owner:davidlu */
HANDNATIVE StandardPap(ppap)
struct PAP *ppap;
{
	SetWords(ppap, 0, cwPAPBase);
}


#endif /* MAC */


#ifdef MAC /* in resident.asm if WIN */
/* S T A N D A R D  S E P */
/* creates most basic sep */
/* %%Function:StandardSep %%Owner:davidlu */
HANDNATIVE StandardSep(psep)
struct SEP *psep;
{
	SetWords(psep, 0, cwSEP);
	Mac(
	psep->dyaPgn = psep->dxaPgn = 36 * dyaPoint;
	psep->dxaColumns = dxaInch / 2;
	psep->dyaHdrTop = psep->dyaHdrBottom = 36 * dyaPoint;
	)
	Win (
	if (vitr.fMetric)
		psep->dxaColumns = psep->dyaHdrTop = psep->dyaHdrBottom = NMultDiv(5, dxaCm, 4);
	else
		psep->dxaColumns = psep->dyaHdrTop = psep->dyaHdrBottom = dxaInch/2;
	)
	/* psep->dxaColumnWidth = undefined */
	psep->bkc = bkcNewPage;
	psep->fEndnote = fTrue;
}


#endif /* MAC */


#ifdef MAC
/* I B P  L R U */
/* Look for least recently used vbptbExt cache page.
Returns ibp of page.
If vfScratchFile and scratch file pages
routine will strive to maintain a fraction of ext buffers clear.
This means that if there are too many dirty buffers, a dirty one
will be picked regardless of lru.
*/
NATIVE IbpLru() /* WINIGNORE - MAC version */
{
	extern struct MERR vmerr;
	extern BOOL vfScratchFile;

	int ibp, ibpLru = 0;
	int ibpMac = vbptbExt.ibpMac;
	int cbpsClean = ibpMac;
	int ibpScratch;
	TS ts, tsLru;
	struct BPS HUGE *hpbps = HpbpsIbp(0);
	TS tsMruBps = vbptbExt.tsMruBps;

	/* In order for ibpScratch to be valid the following Assert()
		must be true */
	Assert(ibpMac >= 5);
	tsLru = tsMax;
	for (ibp = 0; ibp < ibpMac; ibp++, hpbps++)
		{
		if (hpbps->fn == fnScratch)
			{
			if (!vfScratchFile)
				{
				cbpsClean--;
				continue;
				}
			if (hpbps->fDirty)
				{
				cbpsClean--;
				if (ibp != vibpProtect)
					ibpScratch = ibp;
				}
			}
/* normalize time stamps so that the MRU is intMax-1, and the others smaller
in unsigned arithmetic */
		ts = hpbps->ts - (tsMruBps + 1);
		if (ts < tsLru && ibp != vibpProtect)
			{
			tsLru = ts;
			ibpLru = ibp;
			}
		}

/* check for no scratch file and buffers almost full (7/8 full) */
	if (cbpsClean < max(ibpMac >> 3, 4))
		{
		if (vfScratchFile)
			return ibpScratch;
		else
			{
			if (!vmerr.fDiskEmerg)
				{
				ErrorEid(!vmerr.fScratchFileInit ? 
						eidSysLock : eidSysFull,"IbpLru");
				}
			vmerr.fDiskEmerg = fTrue;
			}
		}
	return(ibpLru);
}


#endif /* MAC */

#ifdef WIN
#ifdef DEBUG
/* I B P  L R U */
/* Look for least recently used vbptbExt cache page.
Returns ibp of page.
If vfScratchFile and scratch file pages
routine will strive to maintain a fraction of ext buffers clear.
This means that if there are too many dirty buffers, a dirty one
will be picked regardless of lru.
Note: this version will select a clean page over a dirty page unless 50%
of the pages are dirty.
*/
/* %%Function:C_IbpLru %%Owner:davidlu */
HANDNATIVE C_IbpLru()
{
	extern struct MERR vmerr;
	extern BOOL vfScratchFile;

	int ibp, ibpLruClean = 0, ibpLruDirty = 0;
	int ibpMac = vbptbExt.ibpMac;
	int cbpsScratchClean = ibpMac;
	int cbpsClean = 0;
	int cbpsDirty = 0;
	int ibpScratch;
	TS ts, tsLruClean, tsLruDirty;
	struct BPS HUGE *hpbps = HpbpsIbp(0);
	TS tsMruBps = vbptbExt.tsMruBps;

	/* In order for ibpScratch to be valid the following Assert()
		must be true */
	Assert(ibpMac >= 5);
	tsLruClean = tsLruDirty = tsMax;
	for (ibp = 0; ibp < ibpMac; ibp++, hpbps++)
		{
		if (hpbps->fn == fnScratch)
			{
			if (!vfScratchFile)
				{
				cbpsScratchClean--;
				continue;
				}
			if (hpbps->fDirty)
				{
				cbpsScratchClean--;
				if (ibp != vibpProtect)
					ibpScratch = ibp;
				}
			}
/* normalize time stamps so that the MRU is intMax-1, and the others smaller
in unsigned arithmetic */
		ts = hpbps->ts - (tsMruBps + 1);
		if (ibp != vibpProtect)
			{
/* keep separate tabs on clean and dirty pages so we can choose between them */
			if (hpbps->fDirty)
				{
				cbpsDirty++;
				if (ts <= tsLruDirty)
					{
					tsLruDirty = ts;
					ibpLruDirty = ibp;
					}
				}
			else
				{
				cbpsClean++;
				if (ts <= tsLruClean)
					{
					tsLruClean = ts;
					ibpLruClean = ibp;
					}
				}
			}
		}

/* check for no scratch file and buffers almost full (7/8 full) */
	if (cbpsScratchClean < max(ibpMac >> 3, 4))
		{
		if (vfScratchFile)
			{
			Assert (ibpScratch != vibpProtect);
			return ibpScratch;
			}
		else
			{
			if (!vmerr.fDiskEmerg)
				{
				ErrorEid(!vmerr.fScratchFileInit ? 
						eidSysLock : eidSysFull,"IbpLru");
				}
			vmerr.fDiskEmerg = fTrue;
			}
		}
	if (tsLruClean > tsLruDirty && cbpsDirty > cbpsClean)
		/* only return the dirty page if it is older and there are
			more dirty pages than clean pages */
		{
		Assert (ibpLruDirty != vibpProtect);
		return ibpLruDirty;
		}
	else
		{
		Assert (ibpLruClean != vibpProtect);
		return ibpLruClean;
		}
}


#endif /* DEBUG */
#endif /* WIN */

#ifdef	MAC
/***************************/
/* %%Function:LpInitFetch %%Owner:NOTUSED */
native long LpInitFetch(lpmh4) /* WINIGNORE - unused in WIN */
char far *lpmh4;
{
/* fixed segment initialization routine */
	return(0L);
}


/******
	WARNING: LpInitXxxx must be the last routine in this module
	because it is a fixed segment
******/
#endif
