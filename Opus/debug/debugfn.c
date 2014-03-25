/* debugfn.c */

#ifdef DEBUG
#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"

#include "debug.h"
#include "props.h"
#include "format.h"
#include "doc.h"
#include "sel.h"
#include "file.h"
#include "disp.h"
#include "prm.h"
#include "fkp.h"
#include "doslib.h"
#include "winddefs.h"
#include "inter.h"
#include "ch.h"
#include "opuscmd.h"

#include "idd.h"
#include "sdmdefs.h"
#include "sdmver.h"
#include "sdm.h"
#include "sdmtmpl.h"
#include "sdmparse.h"
#include "dbgfile.hs"
#include "dbgfile.sdm"


extern struct DOD       **mpdochdod[];
extern struct FCB       **mpfnhfcb[];
extern int              fnMac;
extern struct BPTB      vbptbExt;
extern struct MERR      vmerr;
extern struct SEL       selCur;
extern int		vfScratchFile;
extern struct DBS       vdbs;
extern CHAR             szEmpty[];
extern CHAR             stEmpty[];

#define HpOfHq(hq)  	(HpOfSbIb(SbOfHp(hq),*(HQ)(hq)))


/* C K   F I L E   E R R O R */
/* %%Function:CkFileError %%Owner:BRADV */
CkFileError(szMessage1, szMessage2, szMessage3)
char *szMessage1, *szMessage2, *szMessage3;
{
	char *pch;
	char rgchT[256];

	pch = rgchT;
	pch = bltbyte( &vdbs.stRoutineName[1], pch, vdbs.stRoutineName [0] );
	pch += CchCopySz(SzShared(": "), pch);
	pch = bltbyte(&vdbs.stFile[1], pch, vdbs.stFile[0]);
	pch += CchCopySz(SzShared(": "), pch);
	pch += CchCopySz(szMessage1, pch);
	if ((int)szMessage2)
		pch += CchCopySz(szMessage2, pch);
	if ((int)szMessage3)
		pch += CchCopySz(szMessage3, pch);
	*pch = '\0';

	if (IdPromptBoxSz( rgchT, MB_OKCANCEL | MB_APPLMODAL | MB_ICONHAND )
			== IDCANCEL)
		{
		DebugBreak(0);
		}
} /* end CkFileError */


/* A L L O C A T I O N   E R R O R */
/* %%Function:AllocationError %%Owner:BRADV */
AllocationError(ph, szMessage1, szMessage2)
char *ph;
char *szMessage1;
char *szMessage2;
{

	FreePh(ph);
	CkFileError(SzShared("could not allocate room for "), szMessage1, szMessage2);
} /* end AllocationError */


/* F R E A D   F I L E   R G C H */
/* %%Function:FReadFileRgch %%Owner:BRADV */
int FReadFileRgch(hpch, cchRead, szMessage1, szMessage2)
CHAR HUGE *hpch;
int cchRead;
char *szMessage1, *szMessage2;
	/* read cchRead characters from the file at vdbs.fcCur into *pch */
{
	FC fcCur = vdbs.fcCur;

	vdbs.fcCur = fcCur + cchRead;
	if (DwSeekDw( vdbs.osfn, fcCur, SF_BEGINNING ) < 0)
		{
		CkFileError(SzShared("could not set file position to read "),
				szMessage1, szMessage2);
		return (fFalse);
		}
	if (CchReadHpch( vdbs.osfn, hpch, (uns) cchRead ) < 0)
		{
		CkFileError(SzShared("could not read "),
				szMessage1, szMessage2);
		return (fFalse);
		}
	return (fTrue);

} /* end ReadFileRgch */


/* H P L C   R E A D   F I L E   P L C */
/* %%Function:HplcReadFilePlc %%Owner:BRADV */
struct PLC **HplcReadFilePlc(fcRead, cchRead, cbEntry, szName)
FC	fcRead;
int     cchRead;
int     cbEntry;
char    *szName;
{
	int     iMax;
	struct PLC **hplc;

	if (cchRead <= 0)
		return (hNil);

	iMax = (cchRead - sizeof(CP)) / (cbEntry + sizeof(CP));
	if ((hplc = HplcInit(cbEntry, iMax, cp0, fTrue /*fExternal*/)) == hNil
			|| vmerr.fMemFail)
		{
		AllocationError(&hplc, 0, szName);
		return(hNil);
		}

	(*hplc)->iMac = iMax;
	Assert ((*hplc)->fExternal);
	if (cchRead != iMax * (cbEntry + sizeof(CP)) + sizeof(CP))
		CkFileError(SzShared("impossible cb in file for "),
				szName, 0);
	cchRead = iMax * (cbEntry + sizeof(CP)) + sizeof(CP);

	if (fcRead >= 0L)
		vdbs.fcCur = fcRead;
	if (!FReadFileRgch(HpOfHq((*hplc)->hqplce), cchRead, szName, 0))
		FreePhplc(&hplc);

	return(hplc);

} /* end HplcReadFilePlc */


