/*#define DINTERP*/

/* elsubs2.c -- Support for the EL interpreter */

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "debug.h"
#include "heap.h"
#include "rerr.h"
#include "cmdtbl.h"
#include "el.h"
#include "idd.h"
#include "opuscmd.h"
#include "macrocmd.h"
#include "ch.h"
#include "version.h"
#include "props.h"
#include "prm.h"
#include "doc.h"
#include "sel.h"
#include "disp.h"
#include "dde.h"
#include "keys.h"
#include "help.h"
#include "rareflag.h"

extern struct MERR vmerr;
extern struct SEL selCur;
extern vfSeeSel;
extern int vcElParams;
extern BOOL vfElFunc;
extern MES ** vhmes;
extern struct CA caPara;
extern struct WWD ** mpwwhwwd [];
extern struct WWD ** hwwdCur;
extern SB sbTds;

SB sbMacSavEdMacro = sbNil;
BOOL vfElDisableInput;

struct SEL selMacro;

#define FEditHpeli(hpeli) ((hpeli)->w == 0xffff)
#define ImeiFromEditHpeli(hpeli) (LOWORD((hpeli)->l))


/* C P  M A C R O  L I N E  I B */
/* Returns the cpText of the ibToken for the specified macro. */
/* %%Function:CpMacroLineIb %%Owner:bradch */
CP CpMacroLineIb(imei, ib)
int imei;
uns ib;
{
	MEL * rgmel;
	MEL * pmel;
	MEI * pmei;
	struct CA ca;
	int imelMin, imelLim, imelGuess;

	Assert(vhmes != hNil);

	pmei = PmeiImei(imei);
	if (pmei->fNotExpanded && !FExpandHqrgbToPca(pmei->hqrgbTokens, 
			pmei->cbTokens, PcaSetWholeDoc(&ca, pmei->docEdit), imei, fFalse))
		{
		return cpNil; /* as hrgmel may not have been set up */
		}

	PmeiImei(imei)->fNotExpanded = fFalse;

	pmei = PmeiImei(imei);
	rgmel = *pmei->hrgmel;
	Assert(rgmel != hNil);
	imelMin = 0;
	imelLim = pmei->imelMac;
	pmel = pNil;
	while (imelMin < imelLim)
		{
		imelGuess = (imelMin + imelLim) >> 1;

		pmel = &rgmel[imelGuess];
		if (pmel->ib == ib)
			return (CP) pmel->cp;

		if (pmel->ib > ib)
			imelLim = imelGuess;
		else
			imelMin = imelGuess + 1;
		}

	Assert(pmel != pNil);
	Assert((1 > 0) == 1);
	return (CP) (pmel - (pmel->ib > ib))->cp;
}



int vceliNest = 0;


/* F  A L L O C  M A C R O  S B S */
/* %%Function:FAllocMacroSbs %%Owner:bradch */
FAllocMacroSbs()
{
	vceliNest += 1;

	return fTrue;
}


/* F R E E  M A C R O  S B S */
/* %%Function:FreeMacroSbs %%Owner:bradch */
FreeMacroSbs()
{
	Assert(vceliNest != 0);

	if ((vceliNest -= 1) == 0)
		CleanupEl();
}


/* C C H  R E A D  E D I T */
/* Token fetching EL callback for macros being edited */
/* %%Function:CchReadEdit %%Owner:bradch */
WORD CchReadEdit(pb, cbReq, heli)
char * pb;
WORD cbReq;
ELI ** heli;
{
	ELI huge * hpeli;
	char far * lp;
	long lib;
	int imei;
	MEI * pmei;

	hpeli = HpeliOfHeli(heli);

	Assert(FEditHpeli(hpeli));

	/* Macro being edited */

	imei = LOWORD(hpeli->l);

	pmei = PmeiImei(imei);
	Assert(pmei->hqrgbTokens != hqNil);

	if ((lib = hpeli->lib) > pmei->cbTokens)
		return 0;

	if (lib + cbReq > pmei->cbTokens)
		cbReq = pmei->cbTokens - lib;
	if (cbReq > 0)
		{
		lp = (char far *) LpFromHq(pmei->hqrgbTokens) + lib;
		bltbx(lp, (char far *) pb, cbReq);
		}

	return cbReq;
}