/* H S T T B   R E A D   F I L E   S T T B */
/* %%Function:HsttbReadFileSttb %%Owner:BRADV */
struct STTB **HsttbReadFileSttb(szName, fStyleRules)
char *szName;
int fStyleRules;
{
	uns	cchStrings;
	struct STTB    **hsttb;
	uns	cst;
	char HUGE *hpst;
	char HUGE *hpstStart;
	uns	cbNonStrings;
	int     ist;
	HQ	hq;

	if (!FReadFileRgch((CHAR HUGE *)(&cchStrings), sizeof(int),
			SzShared("count of bytes in strings for "), szName))
		return (hNil);
	cchStrings -= sizeof(int);

	if ((hsttb = (struct STTB **)HAllocateCw(cwSTTB)) == hNil
			|| vmerr.fMemFail)
		{
		AllocationError(&hsttb, SzShared("sttb base in "), szName);
		return(hNil);
		}

	if (((*hsttb)->hqrgbst =    /* HM */
			HqAllocLcb((long)max(cchStrings, 8))) == hqNil
			|| vmerr.fMemFail)
		{
		AllocationError(&hsttb, SzShared("strings in "), szName);
		FreeH(hsttb);
		return (hNil);
		}

	if (!FReadFileRgch(HpOfHq((*hsttb)->hqrgbst), cchStrings, SzShared("strings in "), szName))
		{
		FreeHsttb(hsttb);
		return(hNil);
		}

	for (hpst = hpstStart = HpOfHq((*hsttb)->hqrgbst), cst = 0;
			hpst < hpstStart + cchStrings;
			hpst += ((*hpst == 0xFF && fStyleRules) ? 0 : *hpst) + 1, cst++);

	if (hpst != hpstStart + cchStrings)
		{
		CkFileError(SzShared("Bad file representation of "), szName, 0);
		FreeHsttb(hsttb);
		return(hNil);
		}

	cbNonStrings = cst * sizeof(int);
	if ((long)cst + (long)cst + (long)cchStrings > 65535L)
		{
		CkFileError(SzShared("cb too large for "),
				szName, 0);
		FreeHsttb(hsttb);
		return(hNil);
		}

	hq = (*hsttb)->hqrgbst;
	/* FChngSizePhqLcb may cause heap movement! */
	if (!FChngSizePhqLcb(&hq, (long)(cbNonStrings + cchStrings)))
		{
		AllocationError(&hsttb, SzShared("bst's in "), szName);
		FreeHsttb(hsttb);
		return(hNil);
		}
	(*hsttb)->hqrgbst = hq;

	bltbh(HpOfHq(hq), ((char HUGE *)HpOfHq(hq)) + cbNonStrings, cchStrings);

	(*hsttb)->fExternal = fTrue;
	(*hsttb)->bMax = cbNonStrings + cchStrings;
	(*hsttb)->cbExtra = 0;
	(*hsttb)->ibstMac = cst;
	(*hsttb)->ibstMax = cst;
	(*hsttb)->fStyleRules = fStyleRules;
	(*hsttb)->fNoShrink = fFalse;

	for (hpst = (hpstStart = HpOfHq((*hsttb)->hqrgbst)) + cbNonStrings, ist = 0;
			hpst < hpstStart + cbNonStrings + cchStrings;
			((int HUGE *)hpstStart)[ist++] = IbOfHp(hpst) - IbOfHp(hpstStart),
			hpst += ((*hpst == 0xFF && fStyleRules) ? 0 : *hpst) + 1);

	return (hsttb);

} /* end HsttbReadFileSttb */


/* H P L   R E A D   F I L E   P L */
/* %%Function:HplReadFilePl %%Owner:BRADV */
struct PL **HplReadFilePl(cbEntry, szName)
int cbEntry;
char *szName;
{
	int     iMacPl;
	struct PL **hpl;

	if (!FReadFileRgch((CHAR HUGE *)(&iMacPl), sizeof(int),
			"count of bytes in plex for ", szName))
		return (hNil);

	if ((hpl = HplInit(cbEntry, iMacPl)) == hNil || vmerr.fMemFail)
		{
		AllocationError(&hpl, 0, szName);
		return(hNil);
		}
	(*hpl)->iMac = iMacPl;
	if (!FReadFileRgch((CHAR HUGE *)(&(*hpl)->rgbHead), cbEntry * iMacPl, 0, szName))
		{
		FreePh(&hpl);
		}
	return(hpl);

} /* end HplReadFilePl */


/* C M D   C K   D I S K   F I L E */
/* checks the internal consistency of a file on disk */
/* %%Function:CmdCkDiskFile %%Owner:BRADV */
CMD CmdCkDiskFile(pcmb)
CMB * pcmb;
{
	CABCKDISKFILE *pcabCkDiskFile;
	HCAB hcab;
	char *pch;
	int fn;
	int tmc = tmcOK;
	struct FCB **hfcb = hNil, *pfcb;
	int cwfcb;
	struct PLCBTE **hplcbteChp = hNil, **hplcbtePap = hNil;
	struct PLCBTE **HplcReadFilePlc();
	char szFileName[ichMaxFile];
	struct FIB fib;
	FC cbFileMac;
	int fCkPlcbteSav;
	int fCkFkpSav;
	int fCkBptbSav;
	char rgchT[256];
	char dlt [sizeof (dltCkDiskFile)];
	struct OFS ofs;

	SzToSt(SzShared("CmdCkDiskFile"), vdbs.stRoutineName);

	hcab = pcmb->hcab;
	FSetCabSz(hcab, szEmpty, Iag(CABCKDISKFILE, hszFile));
	if (pcmb->fDialog)
		{
		BltDlt(dltCkDiskFile, dlt);
		if ((tmc = TmcOurDoDlg(dlt, pcmb)) == tmcCancel
				|| tmc == tmcError)
			return cmdError;
		}

	if (pcmb->fAction)
		{
		Assert (tmc == tmcOK);
		pcabCkDiskFile = (CABCKDISKFILE *) PcabFromHcab(pcmb->hcab);
		GetSzFile(pcmb->hcab, szFileName, sizeof(szFileName), Iag(CABCKDISKFILE, hszFile));
		}

	/* save the file name in a global for error messages */
	SzToSt(szFileName, vdbs.stFile);

	if ((vdbs.osfn = OpenFile(szFileName, &ofs, 0)) < 0)
		{
		/* could not open file */
		CkFileError(SzShared("couldn't open file"), 0, 0);
		goto CkDiskFileQuit;
		}

	vdbs.fcCur = (FC)cp0;
	if (!FReadFileRgch((CHAR HUGE *)(&fib), sizeof(struct FIB), SzShared("fib"), 0))
		{
		/* could not read fib */
		goto CkDiskFileClose;
		}

	if ((cbFileMac = DwSeekDw( vdbs.osfn, 0L, SF_END )) < 0)
		{
		/* could not determine eof position */
		CkFileError(SzShared("couldn't determine eof position"), 0, 0);
		goto CkDiskFileClose;
		}

	if (fib.wIdent != wMagic)
		{
		/* this is not a formatted file - none of the
			rest of this procedure is applicable */
		CkFileError( SzShared("Not a formatted file"), 0, 0);
		goto CkDiskFileClose;
		}

	hfcb = (struct HH **)HAllocateCw(cwfcb
			= CwFromCch(sizeof(struct FCB)
			+ max(0, vdbs.stFile[0] - 1)));
	pfcb = *hfcb;
	SetWords((int *)pfcb, 0, cwfcb);
	pfcb->fHasFib = fTrue;
	pfcb->fDoc = fTrue;
	pfcb->cbMac = fib.fcClx + fib.cbClx;
	Assert(pfcb->cbMac <= cbFileMac);
	pfcb->osfn = vdbs.osfn;
	pfcb->nFib = fib.nFib;
	CopySt(vdbs.stFile, &pfcb->stFile);

	vdbs.fcCur = fib.fcPlcfbteChpx;
	if (!(hplcbteChp = HplcReadFilePlc(fib.fcPlcfbteChpx,
			fib.cbPlcfbteChpx, cbBTE,
			SzShared("plcbteChp"))))
		{
		/* could not read plcbteChp */
		goto CkDiskFileClose;
		}

	vdbs.fcCur = fib.fcPlcfbtePapx;
	if (!(hplcbtePap = HplcReadFilePlc(fib.fcPlcfbtePapx,
			fib.cbPlcfbtePapx, cbBTE,
			SzShared("plcbtePap"))))
		{
		/* could not read plcbtePap */
		goto CkDiskFileClose;
		}

	pfcb = *hfcb;
	pfcb->hplcbteChp = hplcbteChp;
	pfcb->hplcbtePap = hplcbtePap;

	fCkPlcbteSav = vdbs.fCkPlcbte;
	fCkFkpSav = vdbs.fCkFkp;
	fCkBptbSav = vdbs.fCkBptb;
	vdbs.fCkPlcbte = fTrue;
	vdbs.fCkFkp = fTrue;
	vdbs.fCkBptb = fTrue;
	CkHfcb(hfcb, &fib,
			fFalse /* not scratch file */,
			fTrue /* check file structures */,
			fFalse /* fAbortOK */);
	vdbs.fCkPlcbte = fCkPlcbteSav;
	vdbs.fCkFkp = fCkFkpSav;
	vdbs.fCkBptb = fCkBptbSav;

	pch = rgchT;
	pch += CchCopySz(SzShared("The checking of "), pch);
	bltb(&vdbs.stFile[1], pch, vdbs.stFile[0]);
	pch += vdbs.stFile[0];
	pch += CchCopySz(SzShared(" is complete. "), pch);
	IdPromptBoxSz( rgchT, MB_MESSAGE );

CkDiskFileClose:
	Assert(FCloseDoshnd(vdbs.osfn) != 0);

CkDiskFileQuit:
	FreePhplc(&hplcbtePap);
	FreePhplc(&hplcbteChp);
	FreePh(&hfcb);

	return cmdOK;
} /* end CmdCkDiskFile */


/* C K   F N S */
/* checks all fns */
/* %%Function:CkFns %%Owner:BRADV */
CkFns(fAbortOK)
BOOL fAbortOK;
{
	int fn;
	if (!vdbs.fCkFn) return;
	for (fn = 1 + !(vmerr.fScratchFileInit && !vmerr.fPostponeScratch); 
			fn < fnMac && (!fAbortOK || !FMsgPresent(mtyIdle)); 
			fn++)
		CkFn2(fn, fAbortOK);
} /* end CkFns */


/* C K   F N */
/* %%Function:CkFn %%Owner:BRADV */
CkFn(fn)
int fn;
{
	CkFn2(fn, fFalse);
}


/* C K   F N 2 */
/* %%Function:CkFn2 %%Owner:BRADV */
CkFn2(fn, fAbortOK)
int fn;
BOOL fAbortOK;
	{ /*
	DESCRIPTION:
	check consistency of an fn
*/
	struct FIB fib;

	if (!vdbs.fCkFn) return;
	SzToSt(SzShared("CkFn"), vdbs.stRoutineName);
	Assert(fn >= 0 && fn < fnMac);
	if (!mpfnhfcb[fn]) return;
	vdbs.stFile[0] = min((*mpfnhfcb[fn])->stFile[0], 29);
	bltb(&((*mpfnhfcb[fn])->stFile[1]), &vdbs.stFile[1], vdbs.stFile[0]);
	if ((vdbs.osfn = (*mpfnhfcb[fn])->osfn) == osfnNil)
		{
		/* Since we are going to eventually open a file,
			make sure that at most one less than the max number
			of open files are actually opened. */
		FAccessFn( fnNil, 0 );
		/* following can conceivably fail under obscure
			circumstances involving swapping of floppies (JBL)
		But, we aren't interested in running opus off of
			floppies! (pj)*/
		vdbs.osfn = OsfnValidAndSeek( fn,fc0 );
		if (vdbs.osfn == osfnNil)
			return;
		}
	vdbs.fcCur = (FC)cp0;
	if (fn != fnScratch && (*mpfnhfcb[fn])->fHasFib)
		{
		if (!FReadFileRgch((CHAR HUGE *)(&fib), sizeof(struct FIB), 
				SzShared("fib"), 0))
			return;
		ConvertFib(&fib);
		if (fib.pnNext)
			{
			CkHfcb(mpfnhfcb[fn], &fib, fFalse, fFalse, fAbortOK);
			vdbs.fcCur = FcFromPn (fib.pnNext);
			if (!FReadFileRgch((CHAR HUGE *)(&fib), sizeof(struct FIB), 
					SzShared("fibGlsy"), 0))
				return;
			ConvertFib(&fib);
			}

		}
	CkHfcb(mpfnhfcb[fn], &fib, fn == fnScratch,
			fFalse /* don't check file structures */, fAbortOK);

} /* end CkFn */