/* C C H  R E A D  S O U R C E */
/* %%Function:CchReadSource %%Owner:bradch */
WORD CchReadSource(pb, cbReq, heli)
char * pb;
WORD cbReq;
ELI ** heli;
{
	uns cbGot, cbLeft;
	LONG libReq;
	struct CA ca;
	CP cpFirst, cpLim;
	int doc;
	ELI huge * hpeli;

	hpeli = HpeliOfHeli(heli);

	Assert(!FEditHpeli(hpeli));

	/* Normal macro */
	CaFromIhdd(LOWORD(hpeli->l), hpeli->w, &ca);
	cpFirst = ca.cpFirst;
	cpLim = ca.cpLim;
	doc = ca.doc;

	/* Very big programs are not suported yet! */
	Assert(hpeli->lib <= 0x0000ffffL);

	libReq = hpeli->lib + cpFirst;

	if (libReq >= cpLim)
		return 0;

	cbLeft = cpLim - libReq;
	cbGot = (cbLeft < cbReq) ? cbLeft : cbReq;
	FetchRgch(&cbGot, pb, doc, libReq, libReq + cbGot, cbGot);

	return cbGot;
}



/* G E T  I N F O  E L X */
/* Copy info from SY to ELE */
/* %%Function:GetInfoElx %%Owner:bradch */
GetInfoElx(elx, pele, heli, fFunction, cmm)
ELX elx;
ELE * pele;
ELI ** heli;
BOOL fFunction;
int cmm;
{
	extern int cmmCache;
	extern BOOL vfExtendSel, vfBlockSel;
	BYTE * pelp;
	char * pelpSrc;
	int ielu, mct;
	SY * psy;
	BCM bcm;

	vfElFunc = fFunction;
	
	bcm = BcmFromIbcm(elx);

	if (fFunction && bcm == imiCalculate)
		{
		extern NUM ElUtilCalculate();
		pele->fFunc = fTrue;
		pele->fStmt = fFalse;
		pele->elr = elrNum;
		pele->ieldi = ieldiNil;
		pele->pfn = ElUtilCalculate;
		pele->celpMin = 0;
		pele->celpMac = 1;
		pele->rgelp[0] = elpHst;
		return;
		}

	FetchSy(bcm);
	cmmCache = cmmNormal; /* so FLegalBcm won't fetch a new symbol */
	if (!FLegalBcm(bcm))
		{
		void ModeError();

		pele->elr = elrVoid;
		pele->pfn = ModeError;
		pele->celpMin = 0;
		pele->celpMac = 0;

		return;
		}

	PrepExecBcm(bcm);
	
	FetchSy(bcm);
	psy = vpsyFetch;

	if ((mct = psy->mct) == mctCmd || mct == mctSdm)
		{
		pele->elr = mct == mctCmd ? elrVoid : elrDialog;
		pele->pfn = psy->pfnCmd;
		pele->celpMin = 0;
		pele->celpMac = 0;
		if (mct == mctSdm)
			pele->ieldi = IeldiFromPsy(psy);
		pele->fStmt = fTrue;
		pele->fFunc = fFalse;
		}
	else if (FElMct(mct))
		{
		pele->fFunc = fTrue;
		pele->fStmt = psy->fStmt;
		pele->ieldi = ieldiNil;

		if (fFunction)
			{
			if ((pele->elr = psy->elr) == elrVoid)
				pele->fFunc = fFalse;
			}
		else
			{
			pele->elr = elrVoid;
			}

		pele->pfn = psy->pfnEl;
		pele->celpMin = psy->cagdMin;
		pele->celpMac = psy->cagdMax;
		pelpSrc = (ARD *) (&psy->stName[psy->stName[0] + 1]);
		if (psy->stName[0] == 0)
			(WORD *) pelpSrc += 1;
		bltb(pelpSrc, pele->rgelp, psy->cagdMax);
		}
#ifdef DEBUG
	else
		{
		Assert(fFalse);
		}
#endif

	if (mct == mctCmd || mct == mctSdm)
		{
 		FetchSy(bcm);
 		cmmCache = cmmNormal; /* so FLegalBcm won't fetch a new symbol */
		PrepAgainBcm(bcm);
		}
}