/* C K   H F C B */
/* %%Function:CkHfcb %%Owner:BRADV */
CkHfcb(hfcb, pfib, fScratch, fCkFileStructures, fAbortOK)
struct FCB **hfcb;
struct FIB *pfib;
int fScratch;
BOOL fAbortOK;
	{ /*
	DESCRIPTION:
	check consistency of an fcb
*/
	int fComplex;
	struct FCB *pfcb;

	if (fAbortOK && FMsgPresent(mtyIdle))
		return;

	pfcb = *hfcb;
#ifdef NOCKCOMPOUND  /* until this stuff gets updated */
	if ((*hfcb)->fCompound)
		return;
#endif /* NOCKCOMPOUND */
	if (!fScratch)
		CkHhh(hfcb,
				sizeof(struct FCB) + max(0, (pfcb->stFile[0] - 1)));
	/* mutually exclusive flags */
	Assert (pfcb->fDoc + pfcb->fTemp + pfcb->fDMEnum <= 1);
	/* fn == fnScratch ==> pnXLimit == 0 */
	Assert (!fScratch || pfcb->pnXLimit == 0);
	Assert (pfcb->stFile[0] <= ichMaxFile);
	if (pfcb->fHasFib)
		{
		if (fScratch)
			{
			fComplex = fFalse;
			}
		else
			{
			fComplex = pfib->fComplex;
			CkFib(pfib, hfcb);

			if (fCkFileStructures)
				CkFileStructures(pfib, fComplex);
			}

		Assert ((*hfcb)->hplcbteChp);
		CkHplcbte((*hfcb)->hplcbteChp, hfcb, fScratch, fComplex, fFalse, fAbortOK);
		Assert ((*hfcb)->hplcbtePap);
		CkHplcbte((*hfcb)->hplcbtePap, hfcb, fScratch, fComplex, fTrue, fAbortOK);
		}

} /* end CkHfcb */


/* C K   F I L E   S T R U C T U R E S */
/* %%Function:CkFileStructures %%Owner:BRADV */
CkFileStructures(pfib, fComplex)
struct FIB *pfib;
int fComplex;
{
	int cstcStd;
	struct STTB **hsttbName;
	struct STTB **hsttbChpx;
	struct STTB **hsttbPapx;
	struct PLESTCP **hplestcp;
	struct PLCBTE *pplcbte;
	struct PLC **hplcfndRef;
	struct PLC **hplcfndTxt;
	struct PLC **hplcsed;
	struct PLC **hplcpgd;
	struct STTB **hsttbGlsy;
	struct PLC **hplcglsy;
	int cprc;
	int cbplcpcd;
	struct PLC **hplcpcd = hNil;
	int ipcd;
	struct PRM *pprm;

	vdbs.fcCur = pfib->fcStshf;
	if (!FReadFileRgch((CHAR HUGE *)(&cstcStd), sizeof(int), 
			SzShared("cstcStd"), 0))
		return;

	hsttbName = HsttbReadFileSttb(SzShared("hsttbName"), fTrue);

	hsttbChpx = HsttbReadFileSttb(SzShared("hsttbChpx"), fTrue);

	hsttbPapx = HsttbReadFileSttb(SzShared("hsttbPapx"), fTrue);

	hplestcp = HplReadFilePl(cbESTCP, SzShared("hplestcp"));

	vdbs.fcCur = pfib->fcPlcffndRef;
	hplcfndRef = (struct PLC **)HplcReadFilePlc(
			pfib->fcPlcffndRef, pfib->cbPlcffndRef,
			cbFRD, SzShared("hplcfndRef"));

	Assert (vdbs.fcCur == pfib->fcPlcffndTxt);
	hplcfndTxt = (struct PLC **)HplcReadFilePlc(
			pfib->fcPlcffndTxt, pfib->cbPlcffndTxt,
			cbFND, SzShared("hplcfndTxt"));

	Assert (vdbs.fcCur == pfib->fcPlcfsed);
	hplcsed = (struct PLC **)HplcReadFilePlc(
			pfib->fcPlcfsed, pfib->cbPlcfsed,
			cbSED, SzShared("hplcsed"));

	Assert (vdbs.fcCur == pfib->fcPlcfpgd);
	hplcpgd = (struct PLC **)HplcReadFilePlc(
			pfib->fcPlcfpgd, pfib->cbPlcfpgd,
			cbPGD, SzShared("hplcpgd"));
	Win((*hplcpgd)->fMult = fTrue);

	hsttbGlsy = hplcglsy = hNil;
	Assert (vdbs.fcCur == pfib->fcSttbfglsy);

	if (pfib->cbSttbfglsy)
		{
		hsttbGlsy = (struct PLC **)HsttbReadFileSttb( SzShared("hsttbglsy"), fFalse );
		hplcglsy = (struct PLC **)HplcReadFilePlc(
				pfib->fcPlcfglsy, pfib->cbPlcfglsy,
				cbPGD, SzShared("hplcglsy") );
		}

	if (fComplex)
		{
		vdbs.fcCur = pfib->fcClx;
		/* here we check the PRC's and the plcpcd */
		CprcReadFilePrcs(fFalse /* no assertions */,
				pfib->cbClx);
		hplcpcd = hNil;
		if (FReadFileRgch((CHAR HUGE *)(&cbplcpcd), sizeof(int),
				SzShared("count of bytes in plcpcd"), 0))
			{
			hplcpcd = (struct PLC **)
					HplcReadFilePlc( -1L,
					cbplcpcd, cbPCD, SzShared("hplcpcd"));
			}
		}

	CkHsttb(hsttbName, 0);
	CkHsttb(hsttbChpx, 0);
	CkHsttb(hsttbPapx, 0);
	CkHplFoo(hplestcp, cbESTCP);
	CkHplcFoo(hplcfndRef, cbFRD, fFalse);
	CkHplcFoo(hplcfndTxt, cbFND, fFalse);
	CkHplcFoo(hplcsed, cbSED, fFalse);
	CkHplcFoo(hplcpgd, cbPGD, fFalse);
	CkHsttb(hsttbGlsy, 0);
	CkHplcFoo(hplcglsy, cbPGD, fFalse);
	if (pfib->cbSttbfglsy)
		{
		Assert((*hsttbGlsy)->ibstMac
				== (*hplcglsy)->iMac);
		}
	if (fComplex)
		{
		struct PCD pcd;
		/* here we check the PRC's and the plcpcd */
		vdbs.fcCur = pfib->fcClx;
		cprc = CprcReadFilePrcs(fTrue /* assertions */,
				pfib->cbClx);
		Assert(hplcpcd != hNil);
		CkHplcFoo(hplcpcd, cbPCD, fTrue);
		/* and we loop thru the piece entries.
		We must ensure that the iclx stored in
		the hDiv2 field is not unreasonable. */

		for (ipcd = 0; ipcd < IMacPlc( hplcpcd ); ipcd++ )
			{
			GetPlc( hplcpcd, ipcd, &pcd );
			pprm = (struct PRM *) &pcd.prm;
			Assert(!pprm->fComplex
					|| pprm->cfgrPrc < cprc);
			}
		}

	FreeHsttb(hsttbName);
	FreeHsttb(hsttbChpx);
	FreeHsttb(hsttbPapx);
	FreePh(&hplestcp);
	FreePhplc(&hplcfndRef);
	FreePhplc(&hplcfndTxt);
	FreePhplc(&hplcsed);
	FreePhplc(&hplcpgd);
	FreeHsttb(hsttbGlsy);
	FreePhplc(&hplcglsy);
	FreePhplc(&hplcpcd);

} /* end CkFileStructures */


/* C P R C   R E A D   F I L E   P R C S */
/* %%Function:CprcReadFilePrcs %%Owner:BRADV */
int CprcReadFilePrcs(fUseAssertions, cbClx)
int     fUseAssertions;
int     cbClx;
{
	int     cprc;
	char    clxt;
	int     cbGrpprl;
	char    szMessage[20];
	char    *pch;
	struct MPRC    prc;

	cprc = 0;
	clxt = clxtPrc;
	while (cbClx > 0 && clxt == clxtPrc)
		{
		pch = szMessage;
		pch += CchCopySz(SzShared("clxt #"), pch);
		CchIntToPpch(cprc, &pch);
		*pch = 0;
		SetBytes(&prc, 0, cbMaxGrpprl);
		if (!FReadFileRgch((CHAR HUGE *)(&clxt), sizeof(char), szMessage, 0)
				|| clxt != clxtPrc)
			break;
		pch = szMessage;
		pch += CchCopySz(SzShared("cbGrpprl #"), pch);
		CchIntToPpch(cprc, &pch);
		*pch = 0;
		if (!FReadFileRgch((CHAR HUGE *)(&cbGrpprl), sizeof(int), szMessage, 0))
			break;
		pch = szMessage;
		pch += CchCopySz(SzShared("grpprl #"), pch);
		CchIntToPpch(cprc, &pch);
		*pch = 0;
		if (!FReadFileRgch((CHAR HUGE *)(&prc.grpprl), cbGrpprl, szMessage, 0))
			break;
		if (fUseAssertions)
			{
			CkGrpprl(cbGrpprl, &prc.grpprl, fFalse /* grpprl not from papx */);
			}
		cbClx -= cbGrpprl + 3;
		cprc++;
		}

	/* now we read the piece table into the heap. */
	Assert(!fUseAssertions || (cbClx != 0 && clxt == clxtPlcpcd));

	return(cprc);
}


/* C K   H P L C B T E */
/* %%Function:CkHplcbte %%Owner:BRADV */
CkHplcbte(hplcbte, hfcb, fScratch, fComplex, fPara, fAbortOK)
struct PLCBTE **hplcbte;
struct FCB **hfcb;
int fScratch;
int fComplex;
int fPara;
int fAbortOK;
	{ /*
	DESCRIPTION:
	Check consistency of bin tables and FKP's.
*/
	struct PLCBTE *pplcbte;
	PN pnMac, pnPrev;
	FC cbMac;
	int ibteMac;
	int ibte;
	BOOL fCompound = (*hfcb)->fCompound;
	PN pnPriorFkp;
	FC fcLimPriorFkp;
	struct FKP fkp;
	char rgchText[cbSector];
	struct BTE bte;
	char chCur;
	extern FC FcCkFkp();

	if (!vdbs.fCkPlcbte) return;
	if (!hplcbte) return;

	CkHplcFoo(hplcbte, sizeof(struct BTE),
			!fScratch /* controls iMac check */);

	cbMac = (*hfcb)->cbMac;
	ibteMac = IMacPlc(hplcbte);
	pnMac = (PN) ((cbMac + cbSector - 1) >> shftSector);
	fcLimPriorFkp = fScratch ? 0 : -1;
	pnPrev = 0xFFFF;
	for (ibte = 0; ibte < ibteMac; ibte++)
		{
		GetPlc(hplcbte, ibte, &bte);
		Assert(pnPrev == 0xFFFF || bte.pn > pnPrev);

		if (fAbortOK && FMsgPresent(mtyIdle))
			return;

		if (vdbs.fCkFkp)
			{
			/* get FKP page */
			if (!FGetPn(fScratch, bte.pn, &fkp))
				return;

			/* in a non complex file, FKP pages must be adjacent and
			refer to contiguous fc's. */
			if (fcLimPriorFkp != -1)
				{
				if (!fComplex)
					{
					if (fCompound && fkp.rgfc[0] != fcLimPriorFkp)
						{
						fCompound = fFalse;
						goto LGetFcLim;
						}
					else
						Assert(fkp.rgfc[0] == fcLimPriorFkp);
					}
				else
					{
					Assert(fkp.rgfc[0] >= fcLimPriorFkp);
					}
				Assert((FC)CpPlc(hplcbte, ibte) == fcLimPriorFkp);
				}
LGetFcLim:
			fcLimPriorFkp = FcCkFkp(hfcb, &fkp, fPara, fScratch);
			if (fcLimPriorFkp == 0xffffffff)
				return;
			}

		pnPrev = bte.pn;
		}
	if (vdbs.fCkFkp)
		{
		Assert((FC)CpPlc(hplcbte, ibte) == fcLimPriorFkp);
		}

}	/* end CkHplcbte */


/* G E T   P N */
/* %%Function:FGetPn %%Owner:BRADV */
FGetPn(fScratch, pn, pchText)
int fScratch;
PN pn;
char *pchText;
{
	char szMessage[20], *pch;

	if (!fScratch || !FGetPnFromCache(fnScratch, pn, pchText))
		{
		pch = szMessage;
		pch += CchCopySz("pn ", pch);
		CchLongToPpch((long)pn, &pch);
		*pch = 0;
		vdbs.fcCur = ((FC)pn) * cbSector;
		if (!FReadFileRgch((CHAR HUGE *)(pchText), cbSector, szMessage,
				0))
			return fFalse;
		}
	return fTrue;
}       /* end GetPn */