/* F  S E N D  K E Y S  P E N D I N G */
/* %%Function:FSendKeysPending %%Owner:bradch */
FSendKeysPending()
{
	extern HEVT FAR *lphevtHead;
	BOOL fKeys = fFalse;

	if (lphevtHead != NULL && *lphevtHead != NULL)
		{
		EVT FAR *lpevt;
		lpevt = GlobalLock(*lphevtHead);
		Assert(lpevt != NULL);
		fKeys = lpevt->ieventMac > 0;
		GlobalUnlock(*lphevtHead);
		}
	return fKeys;
}



/* E L A  D E B U G */
/* %%Function:ElaDebug %%Owner:bradch */
ElaDebug(eld, heli)
ELD eld;
ELI ** heli;
	/* CSNative as called for every macro command */
{{/* NATIVE - ElaDebug */
	extern int vcSubNest;
	int imei;
	ELI huge * hpeli;
	extern int wwCur;
	extern long tickNextYield;
	extern int vlm;
	extern BOOL vfDeactByOtherApp;
	extern struct SEL selDotted;

	hpeli = HpeliOfHeli(heli);

	switch (eld)
		{
	case eldSubOut:
		vcSubNest -= 1;
		goto LCheckBreak;

	case eldSubIn:
		vcSubNest += 1;
		goto LCheckBreak;

	case eldNormalStep:
		/* TopOfMainLoop resets many flags, must check before calling */
		if (vmerr.fMemFail || vmerr.fDiskFail)
			RtError(rerrOutOfMemory);

		TopOfMainLoop();

		if (vlm != lmPreview)
			{
			if (hwwdCur != hNil)
				{
				/* update windows only if current one dirty */
				if ((*hwwdCur)->fDirty)
					UpdateAllWws(fFalse /* fAbortOK */);
			
				if ((*hwwdCur)->fNeedEnhance && FEnhanceWw(wwCur))
					(*hwwdCur)->fNeedEnhance = fFalse;
				
				if (vfSeeSel)
					{
					SeeSel();
					vfSeeSel = fFalse;
					}
				}

			if (!vfDeactByOtherApp && 
					(vhmes == hNil || !(*vhmes)->fAnimate || 
					selCur.doc != selMacro.doc))
				{
				if (!selCur.fNil && selCur.fHidden)
					TurnOnSelCur();
				if (!selDotted.fNil && selDotted.fHidden)
					TurnOnSel(&selDotted);
				}
			}

		if (vmerr.fMemFail || vmerr.fDiskFail)
			RtError(rerrOutOfMemory);

		if ((vfDeactByOtherApp || GetTickCount() > tickNextYield) && 
				!FSendKeysPending())
			{
			OurYield(fFalse);
			}

		if (FEditHpeli(hpeli))
			{
			/* We are running a macro in a window */
			Assert(vhmes != hNil);
			imei = ImeiFromEditHpeli(hpeli);

			if (imei != (*vhmes)->imeiCur)
				SetImeiCur(imei);

			if ((*vhmes)->fAnimate)
				{
				CP cp = CpMacroLineIb(imei, (uns) hpeli->lib);

				if (cp == cpNil)
					RtError(rerrHalt);
				else
					SelectMacroPara(imei, cp);
				}

			if ((*vhmes)->fStep || 
					((*vhmes)->fStepSUBs && vcSubNest == 0))
				{
				StopHeli(heli, rerrSuspend);
				}
			}
LCheckBreak:
		/* FCheckAbortKey is always called to keep key queue clear */
		if (FCheckAbortKey() && !vfElDisableInput)
			RtError(rerrStop);
		/* break; IE */
		}
}}