/* F   G E T   P N   F R O M   C A C H E */
/* %%Function:FGetPnFromCache %%Owner:BRADV */
int FGetPnFromCache(fn, pn, pchText)
int fn;
PN pn;
char *pchText;
{
	int ibp, iibp;
	struct BPS HUGE *hpbps;
	extern struct BPTB vbptbExt;
	CHAR HUGE *HpBaseForIbp();
	Assert(fn >= fnScratch && mpfnhfcb[fn] != hNil);
	iibp = IibpHash(fn, pn, vbptbExt.iibpHashMax);
	ibp = (vbptbExt.hprgibpHash)[iibp];
/* search list of buffers with the same hash code */
	while (ibp != ibpNil)
		{
		hpbps = &vbptbExt.hpmpibpbps[ibp];
		if (hpbps->pn == pn && hpbps->fn == fn)
			{
/* page found in the cache */
			bltbh(HpBaseForIbp(ibp), pchText, cbSector);
			return fTrue;
			}
		ibp = hpbps->ibpHashNext;
		}
	return fFalse;
}	/* end FGetFkpFromCache */


/* F C   C K   F K P */
/* %%Function:FcCkFkp %%Owner:BRADV */
FC FcCkFkp(hfcb, pfkp, fPara, fScratch)
struct FCB **hfcb;
struct FKP *pfkp;
int fPara;
int fScratch;
	{ /*
	DESCRIPTION:
	Checks consistency of a FKP record.
	RETURNS:
	fc limit of FKP record (fcLim of last run).
*/
#define bitCrunByte	1
#define bitFcByte	2
#define lfcFlags	0x02020202L
#define bitBrunByte	4
#define bitByteRunFirst	8
#define bitByteRunOther	16
#define bitByteRunPad	32

	int irun, crun, brun, cch;
	char *pbrunD2;
	int crunUnused;
	/* array of grpf's, 1 for each byte of the FKP */
	char rggrpf[cbSector];
	char rgchText[cbSector];
	char chCur;

	/* initialize grpf to zero for each byte */
	SetBytes(rggrpf, 0, cbSector);

	/* crun byte */
	rggrpf[cbSector-1] |= bitCrunByte;
	Assert ((crun = pfkp->crun) > 0);
	Assert (pfkp->rgfc[0] < (*hfcb)->cbMac);

	/* fcLim of every run must be within confines of file's fc's
		and be less than the following run's fcLim. */
	for (irun = 0; irun < crun ; irun++)
		{
		/* bytes containing FC's */
		*((long *)&(rggrpf[sizeof(FC) * irun])) |= lfcFlags;
		Assert (pfkp->rgfc[irun] < pfkp->rgfc[irun + 1]);
		if (fPara)
			{
			if (!FGetPn(fScratch,
					(PN)((pfkp->rgfc[irun + 1] - 1) >> shftSector),
					&rgchText))
				return 0xffffffff;
			chCur = rgchText[(int)((pfkp->rgfc[irun + 1] - 1)
				& maskSector)];
			Assert(chCur == chEop || chCur == chSect || chCur == chTable);
			}
		pbrunD2 = ((char *)&(pfkp->rgfc[crun + 1])) + irun;
		/* bytes containing brun's */
		rggrpf[(int)pbrunD2 - (int)pfkp] |= bitBrunByte;
		/* with 512 byte sectors, runs start on word boundries and
			bruns are packed (I guess it's not a "b" anymore, is it?)*/
		if (brun = (*pbrunD2 * sizeof(int)))
			{
			/* make sure *pbrun is between fc's and crun */
			Assert ((crun + 1) * sizeof(long) <= brun
					&& brun < cbSector-1);
			/* first bytes of runs */
			rggrpf[brun] |= bitByteRunFirst;
			cch = ((char *)pfkp)[brun]
					<< (fPara && (*hfcb)->nFib >= 25);
			Assert (cch >= 0 && brun + 1 + cch < cbSector);
			if (fPara)
				{
				if (cch > cbPHE+1)
					{
					CkGrpprl(cch - (cbPHE + 1),
							&((char *)pfkp)[brun + cbPHE + 2],
							fTrue /* grpprl from papx */);
					}
				else
					Assert(cch == cbPHE + 1 || cch == 0);
				}
			else
				{
				Assert (cch <= sizeof(struct CHP));
				}
			/* non-first bytes of runs */
			while (cch--)
				rggrpf[++brun] |= bitByteRunOther;
			/* count the padding to the next even byte */
			if ((brun < cbSector - 3 && !(brun & 1)) || brun == cbSector - 3)
				rggrpf[++brun] |= bitByteRunPad;
			}
		}
	*((long *)&(rggrpf[sizeof(FC) * irun])) |= lfcFlags;
	crunUnused = 0;
	for (brun = 0; brun < cbSector; brun++)
		{
		if (brun && rggrpf[brun - 1] != 0 && rggrpf[brun] == 0)
			crunUnused++;
		Assert(
				rggrpf[brun] == 0	/* byte is unused */
		|| rggrpf[brun] == bitCrunByte
				|| rggrpf[brun] == bitFcByte
				|| rggrpf[brun] == bitBrunByte
				|| rggrpf[brun] == bitByteRunFirst
				|| rggrpf[brun] == bitByteRunOther
				|| rggrpf[brun] == bitByteRunPad
				);
		}
	/* Assert(crunUnused <= 1); */
	return (pfkp->rgfc[irun]);
#undef bitCrunByte
#undef bitFcByte
#undef lfcFlags
#undef bitBrunByte
#undef bitByteRunFirst
#undef bitByteRunOther
#undef bitByteRunPad

}       /* end FcCkFkp */


/* C K   F I B  */
/* %%Function:CkFib %%Owner:BRADV */
CkFib(pfib, hfcb)
struct FIB *pfib;
struct FCB **hfcb;
	{ /*
	DESCRIPTION:
	Check the File Information Block structure at the head of the file fn.
*/
	struct FCB *pfcb = *hfcb;

#ifdef NOCKCOMPOUND  /* until this stuff gets updated */
	if ((*hfcb)->fCompound)
		return;
#endif /* NOCKCOMPOUND */

	if (pfib->nFib < nFibCurrent)
		return;

	Assert (pfcb->cbMac >= cbFIB);
	Assert (pfib->wIdent == wMagic);
	Assert (pfib->fcMac > pfib->fcMin);
	Assert (pfib->ccpText >= cp0);
	Assert (pfib->ccpFtn >= cp0);
	Assert (pfib->ccpHdd >= cp0);
	Assert (pfib->ccpMcr >= cp0);
	Assert (pfib->ccpAtn >= cp0);
	Assert (pfib->pnNext == pn0 || pfib->fDot);
	Assert (!pfib->fGlsy || (pfib->pnNext == pn0 && !pfib->fDot));
	Assert (!pfib->fComplex || (!pfib->fGlsy && !pfib->pnNext));
	Assert (! (pfcb->fCompound && pfib->fComplex));
	Assert (!pfcb->fCompound || (pfib->fGlsy || pfib->fDot));
	Assert (pfcb->fCompound == (pfib->pnNext != pn0 || pfib->fGlsy));
	if (pfib->fComplex)
		{
		Assert(pfib->fcStshf == pfib->fcStshfOrig
				|| (pfib->fcStshf >
				pfib->fcStshfOrig + pfib->cbStshfOrig
				&& pfib->fcStshf + pfib->cbStshf
				<= pfib->fcPlcffndRef));
		/* check that pfib->fcStshfOrig == pfib->fcStshf ==>
		pfib->cbStshf <= pfib->cbStshfOrig */
		Assert(pfib->fcStshfOrig != pfib->fcStshf
				|| pfib->cbStshf <= pfib->cbStshfOrig);
		/* check that pfib->fcStshfOrig != pfib->fcStshf ==>
		pfib->cbStshf > pfib->cbStshfOrig */
		Assert(pfib->fcStshfOrig == pfib->fcStshf
				|| pfib->cbStshf > pfib->cbStshfOrig);
		}
	else
		{
		Assert (pfib->fcMin + pfib->ccpText + pfib->ccpFtn
				+ pfib->ccpHdd + pfib->ccpMcr <= pfib->fcStshfOrig);
		Assert(pfib->fcStshfOrig == pfib->fcStshf);
		Assert(pfib->cbStshfOrig == pfib->cbStshf);
		}
	Assert (pfib->cbStshf >= 0);
	if (pfib->fComplex)
		{
		Assert (pfib->fcStshfOrig + pfib->cbStshfOrig
				<= pfib->fcPlcffndRef);
		}
	else
		{
		Assert (pfib->fcStshfOrig + pfib->cbStshfOrig
				== pfib->fcPlcffndRef);
		}
	Assert (pfib->cbStshfOrig >= 0);
	Assert (pfib->fcPlcffndRef + pfib->cbPlcffndRef
			== pfib->fcPlcffndTxt);
	Assert (pfib->cbPlcffndRef >= 0);
	Assert (pfib->fcPlcffndTxt + pfib->cbPlcffndTxt
			== pfib->fcPlcfpgdFtn);
	Assert (pfib->cbPlcffndTxt >= 0);
	Assert (pfib->fcPlcfpgdFtn + pfib->cbPlcfpgdFtn
			== pfib->fcPlcfandRef);
	Assert (pfib->cbPlcfpgdFtn >= 0);
	Assert (pfib->fcPlcfandRef + pfib->cbPlcfandRef
			== pfib->fcPlcfandTxt);
	Assert (pfib->cbPlcfandRef >= 0);
	Assert (pfib->fcPlcfandTxt + pfib->cbPlcfandTxt
			== pfib->fcPlcfsed);
	Assert (pfib->cbPlcfandTxt >= 0);
	Assert (pfib->fcPlcfsed + pfib->cbPlcfsed == pfib->fcPlcfpgd);
	Assert (pfib->cbPlcfsed >= 0);
	Assert (pfib->fcPlcfpgd + pfib->cbPlcfpgd == pfib->fcPlcfphe);
	Assert (pfib->cbPlcfpgd >= 0);
	Assert (pfib->fcPlcfphe + pfib->cbPlcfphe == pfib->fcSttbfglsy);
	Assert (pfib->cbPlcfphe >= 0);
	Assert (pfib->cbSttbfglsy + pfib->cbPlcfglsy == 0 || pfib->fGlsy);
	Assert (pfib->fcSttbfglsy + pfib->cbSttbfglsy == pfib->fcPlcfglsy );
	Assert( pfib->cbSttbfglsy >= 0 );
	Assert( pfib->fcPlcfglsy + pfib->cbPlcfglsy == pfib->fcPlcfhdd );
	Assert( pfib->cbPlcfglsy >= 0 );
	Assert( pfib->fcPlcfhdd + pfib->cbPlcfhdd == pfib->fcPlcfbteChpx
			|| pfib->pnNext != pn0);
	Assert( pfib->cbPlcfhdd >= 0 );
	Assert( pfib->fcPlcfbteChpx + pfib->cbPlcfbteChpx
			== pfib->fcPlcfbtePapx );
	Assert( pfib->cbPlcfbteChpx > 0 );
	Assert( pfib->fcPlcfbtePapx + pfib->cbPlcfbtePapx == pfib->fcSttbfffn
			|| pfib->pnNext != pn0);
	Assert( pfib->cbPlcfbtePapx > 0 );
	Assert( pfib->fcSttbfffn + pfib->cbSttbfffn == pfib->fcPlcffldMom );
	Assert( pfib->cbSttbfffn >= 0 );
	Assert( pfib->fcPlcffldMom + pfib->cbPlcffldMom == pfib->fcPlcffldHdr );
	Assert( pfib->cbPlcffldMom >= 0 );
	Assert( pfib->fcPlcffldHdr + pfib->cbPlcffldHdr == pfib->fcPlcffldFtn );
	Assert( pfib->cbPlcffldHdr >= 0 );
	Assert( pfib->fcPlcffldFtn + pfib->cbPlcffldFtn == pfib->fcPlcffldAtn );
	Assert( pfib->cbPlcffldFtn >= 0 );
	Assert( pfib->fcPlcffldAtn + pfib->cbPlcffldAtn == pfib->fcPlcffldMcr );
	Assert( pfib->cbPlcffldAtn >= 0 );
	Assert( pfib->fcPlcffldMcr + pfib->cbPlcffldMcr == pfib->fcSttbfbkmk );
	Assert( pfib->cbPlcffldMcr >= 0 );
	Assert( pfib->fcSttbfbkmk + pfib->cbSttbfbkmk == pfib->fcPlcfbkf );
	Assert( pfib->cbSttbfbkmk >= 0 );
	Assert( pfib->fcPlcfbkf + pfib->cbPlcfbkf == pfib->fcPlcfbkl );
	Assert( pfib->cbPlcfbkf >= 0 );
	Assert( pfib->fcPlcfbkl + pfib->cbPlcfbkl == pfib->fcCmds );
	Assert( pfib->cbPlcfbkl >= 0 );
	Assert( pfib->fcCmds + pfib->cbCmds == pfib->fcPlcmcr );
	Assert( pfib->cbCmds >= 0 );
	Assert( pfib->fcPlcmcr + pfib->cbPlcmcr == pfib->fcSttbfmcr );
	Assert( pfib->cbPlcmcr >= 0 );
	Assert( pfib->fcSttbfmcr + pfib->cbSttbfmcr == pfib->fcPrEnv );
	Assert( pfib->cbSttbfmcr >= 0 );
	Assert( pfib->fcPrEnv + pfib->cbPrEnv == pfib->fcWss );
	Assert( pfib->cbPrEnv >= 0);
	Assert( pfib->fcWss + pfib->cbWss == pfib->fcClx );
	Assert( pfib->cbWss >= 0);
	Assert( pfib->fcClx + pfib->cbClx == pfib->fcDop );
	Assert( pfib->cbClx >= 0 );
	Assert( pfib->fcDop + pfib->cbDop == pfib->fcSttbfAssoc );
	Assert( pfib->cbDop >= 0 );
	Assert( pfib->fcSttbfAssoc + pfib->cbSttbfAssoc == pfib->cbMac
			|| pfcb->fCompound );
/*        Assert( pfib->cbMac == pfcb->cbMac ||
			( (pfib->cbMac >> shftSector) == 
		(pfcb->cbMac >> shftSector)-1 )); BOGUS ASSERT! (BAC) */
	if (pfib->fComplex)
		{
		Assert (pfib->cbClx > 0);
		}
	else
		{
		Assert (pfib->cbClx == 0);
		}
} /* end CkFib */