/* S E L E C T  M A C R O  P A R A */
/* If cp < 0, just removes hilighting */
/* FUTURE: would be cool if this checked for a line cont. char at eop
and extend selection ad infinitum... */
/* %%Function:SelectMacroPara %%Owner:bradch */
SelectMacroPara(imei, cp)
int imei;
CP cp;
{
	MEI * pmei;
	int doc, ww;
	struct CA  caT;
	extern int wwCur;

	pmei = PmeiImei(imei);
	doc = pmei->docEdit;
	ww = PmwdMw(pmei->mw)->wwActive;

	Assert(cp > 0);
	Assert(ww != wwNil);
	Assert(wwCur != wwNil);
	Assert((*vhmes)->fAnimate);

	if (FWindowHidden(ww))
		return;

	if (ww==wwCur)
		{
		pmei->fHlt=fFalse;
		if ((*vhmes)->fCanCont) /* macro stepped from its own window */
			TurnOffSel(&selCur);
		}

	if (selMacro.doc != doc) /* when macro window animated for the first
	time or thru another macro window */
		{
		if (selMacro.doc != docNil)
			TurnOffSel(&selMacro);
		*((struct SELS *) &selMacro) = PwwdWw(ww)->sels;
		selMacro.fHidden = fTrue;
		}

	selMacro.ww = ww;

	TurnOffSel(&selMacro);

	if (cp >= CpMacDocEdit(doc)) /* can only happen when a macro line is
	deleted by a previous macro command */
		{
		Assert(ww==wwCur);
		Assert(selMacro.doc==doc);
		return;
		}

	CacheSavePara(doc, cp, &caT);
	NormCp(ww, doc, caPara.cpFirst, ncpVisifyPartialDl + ncpHoriz, 
			DyOfRc(&PwwdWw(ww)->rcwDisp) / 4, fFalse);
	Select(&selMacro, caT.cpFirst, caT.cpLim);
	TurnOnSel(&selMacro);

	if (ww != wwCur) /* macro being stepped/traced from a diff window */
		{
		blt(&selMacro, &PwwdWw(ww)->sels, cwSELS);
		PmeiImei(imei)->fHlt=fTrue;
		}
}


/* H I L I G H T  M A C R O  P A R A */
/* %%Function:HilightMacroPara %%Owner:bradch */
HilightMacroPara(imei, cp)
int imei;
CP cp;
{
	MEI * pmei;
	int doc, ww;
	char rgb [2];
	struct CA caT;
	extern int vlm;

	pmei = PmeiImei(imei);
	doc = pmei->docEdit;
	ww = PmwdMw(pmei->mw)->wwActive;
	pmei->cpHlt = cp;

	/* Can't handle making selections while in preview mode */
	if (vlm == lmPreview)
		EndPreviewMode();

	Assert(!FWindowHidden(ww));

	CacheSavePara(doc, cp, &caT);
	NormCp(ww, doc, caPara.cpFirst, ncpVisifyPartialDl + ncpHoriz, 
			DyOfRc(&(*mpwwhwwd[ww])->rcwDisp) / 4, fFalse);

	rgb[0] = sprmCIco;
	rgb[1] = icoRed;
	if (selCur.doc == caT.doc)
		{
		Select(&selCur, caT.cpFirst, caT.cpLim - ccpEop);
		ApplyGrpprlSelCur(rgb, 2, fFalse /* fSetUndo */);
		}
	else
		{
		ApplyGrpprlCa(rgb, 2, &caT);
		InvalCp(&caT);
		}
}



/* T E R M  M A C R O */
/* %%Function:TermMacro %%Owner:bradch */
TermMacro(heli)
ELI ** heli;
{
	MSG msg;

	vfElFunc = fFalse;
	vcElParams = 0;
	FreeHeli(heli);
}


/* C M D  E X E C  M A C R O  D O T  I M C R */
/* This is called from the main command dispatcher (CmdExecCmb) to invoke
	a macro. */