/* C K   P B P T B */
/* %%Function:CkPbptb %%Owner:BRADV */
CkPbptb(pbptb, fAbortOK)
struct BPTB *pbptb;
BOOL fAbortOK;
	{ /*
	DESCRIPTION:
	Checks consistency of the file page cache pbptb.
*/
	int ibp, ibpList, iibpHash;
	struct BPS HUGE *hpbps;
	struct BPS HUGE *hpbpsList;
	int cibpOccupied;
	int cibpHashNext;
	int ciibpNonNil;
	int fInList;

	if (!vdbs.fCkBptb) return;
	Assert(pbptb->ibpMac <= pbptb->ibpMax);

	/* for each of the occupied pages in the cache:
		- check if valid fn and pn.
		- check that page can be found using the hash table */
	cibpHashNext = 0;
	cibpOccupied = 0;
	for (ibp = 0, hpbps = (&(pbptb->hpmpibpbps[0]));
			ibp < pbptb->ibpMac; ibp++, hpbps++)
		{
		if (fAbortOK && FMsgPresent(mtyIdle))
			return;

		if (hpbps->fn != fnNil)
			{
			Assert(FValidFn(hpbps->fn));
			Assert(vdbs.fQuicksave || !PfcbFn(hpbps->fn)->fHasFib
					||  hpbps->pn <=
					(PN) (CpMax(PfcbFn(hpbps->fn)->cbMac,
					PfcbFn(hpbps->fn)->fcMacFile) / pbptb->cbBp));

			ibpList = pbptb->hprgibpHash[IibpHash(hpbps->fn,
								hpbps->pn, pbptb->iibpHashMax)];
			/* search list of buffers with the same hash code */
			fInList = fFalse;
			while (ibpList != ibpNil)
				{
				hpbpsList = (&(pbptb->hpmpibpbps [ibpList]));
				if (hpbpsList->pn == hpbps->pn
						&& hpbpsList->fn == hpbps->fn)
					{
					/* page found in the cache */
					Assert(!fInList);
					fInList = fTrue;
					}
				ibpList = hpbpsList->ibpHashNext;
				}
			Assert(fInList);

			cibpOccupied++;
			if (hpbps->ibpHashNext != ibpNil)
				cibpHashNext++;
			}
		}

	/* how many entries are in the hash table? */
	ciibpNonNil = 0;
	for (iibpHash = 0; iibpHash < pbptb->iibpHashMax; iibpHash++)
		if (pbptb->hprgibpHash[iibpHash] != ibpNil)
			ciibpNonNil++;
	/* the number of ibp's with a Hash Next field plus the number of
		entries in the hash table must equal the number of occupied
		file cache pages. */
	Assert(cibpHashNext + ciibpNonNil == cibpOccupied);
}       /* end CkFileCache */


/* %%Function:FValidFn %%Owner:BRADV */
FValidFn(fn)
int fn;
	{ /*
	DESCRIPTION:
	Returns fTrue if fn valid.
*/
	if (fn == fnNil)
		return fTrue;
	if (fn < 0 || fn >= fnMac)
		return fFalse;
	if (!mpfnhfcb[fn])
		return fFalse;
	return(fTrue);
}       /* end ValidFn */


/* %%Function:CchReadHpch %%Owner:BRADV */
CchReadHpch( osfn, hpch, cchRead )
unsigned osfn;
CHAR HUGE *hpch;
unsigned cchRead;
{
	int cchReturn;

	cchReturn = CchReadDoshnd(osfn, LpLockHp(hpch), cchRead);
	UnlockHp( hpch );
	return cchReturn;
}


#endif /* DEBUG */