/* %%Function:CmdExecMacroDotImcr %%Owner:bradch */
CMD CmdExecMacroDotImcr(docDot, imcr)
int docDot;
WORD imcr;
{
	extern int GetInfoElx();
	extern ELI ** HeliNew();
	extern int vcElParams;
	extern BOOL vfMcrRunning;


#ifdef DEBUG
	extern int vfInCmd;
	int fInCmdSave = vfInCmd;
#endif /* DEBUG */

	ELI ** heli;
	RERR rerr;
	CMD cmdReturn;
	int imei;

	cmdReturn = cmdError;

	/* Prevent recursion... */
	if (FRunningElgElm(docDot, imcr))
		{
		ModeError();
		goto LReturn;
		}

#ifdef DEBUG
	if (!vdbs.fFailInMcr)
		vfInCmd = 0;
#endif /* DEBUG */


	/* Terminate completely any suspended macros as the poor interpreter
		cannot handle creating a new one then... */
	if (vhmes != hNil && (*vhmes)->fCanCont && FSuspended(vhmes))
		FreeEditMacro(iNil);


	/* If the macro is open for editing, run it from there... */
	if ((imei = ImeiFromElgElm(docDot, imcr)) != iNil)
		{
		MES * pmes;
		BOOL fStepSav, fStepSUBsSav;

		/* The called macro will not be single stepped... */
		pmes = *vhmes;
		fStepSav = pmes->fStep;
		fStepSUBsSav = pmes->fStepSUBs;
		pmes->fStep = fFalse;
		pmes->fStepSUBs = fFalse;

#ifdef REDUNDANT
		SetImeiCur(imei);
#endif
		PmeiImei(imei)->fRestartable = fFalse;
		cmdReturn = CmdCallInterp(imei);
		PmeiImei(imei)->fRestartable = fTrue;

		pmes = *vhmes;
		pmes->fStep = fStepSav;
		pmes->fStepSUBs = fStepSUBsSav;

		goto LReturn;
		}


	if (!FAllocMacroSbs())
		{
		cmdReturn = cmdNoMemory;
		goto LReturn;
		}


	Assert(PdodDoc(docDot)->docMcr != docNil);

	heli = HeliNew((WORD)docDot, imcr, CchReadSource,
			ElaDebug, GetInfoElx, imcr, (long) PdodDoc(docDot)->docMcr);
	if (heli == (ELI **) 0)
		{
		cmdReturn = cmdNoMemory;
		goto LFreeSbs;
		}

	ClearAbortKeyState();

	vfMcrRunning = fTrue;
	/* keep template from being closed */
	PdodDoc(docDot)->crefLock++;
	StartLongOp();
	rerr = RerrRunHeli(heli);
	EndLongOp(fFalse);
	Assert(PdodDoc(docDot)->crefLock > 0);
	if (--PdodDoc(docDot)->crefLock == 0)
		DisposeDoc(docDot);
	vfMcrRunning = fFalse;

	FMacroErrMsg(heli, rerr);

	TermMacro(heli);
	cmdReturn = cmdOK;

LFreeSbs:
	FreeMacroSbs();

LReturn:
	Debug(vfInCmd = fInCmdSave);
	return cmdReturn;
}



/* F  S U S P E N D E D */
/* The following function checks whether any instance of the current
	editing state is suspended or not. */
/* %%Function:FSuspended %%Owner:bradch */
BOOL FSuspended(hmes)
MES **hmes;
{
	int imei;

	Assert(hmes != hNil);

	for (imei=0; imei < (*hmes)->imeiMax; ++imei)
		if (PmeiImei(imei)->fRunning)
			return fFalse;

	return fTrue;
}


/* K I L L  S E L  M A C R O */
/* %%Function:KillSelMacro %%Owner:bradch */
KillSelMacro()
{
	if (selMacro.doc != docNil)
		{
		TurnOffSel(&selMacro);
		selMacro.doc = docNil;
		}
}


/* C A C H E S A V E P A R A */
/* cache and save para props */
/* %%Function:CacheSavePara %%Owner:bradch */
CacheSavePara(doc, cp, pca)
int doc;
CP cp;
struct CA *pca;
{
	CachePara(doc, cp);
	blt(&caPara, pca, cwCA);
}
 

/* E L  S E T  A G A I N  C A B */
/* %%Function:ElSetAgainCab %%Owner:bradch */
ElSetAgainCab(bcm, hcab)
BCM bcm;
HCAB hcab;
{
	extern struct AAB vaab;
	if (vaab.bcm == bcmNil && FRepeatableBcm(bcm))
		SetAgainCab(bcm, HcabDupeCab(hcab));
}
