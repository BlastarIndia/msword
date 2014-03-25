/* C O M M A N D 2 . C */
/*  Command processing code */


#define RSHDEFS
#define OEMRESOURCE /* So we get OBM_CLOSE from qwindows.h */
#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "debug.h"
#include "menu2.h"
#include "disp.h"
#include "doc.h"
#define NOEXTERN
#include "cmdtbl.h"
#include "ch.h"
#include "field.h"
#include "print.h"
#include "props.h"
#include "sel.h"
#include "keys.h"
#include "recorder.h"
#include "screen.h"
#include "status.h"
#include "el.h"
#include "sdmtmpl.h"
#include "layout.h"
#include "preview.h"
#include "macrocmd.h"
#define RULER
#define REPEAT
#include "ruler.h"
#include "fontwin.h"
#include "help.h"
#include "idd.h"
#include "tmc.h"
#include "rareflag.h"

#define MPATMST
#include "automcr.h"

#define VERBFIRST

#define USEBCM
#include "cmd.h"

#include "runmacro.hs"
#include "runmacro.sdm"

/* G L O B A L S */
struct UAB      vuab;
struct AAB 	vaab;
HPSYT           vhpsyt = hpNil;
KMP **          hkmpCur;
KMP **          hkmpBase;
CMB		vcmb;
FARPROC         vlppFWHChain;
FARPROC         lppFWHKey;
struct STTB **  hsttbMenu;
MUD **          vhmudBase;
MUD **          vhmudUser;
KMP **          vhkmpUser;



/* E X T E R N A L S */
extern BOOL vfExtendSel;
extern BOOL vfBlockSel;
extern BOOL vfRecording;
extern BOOL vfElFunc;
extern int vcElParams;
extern HPSYT vhpsyt;
extern struct SEL selCur;
extern HWND vhwndStatLine;
extern int docGlobalDot;
extern BOOL vfInsertMode;
extern struct AAB vaab;
extern struct WWD ** hwwdCur;
extern int vlm;
extern RRF rrf;
extern PVS vpvs;
extern BOOL vfRecordNext;
extern BOOL vfSingleApp;
extern BOOL vfFileCacheDirty;
extern BOOL vfWndCacheDirty;
extern CHAR szNone[];
extern int  vssc;
extern struct CA caPara;
extern BOOL vfSearchRepl;
extern BOOL vfDefineStyle;
extern HANDLE vhInstance;
extern int	vfLastCursor;
extern int	vcConseqScroll;
extern int	sbScroll;
extern int	sbScrollLast;

/* L O C A L  F U N C T I O N S */

BCM BcmOfSt();
NATIVE uns WHash(); /* DECLARATION ONLY */
ELG ElgFromDoc();
HPSY HpsyFromBcmElg();
PFN PfnFromBsy();
CMD CmdExecBcmKc();
CMD CmdExecCmb();


extern struct STTB      **hsttbMenu;
extern struct SCI       vsci;
extern KMP ** hkmpCur;
extern int mwCur;
extern HWND vhwndRibbon;
extern HWND vhwndStatLine;
extern HWND vhwndCBT;
extern struct STTB ** vhsttbOpen;
extern struct STTB ** vhsttbWnd;
extern struct PRI vpri;
extern struct MERR vmerr;
extern int cfLink;
extern int cfRTF;
extern struct SAB vsab;
extern struct UAB vuab;
extern struct PREF vpref;
extern HANDLE vhMenuLongFull;
extern HMENU vhMenu;
extern int viMenu;
extern HWND vhwndApp;
extern struct MWD ** mpmwhmwd [];
extern struct WWD ** mpwwhwwd [];
extern int    wwCur;
extern BOOL vfRecording;
extern struct MWD ** hmwdCur;
extern MES ** vhmes;
extern KME vkme;
extern struct PAP vpapFetch;

extern int vgrfMenuCmdsAreDirty;
extern int vgrfMenuKeysAreDirty;
extern int docDotMudCur;
extern int vkcPrev;
extern int vfSysMenu;
extern BCM vbcmFetch;
extern CHAR vrgchsyFetch[];


KME * PkmeFromKc();
KME * PkmeOfKcInChain();

SY * PsyGetSyOfBsy();

#ifdef NOASM
NATIVE IScanLprgw( int far *, WORD, int ); /* DECLARATION ONLY */
#endif /* NOASM */

extern HBITMAP hbmpSystem; /* System menu bitmap */

#define FWInWloWhi(w, wLo, wHi) ((uns) ((w) - (wLo)) <= (wHi) - (wLo))

#define CatCsSt(csstFrom, stTo) \
	(bltbx((csstFrom) + 1, (char far *) ((stTo) + 1 + (stTo)[0]), \
		(csstFrom)[0]), \
	(stTo)[0] += *(csstFrom))


/* C B  H P S Y */
/* %%Function:CbHpsy %%Owner:bradch */
CbHpsy(hpsy)
HPSY hpsy;
{
	int cb = CbLpsy(LpLockHp(hpsy));
	UnlockHp(hpsy);
	return cb;
}


/* %%Function:CbLpsy %%Owner:bradch */
CbLpsy(lpsy)
SY FAR *lpsy;
{
	int cb;

	if ((cb = lpsy->stName[0]) == 0)
		cb += sizeof (uns); /* for back pointer */

	cb += cbSY;

	switch (lpsy->mct)
		{
	case mctSdm:
		cb += 2 * sizeof (uns); /* for cabi and ieldi */
		break;

	case mctCmd:
		break;

	case mctEl:
		cb += lpsy->cagdMax * cbAGD;
		break;

	case mctMacro:
	case mctGlobalMacro:
		/* break; */;
		}

#ifdef DEBUG
	if (cb > cbMaxSy)
/* %%Function:MessageBox %%Owner:bradch */
		MessageBox(vhwndApp, lpsy->stName+1, 
				(LPSTR) SzShared("CbLpsy: Bad sy!"), MB_OK|MB_SYSTEMMODAL);
#endif
	Assert(cb <= cbMaxSy);

	return cb;
}


/* P S Y  G E T  S Y  F R O M  B S Y */
/* %%Function:PsyGetSyOfBsy %%Owner:bradch */
SY * PsyGetSyOfBsy(psy, bsy)
CHAR * psy;
uns bsy;
{
	HPSY hpsy;
	hpsy = vhpsyt->grpsy + bsy;
	bltbh(hpsy, (CHAR HUGE *)psy, CbHpsy(hpsy));
	return psy;
}



#ifdef DEBUG
int vfInCmd = fFalse;
#endif

/* F  E X E C  K C */
/* %%Function:FExecKc %%Owner:bradch */
BOOL FExecKc(kc)
int kc;
{
	KME * pkme;
	BOOL fTranslated;

#ifdef DKEYMAP
	CommSzNum(SzShared("FExecKc: "), kc);
#endif

#ifdef DEBUG
	if (FProcessDbgKey (kc))
		return fTrue;
#endif /* DEBUG */

	if ((pkme = PkmeFromKc(kc, &fTranslated)) == 0)
		{
		vkme.kt = ktNil;
		return fTranslated;
		}

	vkme = *pkme;

#ifdef DKEYMAP
	CommSzNum(SzShared("kc: "), vkme.kc);
	CommSzNum(SzShared("kt: "), vkme.kt);
	CommSzNum(SzShared("w: "), vkme.w);
#endif

	switch (pkme->kt)
		{
	case ktFunc:            /* actual C function */
		Debug( vfInCmd++ );
		(*pkme->pfn)();
		Debug( vfInCmd-- );
		return fTrue;

	case ktMacro:           /* something in the command table */
		FetchCm(pkme->bcm);
		if (vbcmFetch == bsyNil) /* Deleted or renamed macro */
			goto LBeep;


		/* if CBT installed, must send this message before we
			process the key, so they can mask it out if they want to */

		if (vhwndCBT && !SendMessage(vhwndCBT, WM_CBTSEMEV,
				smvCommand, MAKELONG(CxtFromBcm(pkme->bcm), 1 /* from keyboard */)))
			return fTrue; /* yes it was processed */

		/* CBT returned true; do normal handling */


		switch (pkme->bcm)
			{
		case bcmTraceMacro:
		case bcmStepMacro:
		case bcmContinueMacro:
		case bcmAnimateMacro:
			PostMessage(vhwndApp, WM_COMMAND, pkme->bcm, 0L);
			return fTrue;
			}

#ifdef DEBUG
			{
			CMD cmd;

			cmd = CmdExecBcmKc(pkme->bcm, kc);
			return cmd == cmdOK;
			}

#else
		return CmdExecBcmKc(pkme->bcm, kc) == cmdOK;
#endif

	case ktBeep:		/* just beep */
LBeep:
		Beep();
		/* FALL THROUGH */

	case ktIgnore:		/* totally ignore (rare) */
		return fTrue;
		}

	return fFalse;
}


/* F  P R O C E S S  K E Y */
/* This function handles all non-insert type keys.  Returns true iff
key was handled */
/* %%Function:FProcessKey %%Owner:bradch */
BOOL FProcessKey(kc)
int kc;
{
#ifdef DKEYMAP
	CommSzNum(SzShared("FProcessKey: "), kc);
#endif /* DKEYMAP */

/* Check for cursor movement key */

	if (FCursKey(kc, fTrue /* fDoIt */))
		return fTrue;

	return (FExecKc(KcModified(kc)));
}



/* P K M E  F R O M  K C */
/* Searches the current keymap chain for the given kc.  fDoIt indicates
whether this kc is supposed to be acted upon as opposed to just looked
up.  fDoIt is only false in insert the loop so that keys may be checked
for whether or not they abort the insert loop. */
/* %%Function:PkmeFromKc %%Owner:bradch */
KME * PkmeFromKc(kc, pfTranslated)
int kc;
BOOL * pfTranslated;
{
	KMP ** hkmp, * pkmp;
	int ikme, kt;
	BOOL fStopHere;

	*pfTranslated = fFalse;
	hkmp = hkmpCur;
	while (hkmp != hNil)
		{
		pkmp = *hkmp;
		if (FSearchKmp(hkmp, kc, &ikme))
			return &pkmp->rgkme[ikme]; /* FOUND! */

		if (pkmp->fTranslate && !(FCtrlKc(kc) && !FAltKc(kc)))
			{
			extern MSG vmsgLast;
			/* NOTE: assumes last message was WM_KEYDOWN */
			if (TranslateMessage((LPMSG) &vmsgLast))
				{
				/* TRANSLATED!  Ignore key message, we'll
					get a CHAR message rsn... */
				DispatchMessage((LPMSG) &vmsgLast);
				*pfTranslated = fTrue;
				return 0;
				}
			}

		if (pkmp->fBeep)
			Beep();

		fStopHere = pkmp->fStopHere;

		if (pkmp->fModal)
			{
			EndKeyMode();
			hkmp = hkmpCur;
			}
		else
			hkmp = pkmp->hkmpNext;

		if (fStopHere)
			break;
		}

	return 0;
}


/* F  S E A R C H  K M P */
/* Binary search a keymap for a given kc.  Sets *pikme to where kc is
found (returns fTrue), or where it would be (returns fFalse). */
/* %%Function:FSearchKmp %%Owner:bradch */
FSearchKmp(hkmp, kcSearch, pikme)
KMP ** hkmp;
int kcSearch;
int * pikme;
{
	KMP * pkmp;
	KME * rgkme;
	int iMin, iLim, iGuess, kc;

	pkmp = *hkmp;
	rgkme = &pkmp->rgkme[0];
	iMin = 0;
	iLim = pkmp->ikmeMac;
	if (iLim > 0 && kcSearch >= rgkme[0].kc)
		{{ /* NATIVE - loop in FSearchKmp */
		while (iMin < iLim)
			{
			iGuess = (iMin + iLim) >> 1;
			if ((kc = rgkme[iGuess].kc) == kcSearch)
				{{ /* !NATIVE - FSearchKmp */
				*pikme = iGuess;
				return fTrue;
				}}
			if (kc < kcSearch)
				iMin = iGuess + 1;
			else
				iLim = iGuess;
			}
		}}
	*pikme = iMin;

	return fFalse;
}


/* P K M E  O F  K C  I N  C H A I N */
/* %%Function:PkmeOfKcInChain %%Owner:bradch */
KME * PkmeOfKcInChain(kc)
int kc;
{
	KMP ** hkmp, * pkmp;
	int ikme;

	for (hkmp = hkmpCur; hkmp != hNil; hkmp = pkmp->hkmpNext)
		{
		pkmp = *hkmp;
		if (FSearchKmp(hkmp, kc, &ikme))
			{
			return &pkmp->rgkme[ikme];
			}

		if (pkmp->fStopHere || pkmp->fTranslate || pkmp->fModal)
			{
			break;
			}
		}

	return (KME *) 0;
}






/*  *****************************  */


/* %%Function:FetchCm %%Owner:bradch */
FetchCm(bcm)
BCM bcm;
{
	FetchCm2(bcm, cmmNormal);
}


int cmmCache;

/* %%Function:FetchCm2 %%Owner:bradch */
FetchCm2(bcm, cmm)
BCM bcm;
{
	HPSY hpsy;

	if (bcm == vbcmFetch && cmm == cmmCache)
		return;
	
	cmmCache = cmm;

	vbcmFetch = bcm;

	/* FUTURE: is it worth it to cache the whole symbol? */
	if ((hpsy = HpsyFromBcmElg(vhpsyt, bcm, 
			ElgFromDoc(selCur.doc), cmm)) != NULL)
		{
		bltbh(hpsy, (HPSY) vrgchsyFetch, CbHpsy(hpsy));
		vbcmFetch = (uns) ((char *) hpsy - vhpsyt->grpsy);
		}
	else
		{
		vbcmFetch = bcmNil;
		}
}


/* %%Function:FetchSy %%Owner:bradch */
FetchSy(bsy)
uns bsy;
{
	HPSY hpsy;

	if (bsy == vbcmFetch && cmmCache == 0)
		return;

	cmmCache = 0;
	/* FUTURE: is it worth it to cache the whole symbol? */
	hpsy = (HPSY)(vhpsyt->grpsy + bsy);
	bltbh(hpsy, (HPSY) vrgchsyFetch, CbHpsy(hpsy));
	vbcmFetch = bsy;
}


/* F  E X E C  C M D */
/* This one is about to bite the big one... */
/* %%Function:FExecCmd %%Owner:bradch */
BOOL FExecCmd(bcm)
BCM bcm;
{
	return CmdExecBcmKc(bcm, kcNil) == cmdOK;
}


/* %%Function:FInitCmb %%Owner:bradch */
FInitCmb(pcmb, bcm, hcab, cmm)
CMB * pcmb;
BCM bcm;
HCAB hcab;
int cmm;
{



#ifdef NOTYET
	if (bcm == bcmRepeat)
		{
		if ((bcm = vaab.bcm) == bcmNil)
			{
			/* Nothing to repeat! */
			Beep();
			return fFalse;
			}
		hcab = vaab.hcab;
		}
#endif

/* init SDM if we haven't already */
	if (!vmerr.fSdmInit && !FAssureSdmInit())
			goto ErrRet;

	FetchCm2(bcm, cmm);

	if (hcab == hNil && vpsyFetch->mct == mctSdm)
		{
		if ((hcab = HcabAlloc((WORD) CabiFromPsy(vpsyFetch))) == hNil)
			{
ErrRet:
            pcmb->hcab = hNil;
			return fFalse;
			}
		NinchCab(hcab);
		}

	pcmb->hcab = hcab;
	pcmb->bcm = bcm;
	pcmb->kc = kcNil;
	pcmb->pv = (void *) 0;
	pcmb->cmm = cmm;

	return fTrue;
}


extern BOOL FRecordCab();


/* C M D  E X E C  B C M */
/* Use this to just call a command from another command.  This does NOT
set up undo or repeat and is NOT recorded. */
/* %%Function:CmdExecBcm %%Owner:bradch */
CMD CmdExecBcm(bcm, hcab)
BCM bcm;
HCAB hcab;
{
	CMD cmd;
	CMB cmb;

	if (!FInitCmb(&cmb, bcm, hcab, cmmNormal))
		return cmdNoMemory;

	cmd = CmdExecCmb(&cmb);

	if (cmb.hcab != hNil)
		FreeCab(cmb.hcab);

	return cmd;
}


/* C M D  E X E C  B C M  K C */
/* This one deals with keys, menus, and icons... (kc == kcNil means this
was invoked from a menu or icon).  This takes care of undo, repeat, and
recording. */
/* %%Function:CmdExecBcmKc %%Owner:bradch */
CMD CmdExecBcmKc(bcm, kc)
BCM bcm;
int kc;
{
	CMD cmd;
	CMB cmb;
#ifdef FLASHMENU
	BOOL fHilited;
#endif
	BOOL fRepeatable = fFalse;

#ifdef DEBUG
	if (bcm == imiDebugFailures)
		vfAllocGuaranteed++;
#endif /* DEBUG */

	if (!FInitCmb(&cmb, bcm, hNil, cmmNormal))
		{
		if (bcm == imiSaveAs)
			DoEmergSave(selCur.doc);
#ifdef NOTUSED
		/* cmdError and cmdNoMemory are the same value.  Old code: */
		return bcm == bcmRepeat ? cmdError : cmdNoMemory;
#endif /* NOTUSED */
		cmd = cmdError;
		goto LRet;
		}

#ifdef FLASHMENU
	fHilited = fFalse;
	if (kc != kcNil)
		{
		fHilited = HiliteMenuItem(vhwndApp, vhMenu, bcm, 
				MF_HILITE | MF_BYCOMMAND);
		}
#endif

	cmb.kc = kc;

	PrepAgainBcm(bcm);

	switch (cmd = CmdExecCmb(&cmb))
		{
	default:
		/* q: (bac) Do we really want to trash again
			if the user cancells?
					a: (bz) yes, because we set bcm to bcmNil at the start, so
						the old repeat was already lost, and anything set should
						be disposed of, since we cancelled. We could save away
						the vaab and its handle data and restore if we cancel, but
						it's probably not worth it if noone complains
				*/
		/* in case something within the cancelled command set the
						again stuff (e.g. Ctrl+B in the Replace dialog) */
		/* [rp] except for Search and Goto because they NEVER touch
				the again state as they use FindNext for repeating */
		if (vaab.bcm != bcmNil && bcm != bcmSearch && bcm != bcmGoto)
			SetAgain(bcmNil);
		break;

	case cmdOK:
		/* Record action if applicable */
		if (vfRecording && bcm != bcmRecordNext)
			{
			/* NOTE: cmb.bcm is used because some commands change
				it for the sake of the recorder (exp. Save when
				it really does a SaveAs) */
			GenStatement(cmb.bcm);
			}

		/* Setup repeat command stuff */
		/* note: this says that we assume anyone who set done a
			set again explicitly knew what they were doing, so we
			leave it. bz
		*/
		if (vaab.bcm == bcmNil && FRepeatableBcm(bcm) &&
				!FSpecialBcmInDialog(bcm))
			{
			SetAgainCab(bcm, cmb.hcab);
			fRepeatable = fTrue;
			}
		break;
		}

	/* End RecordNextCommand iff:
	 *	 a) vfRecordNext is true, and
	 *	 b) we did not just finish executing RecordNextCommand, and
	 *  c) the command we just finished was recordable, and
	 *	 d) we did not just finish executing a MacroRun that in turn
	 *     executed RecordNextCommand
	 */
	if (vfRecordNext && bcm != bcmRecordNext &&
		 (FRecordableBcm(bcm) || bcm == bcmRepeat))
		{
		char stName [cchMaxMacroName + 1];

		Assert(vfRecording);
		if (bcm == imiMacro)
			{
			/* We did a MacroRun, check that we didn't run RecordNextCommand */
			GetCabSt(cmb.hcab, stName, cchMaxMacroName + 1, 
				Iag(CABRUNMACRO, hszName));
			if (BcmOfSt(stName) != bcmRecordNext)
				goto EndRecordNext;
			}
		else
			{
EndRecordNext:
			EndSdmTranscription();
			vfRecording = vfRecordNext = fFalse;
			}

		/* Blow away Undo and Repeat, since we have inserted text, and they
		 * may no longer be valid */
		SetUndoNil();
		InvalAgain();
		}

	vkcPrev = cmb.kc;

#ifdef DEBUG
	if (bcm == imiDebugFailures)
		vfAllocGuaranteed--;
#endif /* DEBUG */

#ifdef FLASHMENU
	if (fHilited)
		{
		HiliteMenuItem(vhwndApp, vhMenu, bcm, 
				MF_UNHILITE | MF_BYCOMMAND);
		}
#endif

LRet:
	if (!fRepeatable && cmb.hcab != hNil)
		FreeCab(cmb.hcab);
	return cmd;
}


/* C M D  E X E C  C M B */
/* The lowest level... The hcab must be set up if the command needs one! */
/* %%Function:CmdExecCmb %%Owner:bradch */
CMD CmdExecCmb(pcmb)
CMB * pcmb;
{
	extern int vfInCmd;
	extern BOOL vfHelp;
	extern struct DBS vdbs;

	CMD cmd = cmdOK;
	BCM bcm = pcmb->bcm;

	FetchCm2(pcmb->bcm, pcmb->cmm);

	Assert(bcm != bcmNil);
	Assert(selCur.doc == docNil || (hwwdCur != hNil && hmwdCur != hNil));
	/*  attempt to ensure no asynchronous command executions */
	Assert (!vfInsertMode);

#ifdef RSH
	SetUatMode(uamCommand);
#endif /* RSH */

	/* Check for legality of executing this cmd given the current state */
	if (!FLegalBcm(bcm))
		{
		Beep();
		return cmdError;
		}

	if (vfHelp)  /* Help mode: don't actually execute the cmd */
		{
		if (bcm == bcmCancel)
			CancelContextHelp();
		else
			{
#ifdef RSH
			LogUa(uaCxtHelp);
#endif /* RSH */
			GetHelp(CxtFromBcm(bcm));
			}
		return cmdOK;
		}

	FetchCm2(bcm, pcmb->cmm);
	if (vbcmFetch == bcmNil)
		{
		return cmdError;
		}

	vfElFunc = fFalse;
	vcElParams = 0;

	PrepExecBcm(bcm);
	ClearAbortKeyState();

	switch (vpsyFetch->mct)
		{
	default:
		Beep(); /* most likely a deleted macro */
		return cmdError;

	case mctSdm:
	case mctCmd:

#ifdef RSH
		LogUa(uaBcmFollows);
		LogUa(bcm);
#endif /* RSH */

			{
#ifdef BRADCH
			if (vfRecordNext)
				pcmb->fAction = fFalse;
#endif
				{
#ifdef DEBUG
				int hEatCmd = hNil;
				if (vdbs.cbHeapWantedCmd)
					SetHeapSize(vdbs.cbHeapWantedCmd, &hEatCmd);
#endif
		/* Don't want to call CMD for non-dialog commands
			if action flag is not set because they all 
			ignore it. */
				if (vpsyFetch->mct == mctSdm)
					{
					Debug( vfInCmd++ );
					cmd = (*vpsyFetch->pfnCmd)(pcmb);
					Debug( vfInCmd-- );
					}
				else
					{
					Debug( vfInCmd++ );
					cmd = pcmb->fAction ? (*vpsyFetch->pfnCmd)(pcmb) : cmdOK;
					Debug( vfInCmd-- );
					}
#ifdef DEBUG
				if (hEatCmd)
					FreeH(hEatCmd);
#endif
				}
			break;
			}

	case mctMacro:
	case mctGlobalMacro:
#ifdef RSH
		LogUa(uaTempMacro+(vpsyFetch->mct==mctGlobalMacro));
#endif /* RSH */
		cmd = CmdExecMacroDotImcr(vpsyFetch->docDot, vpsyFetch->imcr);
		if (vmerr.fCBTMacroMemErr)
			{
			Assert(vhwndCBT);
			SendMessage(vhwndCBT, WM_CBTTERM, vhInstance,
				MAKELONG(0, ctrOutOfMemory));
			vmerr.fCBTMacroMemErr = fFalse;
			vmerr.fSentCBTMemErr = fTrue;
			}
		} /* end switch */

	/* Force command failure if we ran out of memory or had disk problem */
	if (vmerr.fMemFail || vmerr.fDiskFail)
		cmd = cmdError;

/* Force a status line update */
	if (vhwndStatLine != NULL)
		{
		selCur.fUpdateStatLine = fTrue;
		}

	if (vaab.bcm != bcmRRFormat)
		{
		ClearRrf();
		}

	if (bcm == bcmRecordNext && cmd == cmdOK)
		SaveCabs(FRecordCab, fFalse);
		
#ifdef RSH
	if (cmd != cmdOK)
		LogUa(uaCancelOrError);
#endif /* RSH */
	return cmd;
}


/* H P S Y  F R O M  B C M  E L G */
/* Given a BCM and ELG (docDot), find the symbol for the command to
	execute.  This is the function that handles distinguishing between
	commands, global macros, and local macros that have the same name. */

/* The first symbol in a bcm chain will always refer to the "most global"
entry (i.e. built-in before global, global before doc-type). */

/* %%Function:HpsyFromBcmElg %%Owner:bradch */
HPSY HpsyFromBcmElg(hpsyt, bcm, elg, cmm)
HPSYT hpsyt;
BCM bcm;
ELG elg;
{
	uns bsy;
	HPSY hpsy;
	HPSY hpsyRet;

	bsy = bcm;

	hpsyRet = hpNil;

	hpsy = (CHAR HUGE *) hpsyt->grpsy + bsy;

	while (bsy != bsyNil)
		{
		switch (hpsy->mct)
			{
/* NOTE: Global macros will be selected over commands automaticaly because
	the commands are listed in the command table first.  If it is
	decided that the user cannot replace global commands with a macro,
	this still will not need to change, only adding a macro will. */

		case mctSdm:
		case mctCmd:
			if (cmm & cmmBuiltIn)
				return hpsy;
			/* FALL THROUGH */

		case mctGlobalMacro:
			if (cmm & cmmSuper)
				return hpsy;
			hpsyRet = hpsy; /* Built-in command or global macro found */
			break;

		case mctMacro:
			if (hpsy->docDot == elg)
				return hpsy; /* Local macro found */
			break;
			}

		if ((bsy = hpsy->bsyNext) == bsyNil)
			break;

		hpsy = (HPSY) (hpsyt->grpsy + bsy);

		if (hpsy->stName[0] != 0)
			break; /* end of same-name chain */
		}
	return hpsyRet;
}


/* E L G  D O C */
/* %%Function:ElgFromDoc %%Owner:bradch */
ELG ElgFromDoc(doc)
int doc;
{
	doc = DocDotMother(doc);
	return ((doc == docNil) ? docGlobalDot : doc);
}


/* P F N  F R O M  B S Y */
/* %%Function:PfnFromBsy %%Owner:bradch */
PFN PfnFromBsy(bsy)
int bsy;
{
	HPSY hpsy;
	PFN pfn;

	Assert(bsy != bsyNil);
	Assert(vhpsyt != hpNil);

		/* BLOCK */
		{
		hpsy = (HPSY) (vhpsyt->grpsy + bsy);
#ifdef DEBUG
			{
			MCT mct = hpsy->mct;
/* NOTE: if this assertion fails, and you Ignore it, Opus WILL crash! */
			Assert(mct == mctCmd || mct == mctSdm || mct == mctEl);
			}
#endif
		pfn = hpsy->pfnEl;
		}

	return pfn;
}


/* made native for optimizing macro tokenization */
/* %%Function:BcmOrBsyOfSt %%Owner:bradch */
NATIVE BcmOrBsyOfSt(st, docDot, fBsy)
CHAR * st;
{
	CHAR * pch;
	CHAR FAR * lpch;
	int cch;
	SY FAR * lpsy;
	SYT FAR *lpsyt;
	uns bcm;
	
	if (st[0] == 0)
		return bsyNil;

	Assert(vhpsyt != hpNil);

	/* note: using long pointers for speed */
	lpsyt = LpLockHp(vhpsyt);

		/* BLOCK */
		{
		CHAR FAR * lpgrpsy;

		lpgrpsy = lpsyt->grpsy;

		for (bcm = lpsyt->mpwbsy[WHash(st)]; bcm != bcmNil;
				bcm = lpsy->bsyNext)
			{
			lpsy = (SY FAR *)(lpgrpsy + bcm);

			pch = st;
			lpch = lpsy->stName;
			cch = *pch++;
			if (cch != *lpch++)
				continue;

			while (cch-- > 0)
				{
				if (ChUpperLookup(*pch++) != ChUpperLookup(*lpch++))
					break;
				}

			if (cch < 0) /* matched */
				break;
			}
		}

	UnlockHp(vhpsyt);

	if (fBsy)
		{
		HPSY hpsy;

		if ((hpsy = HpsyFromBcmElg(vhpsyt, bcm, docDot, 0)) != 0)
			bcm = (CHAR huge *) hpsy - (CHAR huge *) vhpsyt->grpsy;
		/* bcm now contains a bsy */
		}

	return bcm;
}


/* B C M  O F  S T */
/* %%Function:BcmOfSt %%Owner:bradch */
BCM BcmOfSt(st)
CHAR * st;
{
	return BcmOrBsyOfSt(st, docNil, fFalse);
}


/* B S Y  O F  S T */
/* %%Function:BsyOfStDocDot %%Owner:bradch */
uns BsyOfStDocDot(st, docDot)
CHAR * st;
int docDot;
{
	return BcmOrBsyOfSt(st, docDot, fTrue);
}


/* W  H A S H */
/* Return hash code for given string.  NOTE: case insensitive */
/* %%Function:WHash %%Owner:bradch */
NATIVE uns WHash(st)
CHAR * st;
{
	uns wHash, cch;

	wHash = 0;
	cch = *st++;
	while (cch--)
		wHash = ((wHash << 5) + (wHash >> 11) + ChUpperLookup(*st++));
	wHash = ((wHash & 0x7fff) % wHashMax);

	return wHash;
}


/* %%Function:BsyElOfSt %%Owner:bradch */
BsyElOfSt(st)
CHAR * st;
{
	return BsyElOfBcm(BcmOfSt(st));
}


/* made native for optimizing macro tokenization */
/* %%Function:BsyElOfBcm %%Owner:bradch */
NATIVE BsyElOfBcm(bcm)
BCM bcm;
{
	CHAR * pch;
	CHAR FAR * lpch;
	int cch;
	SYT FAR *lpsyt;
	SY FAR *lpsy;
	uns bsy;

	Assert(vhpsyt != hpNil);

	if (bcm == bsyNil)
		return bsyNil;

	/* using long pointers instead of huge pointers for speed */
	lpsyt = LpLockHp(vhpsyt);

		/* BLOCK */
		{
		CHAR FAR * lpgrpsy;

		lpgrpsy = lpsyt->grpsy;

		bsy = bcm;
		lpsy = (SY FAR *) (lpgrpsy + bsy);
		for (;;)
			{
			if (FElMct(lpsy->mct))
				{
				bcm = bsy;
				goto LDone;
				}

			if (lpsy->mct == mctCmd || lpsy->mct == mctSdm)
				{
				bcm = bsy;
				}

			if ((bsy = lpsy->bsyNext) == bsyNil)
				{
				break;
				}

			lpsy = (SY FAR *) (lpgrpsy + bsy);
			if (lpsy->stName[0] != 0)
				{
				break;
				}
			}
		}
LDone:
	UnlockHp(vhpsyt);

	return bcm;
}



/* K C  M O D I F I E D */
/* Given a kc, apply the shift, ctrl, and alt modifier bits as apropriate. */
/* FUTURE: should put vfFooKeys in propper bits so this can be an OR. */
/* %%Function:KcModified %%Owner:bradch */
int KcModified(kc)
int kc;
{
	if (vfOptionKey)
		/* we don't allow Alt-F11 thru Alt-F16 because you
			can't get them on the old-style AT keyboard */
		if ((uns)(kc - VK_F11) > (VK_F16 - VK_F11))
			kc = KcAlt(kc);

	if (vfShiftKey)
		kc = KcShift(kc);

	if (vfControlKey)
		kc = KcCtrl(kc);

	return kc;
}


/* FUTURE: Temporary until cursor keys are in keymap */
csconst int rgkcCursor [] =
{
	kcNextPara,
			kcPrevPara,
			kcWordRight,
			kcWordLeft,
			kcWordRightAlt,
			kcWordLeftAlt,
			kcEndLine,
			kcBeginLine,
			kcRight,
			kcLeft,
			kcTopScreen,
			kcEndScreen,
			kcDown,
			kcUp,
			kcPageDown,
			kcPageUp,
			kcEndDoc,
			kcTopDoc,
			kcClear
};


#define ikcCursorMax (sizeof (rgkcCursor)/ sizeof (int))


/* F  C U R S  K E Y */
/* Check if a kc is a cursor key and optionaly execute the key's function */
/* %%Function:FCursKey %%Owner:bradch */
FCursKey(kc, fDoIt)
int kc;
BOOL fDoIt;
{
	extern BOOL vfBlockSel;
	int ikc;

	if (wwCur == wwNil)
		return fFalse;

	SetOurKeyState();
	kc |= vgrpfKeyBoardState & (wKbsControlMask | wKbsOptionMask);

	/* ignore control key if block selection */
	if (vfBlockSel)
		kc &= ~wKbsControlMask;

	/* search to avoid bringing in curskeys.c */
	if (IScanLprgw( (int far *)rgkcCursor, kc, ikcCursorMax ) != iNil)
		{
		BOOL	fRet;

#ifdef RSH
		if (!vfInsertMode)
			{
			if (vfShiftKey || vfExtendSel)
				LogUa(uaCursExtend+IScanLprgw((int far *)rgkcCursor,kc,ikcCursorMax));
			else
				LogUa(uaCursor+IScanLprgw((int far *)rgkcCursor,kc,ikcCursorMax));
			SetUatMode(uamNavigation);
			StopUatTimer();
			}
#endif /* RSH */

		fRet =  FKeyCmdLeftRight(kc, fDoIt) ||
				FKeyCmdUpDown(kc, fDoIt) || kc == kcClear;
		vkcPrev = kc;
		return fRet;
		}

	return fFalse;
}



/* F  K E Y  C M D  L E F T  R I G H T */
/* Move or drag selection in left or right directions */
/* %%Function:FKeyCmdLeftRight %%Owner:bradch */
FKeyCmdLeftRight(kc, fDoIt)
int	kc;
BOOL fDoIt;
{

	BOOL fFwdKey = fFalse;
	BOOL fExtend = (vfShiftKey || vfExtendSel);
	int	sty;

	switch ( kc )
		{
	case kcNextPara:
		fFwdKey = fTrue;

	case kcPrevPara:
		sty = styPara;
		break;

	case kcWordRightAlt:
		if (vfShiftKey)		/* Avoid interference w/outline key*/
			return fFalse;
		/* FALL THROUGH */
	case kcWordRight:
		fFwdKey = fTrue;
		sty = styWord;
		break;
	case kcWordLeftAlt:
		if (vfShiftKey)		/* Avoid interference w/outline key*/
			return fFalse;
		/* FALL THROUGH */
	case kcWordLeft:
		sty = styWord;
		break;

	case kcEndLine:
		fFwdKey = fTrue;

	case kcBeginLine:
		sty = styLineEnd;
		break;

	case kcRight:
		fFwdKey = fTrue;

	case kcLeft:
		/* 2202 fix: incorrect dragging in outline mode (peterdur)    */
		/* If dragging and a para is selected, do a para move instead */	 
		if ((*hwwdCur)->fOutline && (vfShiftKey || vfExtendSel)
			&& (selCur.sty == styPara))
			sty = styPara;
		else
			sty = styChar;
		break;

	default:
		return fFalse;
		}

	if (fDoIt)
		{
		ClearInsertLine(PselActive());
		if (vfBlockSel)
			CursBlockLeftRight(sty, fFwdKey ? 1 : -1, fFalse/*ignored*/);
		else
			CursLeftRight(sty, fFwdKey ? 1 : -1, fExtend);

		vfLastCursor = fFalse;
		}

	return fTrue;
}





/* F  K E Y  C M D  U P  D O W N */
/* Check if kc is a vertical cursor mover.  If it is, then if fDoIt, do the
movement and return fTrue.  If it is not, return fFalse.
*/
/* %%Function:FKeyCmdUpDown %%Owner:bradch */
FKeyCmdUpDown(kc, fDoIt)
int	kc;
{
	struct SEL *psel;
	int	sty, dSty;
	BOOL fFwdKey = fFalse;
	extern int	vkcPrev;
	BOOL fDragOutline = fFalse;

	/* Reset the scroll counts, if not consecutive scroll up or down. */
	if (kc != vkcPrev)
		{
		vcConseqScroll = 0;
		sbScroll = sbScrollLast = SB_NIL;
		}
	switch (kc)
		{
	case kcDown:
		fFwdKey = fTrue;
		/* FALL THRU */

	case kcUp:
		/* 2202 fix: incorrect dragging in outline mode (peterdur)    */
		/* If dragging and a para is selected, do a para move instead */
		if ((*hwwdCur)->fOutline && (vfShiftKey || vfExtendSel)
			&& (selCur.sty == styPara))
			fDragOutline = fTrue;
		sty = styLine;
		break;

	case kcPageDown:
		fFwdKey = fTrue;
		/* FALL THRU */

	case kcPageUp:
		sty = styScreen;
		break;

	case kcEndDoc:
		fFwdKey = fTrue;
		/* FALL THRU */

	case kcTopDoc:
		sty = styDoc;
		break;

	case kcEndScreen:
		fFwdKey = fTrue;

	case kcTopScreen:
		sty = styScreenEnd;
		break;

	default:
		return fFalse;
		}

	if (fDoIt)
		{
		Profile(vpfi == pfiPgUp ? StartProf(5) : 0);
		dSty = 1;
		ClearInsertLine((psel = PselActive()));
		if (vfBlockSel)
			CursBlockUpDown(sty,
					fFwdKey ? dSty : -dSty, fTrue /* fDrag */);
		else
			{
			EnsureSelInWw(fTrue, psel);
			if (fDragOutline)
				CursLeftRight(styPara, fFwdKey ? dSty : -dSty, fTrue);
			else
				CursUpDown(sty, fFwdKey ? dSty : -dSty,
					(vfShiftKey || vfExtendSel)/* fDrag */);
			}
		Profile(vpfi == pfiPgUp ? StopProf() : 0);
		}

	return fTrue;
}



#ifdef DEBUG
/* F  P R O C E S S  D B G  K E Y */
/*  Handle special debugging keys.  Here they get handled in dialogs,
	message boxes and during normal operation.
*/

/* %%Function:FProcessDbgKey %%Owner:bradch */
FProcessDbgKey (kc)
int kc;
{
	switch (kc)
		{
	case kcPrintScr:
		SlapWnd();
	case kcSysState:
		DumpState ();
		return fTrue;
	case kcStackTrace:
		DoStackTrace (NULL);
		return fTrue;
	case kcExitWin1:
	case kcExitWin2:
		OurExitWindows(); /* does not return */

#ifdef PCJ
	case KcAlt(KcCtrl('1')):
		PCJDbg1 ();
		return fTrue;
	case KcAlt(KcCtrl('2')):
		PCJDbg2 ();
		return fTrue;
	case KcAlt(KcCtrl('3')):
		PCJDbg3 ();
		return fTrue;
#endif /* PCJ */

		}
	return fFalse;
}


#endif /* DEBUG */


/* F  R U N  M A C R O */
/* %%Function:FRunMacro %%Owner:bradch */
FRunMacro(stName)
char * stName;
{
	extern CMB vcmb;
	BCM bcm;

#ifdef DCMDTBL
	CommSz(SzShared("FRunMacro: "));
	CommStNL(stName);
#endif

	if ((bcm = BcmOfSt(stName)) == bcmNil)
		return fFalse;

	vcmb.kc = kcNil;
	vcmb.bcm = bcm;
	vcmb.fDefaults = fTrue;
	vcmb.fAction = fTrue;
	vcmb.fDialog = fTrue;

	return FExecCmd(bcm);
}


csconst int rgbcm[] =
{
/* This file is generated by MKCMD */
#include "rgbcm.h"
};


#define ibcmNil -1
#define ibcmMac (sizeof(rgbcm) / sizeof(int))

/* I B C M  F R O M  B C M */
/* ibcm's are the invariant id numbers used for help contexts and
	semantic events in CBT */
/* made native for macro optimization */
/* %%Function:IbcmFromBcm %%Owner:bradch */
NATIVE IbcmFromBcm(bcm)
int bcm;
{
	return IScanLprgw( (int far *)rgbcm, bcm, ibcmMac );
}


/* %%Function:CxtFromBcm %%Owner:bradch */
CxtFromBcm(bcm)
BCM bcm;
{
	int ibcm;
	HPSY hpsy;
	uns bsyNext;
	uns bsyI;

	if (bcm >= bcmMacStd)
		return cxtUserDefined;

	hpsy = (SY HUGE *) (vhpsyt->grpsy + bcm);
	bsyNext = hpsy->bsyNext;

	for (ibcm = 0; ibcm < ibcmMac; ibcm++)
		{
		bsyI = rgbcm[ibcm];
		if (bcm == bsyI || bsyI != bsyNil && bsyNext == bsyI && 
				((SY HUGE *) (vhpsyt->grpsy + bsyI))->stName[0] == '\0')
			return ibcm;
		}

	return ibcmNil;
}


/* B C M  F R O M  I B C M */
/* %%Function:BcmFromIbcm %%Owner:bradch */
BcmFromIbcm(ibcm)
int ibcm;
{
	Assert (ibcm < ibcmMac);

	return rgbcm[ibcm];
}



/* %%Function:ClearRrf %%Owner:bradch */
ClearRrf()
{
	if (rrf.hgrpprl != hNil)
		{
		FreePh(&rrf.hgrpprl);
		}
	SetBytes(&rrf, '\0', sizeof(RRF));
	rrf.doc = selCur.doc;
	rrf.ftc = ftcNil;
/*
	rrf.stc = stcNil;
*/
}


/* %%Function:FRepeatableBcm %%Owner:bradch */
FRepeatableBcm(bcm)
BCM bcm;
{
	if ((int)bcm >= 0)
		{
		FetchCm(bcm);
		return vpsyFetch->fRepeatable;
		}

	return fTrue;
}


/* F  S P E C I A L  B C M  I N  D I A L O G */
/* Returns true if this is one of the bcm's of formatting keys while
	in a dialog.  These should not touch the again state. */

/* %%Function:FSpecialBcmInDialog %%Owner:bradch */
FSpecialBcmInDialog(bcm)
{
	return (vfSearchRepl && bcm != imiSearch && bcm != imiReplace ||
			vfDefineStyle && bcm != bcmStyles);
}


/* %%Function:FNeedsDocBcm %%Owner:bradch */
FNeedsDocBcm(bcm)
int bcm;
{
	if (bcm >= 0)
		{
		FetchCm(bcm);
		return vpsyFetch->fNeedsDoc;
		}

	return fTrue;
}


/* S E T  A G A I N */

/* %%Function:SetAgain %%Owner:bradch */
SetAgain(bcm)
int bcm;
{
	extern struct AAB vaab;

	if (!FRepeatableBcm(bcm))
		{
		bcm = bcmNil;
		}

	vaab.bcm = bcm;

	/* free up grpprl in vaab if bcmFormatting is not the action repeated */
	if (vaab.hgrpprl != hNil && vaab.bcm != bcmFormatting)
		{
		FreePh(&vaab.hgrpprl);
		vaab.cbGrpprl = 0;
		}


}


/* S E T A G A I N C P */
/* Sets vaab with cp info */
/* %%Function:SetAgainCp %%Owner:bradch */
SetAgainCp(doc, cpFirst, cpLow, cpLim)
int doc;
CP cpFirst, cpLow, cpLim;
{
	extern struct AAB vaab;

	/* if copylooks, the space occupied by these cp's is a handle
	that should be freed. Hopefully, this routine is aways called
	AFTER SetAgain so that will already be taken care of. bz
	*/
	Assert (vaab.bcm != bcmCopyLooks);
	vaab.doc = doc;
	vaab.cpFirst = cpFirst;
	vaab.cpLow = cpLow;
	vaab.cpLim = cpLim;
}


/* S E T  A G A I N  C A B */
/* %%Function:SetAgainCab %%Owner:bradch */
SetAgainCab(bcm, hcab)
BCM bcm;
void ** hcab;
{
	extern struct AAB vaab;

	if (vaab.hcab != hNil && vaab.hcab != hcab)
		FreeCab(vaab.hcab);

	SetAgain(bcm);
	vaab.hcab = hcab;
}


/* %%Function:InvalAgain %%Owner:bradch */
InvalAgain()
{
	SetAgain(bcmNil);
	vaab.acc = accNil;
	vaab.asc = ascNil;
	vaab.doc = docNil;
}


/* %%Function:TestInvalAgain %%Owner:bradch */
TestInvalAgain()
{
/* kill all commands which act on the document and hence can get smashed by
   changed doc. possible overkills in the process. */

	if (vaab.bcm != bcmNil && FNeedsDocBcm(vaab.bcm))
		InvalAgain();
}


/* %%Function:ClearRibbonRrf %%Owner:bradch */
ClearRibbonRrf()
{
	rrf.fBold = rrf.fItalic = rrf.fSCaps = fFalse;
	rrf.kul = rrf.iSuperSub = 0;
	rrf.ftc = ftcNil;
	rrf.hps = 0;
	rrf.fRibbon = fFalse;
	rrf.fDirty = /*(rrf.stc != stcNil) ||*/ (rrf.hgrpprl != hNil);
	rrf.fTouched = fTrue;
}


/* %%Function:ClearRulerRrf %%Owner:bradch */
ClearRulerRrf()
{
/*
	rrf.stc = stcNil;
*/
	if (rrf.hgrpprl != hNil)
		{
		FreePh(&rrf.hgrpprl);
		rrf.hgrpprl = hNil;
		rrf.cbGrpprl = 0;
		}
	rrf.fRibbon = fTrue;
	rrf.fDirty = rrf.fBold || rrf.fItalic || rrf.fSCaps ||
			(rrf.kul != 0) || (rrf.iSuperSub != iSSNormal) ||
			(rrf.ftc != ftcNil) || (rrf.hps != 0);
	rrf.fTouched = fTrue;
}


/* F  S E T  A G A I N  G R P P R L */
/* Sets the Grpprl in the again structure */
/* %%Function:FSetAgainGrpprl %%Owner:bradch */
FSetAgainGrpprl(grpprl, cb, bcm)
char *grpprl;
int cb;
int bcm;
{


	if (vaab.hgrpprl != hNil)
		FreeH(vaab.hgrpprl);
	if (cb > cbMaxAgainGrpprl || (vaab.hgrpprl = HAllocateCw(CwFromCch(cb))) == hNil)
		{
		/* return value set up for whoever cares if repeat set up ok */
		/* note that if allocation fails,repeat will do nothing */
		vaab.cbGrpprl = 0;
		return fFalse;
		}

	bltb(grpprl, *vaab.hgrpprl, cb);
	vaab.cbGrpprl = cb;
	SetAgain (bcm);

	return fTrue;
}


/* F  P R I N T E R  O  K */
/* Check if the printer and the port are valid */
/* If we have never actually tried to get a printer DC
	(DisplayAsPrint and background repag and status line are off)
	this routine may return fTrue because it doesn't know any better
	and finding out is too expensive (because it would entail
	trying to get a printer DC) */

/* %%Function:FPrinterOK %%Owner:bradch */
BOOL FPrinterOK()
{
	return !vmerr.fPrintEmerg && vpri.hszPrinter != NULL &&
			FNeNcSz(*vpri.hszPrPort, szNone);
}


/* F  S C R A P  F U L L */
/* Return fTrue iff the clipboard contains something we can use */
/* %%Function:FScrapFull %%Owner:bradch */
FScrapFull(fPasteLink)
BOOL fPasteLink;
{
	BOOL f = fFalse;

	if (vsab.fOwnClipboard)
		{
		return !fPasteLink && (CpMacDocEdit(docScrap) > cp0);
		}

	if (OpenClipboard(vhwndApp) != 0)
		{
		int fmt = 0;

		while ((fmt = EnumClipboardFormats(fmt)) != 0)
			{
			if ((fPasteLink && fmt == cfLink)
					|| (!fPasteLink && (fmt == cfRTF ||
					fmt == CF_TEXT ||
					fmt == CF_BITMAP ||
					fmt == CF_METAFILEPICT)))
				{
				f = fTrue;
				break;
				}
			}
		CloseClipboard();
		}

	return f;
}


/* F  H A S  G L S Y S */
/* %%Function:FHasGlsys %%Owner:bradch */
FHasGlsys(doc)
int doc;
{
	Assert(doc == docNil || PdodDoc(doc)->fDot);
	if (doc == docNil || (doc = PdodDoc(doc)->docGlsy) == docNil)
		return fFalse;
	else
		return (*PdodDoc(doc)->hsttbGlsy)->ibstMac > 0;
}


/* %%Function:FHasRefs %%Owner:bradch */
FHasRefs(doc, edcDrp)
int doc;
int edcDrp;
{
	struct DOD *pdod;
	struct DRP *pdrp;

	if (doc != docNil && (pdod = PdodDoc(doc))->fMother)
		{
		pdrp = ((int *)pdod + edcDrp);
		if (pdrp->doc != docNil && pdrp->hplcRef != hNil &&
				PdodDoc(pdrp->doc)->hplcfnd != hNil)
			{
			return fTrue;
			}
		}
	return fFalse;
}


/* F  W H O L E  R O W S  P S E L */
/* Returns true if the selection covers all of the rows */
/* %%Function:FWholeRowsPsel %%Owner:bradch */
FWholeRowsPsel(psel)
struct SEL *psel;
{

	if (!psel->fTable)
		return fFalse;
	else  if (psel->sk == skRows)
		return fTrue;
	else  if (psel->itcFirst == 0 && psel->itcLim >= ItcMacPca(&psel->ca))
		return fTrue;

	return fFalse;

}


/* F  T A B L E  I N  P C A */
/* Returns true if there is a table anywhere in the ca */
/* %%Function:FTableInPca %%Owner:bradch */
FTableInPca(pca)
struct CA *pca;
{
	CP cpCur;
	int doc = pca->doc;
	CP cpLim = pca->cpLim;

	cpCur = pca->cpFirst;
	if (cpLim == cpCur)
		cpLim++;

	while (cpCur < cpLim)
		{
		CachePara(doc,cpCur);
		if (FInTableDocCp(doc, cpCur))
			return fTrue;
		cpCur = caPara.cpLim;
		}

	return fFalse;

}


/* F  T T P  P S E L  */
/* Determine whether selection is the end of row paragraph */
/* %%Function:FTtpPsel %%Owner:bradch */
BOOL FTtpPsel(psel)
struct SEL *psel;
{
	extern struct TAP vtapFetch;
	extern struct CA caTap;

	struct CA caSel;
	CP cp;

	if (psel->fIns)
		{
		CachePara(psel->doc, psel->cpFirst);
		return (vpapFetch.fTtp);
		}
	else  if (psel->fTable)
		{
		CpFirstTap(psel->doc, psel->cpFirst);
		return (psel->itcFirst == vtapFetch.itcMac);
		}

	return fFalse;
}



/* %%Function:FLegalBcm %%Owner:bradch */
FLegalBcm(bcm)
BCM bcm;
{
	struct SEL *psel = PselActive();
	struct DOD *pdod = psel->doc != docNil ? PdodDoc(psel->doc) : NULL;

	if (bcm == imiRepeat)
		bcm = vaab.bcm;

	if (bcm == bcmNil)
		return fFalse;

	/* This takes care of menu caches, etc. */
	/* legal except when in Preview */
	if ((int) bcm < 0 && vlm == lmPreview)
		return fFalse;

	if ((int)bcm >= 0)
		{
		FetchCm(bcm);

#ifdef DEBUG
		if (psel->doc != docNil)
			AssertH(hwwdCur);
#endif

		/* REVIEW cslbug: broken up because of native bug */
		if ((bcm == bcmRibbon && vhmes != hNil) ||
				(vpsyFetch->fNeedsDoc && psel->doc == docNil) ||
				(!vpsyFetch->fPreviewMode && vlm == lmPreview))
			{
			return fFalse;
			}

		if (psel->doc != docNil)
			{
			if ((vpsyFetch->fMomLock && PdodMother(psel->doc)->fLockForEdit) ||
					(vpsyFetch->fLockedMode && pdod->fLockForEdit))
				{
				switch (bcm)
					{
				case bcmExpand:
				case bcmCollapse:
				case bcmShowToLevel1:
				case bcmShowToLevel2:
				case bcmShowToLevel3:
				case bcmShowToLevel4:
				case bcmShowToLevel5:
				case bcmShowToLevel6:
				case bcmShowToLevel7:
				case bcmShowToLevel8:
				case bcmShowToLevel9:
				case bcmExpandAll:
				case bcmToggleEllip:
					break;
				default:
					return fFalse;
					}
				}
			if	(vpsyFetch->fFootnoteMode && pdod->fFtn)
				return fFalse;


			if ((vpsyFetch->fHeaderMode &&  (pdod->fHdr || pdod->fDispHdr)) ||
					(vpsyFetch->fAnnotateMode && pdod->fAtn) ||
					(vpsyFetch->fOutlineMode && (*hwwdCur)->wk == wkOutline) ||
					(vpsyFetch->fMacroMode && (*hwwdCur)->wk == wkMacro))
				{
				return fFalse;
				}
			}
		}

	if (psel->doc != docNil)
		{
		if ( (PdodDoc(psel->doc)->fFtn || PdodDoc(psel->doc)->fAtn))
			{
			/* are we pasting into a footnote/annot subdoc outside the
				range of any entry? In the autodelete with selection case
				this is checked in CmdAutoDelete (eventually) but for non-
				autodelete or ins pt, we need this check. Note that in the
				non autodelete case, we use an ins pt. This prevents bogus
				rejections when the selection would not let us paste because it
				spans footnotes, but it would be ok as an ins pt.
			*/
			switch (bcm)
				{
			case bcmInsTable: /* insert table ignores autodelete */
				if (!FCheckRefDoc(psel->doc, psel->cpFirst, psel->cpFirst, fFalse/*fReportErr*/))
					return fFalse;
				break;
			case bcmInsFieldChars:
			case bcmPaste:
			case bcmPasteLink:
			case bcmHdrPage:
			case bcmHdrDate:
			case bcmHdrTime:
			case bcmInsPic:
			case bcmInsFile:
			case bcmInsField:
				if ((psel->fIns || !vpref.fAutoDelete) &&
						!FCheckRefDoc(psel->doc, psel->cpFirst, psel->cpFirst, fFalse/*fReportErr*/))
					{
					return fFalse;
					}
			default:
				break;
				}
			}
		}

	switch (bcm)
		{
	case bcmRuler:
		if ((*hwwdCur)->fOutline)
			return fFalse;
		break;

	case imiFieldCodes:
		if (hwwdCur == hNil || (*hwwdCur)->grpfvisi.fvisiShowAll)
			return fFalse;
		break;

	case imiUndo:
		if (vuab.uac == uacNil || vuab.bcm == bcmNil)
			return fFalse;
		break;

	case bcmMoveWnd:
	case bcmSizeWnd:
	case bcmZoomWnd:
		if (vpref.fZoomMwd)
			return fFalse;
		break;

	case bcmRestoreWnd:
		if (!vpref.fZoomMwd)
			return fFalse;
		break;

	case bcmSplit:
		if (hmwdCur == hNil || !(*hmwdCur)->fCanSplit)
			return fFalse;
		break;

	case imiPrint:
	case imiPrintPreview:
		/* grayed if no printer */
		if (!FPrinterOK())
			return fFalse;
		break;

	case imiCalculate:
	case imiDelCut:
	case imiCopy:
		/* grayed if empty selection */
		if (psel->fIns)
			return fFalse;
		break;

		/* listed here, I think, are all bcm's which cause something
				to be inserted into the document (RP) */
	case bcmPaste:
		if (!FScrapFull(fFalse))
			return fFalse;
		/* FALL THROUGH */

	case bcmTyping:
	case bcmInsBreak:
	case imiInsPicture:
	case imiInsFile:
	case bcmIndexEntry:
	case bcmIndex:
	case imiTOC:
	case imiInsPageBreak:
	case bcmHdrPage:
	case bcmHdrDate:
	case bcmHdrTime:
	case bcmUnspike:
	case imiRenumParas:
	case bcmInsFootnote:
	case imiSort:

/* REVIEW bradch (rp): Are all the checks for fLockForEdit in FLegalBcm
	necessary with the LockMode bit in the command table?  There are a
	couple more below in this routine.  If they aren't, we should make sure
	all the bcm's we're checking specifically in here have the *L in
	opuscmd.cmd before removing this code, but once we do it will save us
	a bunch of code.
*/

		if (psel->doc == docNil || PdodDoc(psel->doc)->fLockForEdit)
			return fFalse;
		if (bcm == imiSort)  /* fttp check not enough for sort */
			break;
		if (bcm == bcmPaste && vsab.fTable)
			break;    /* fTtp check not necessary if pasting cells */
		/* FALL THROUGH */

	case bcmInsAnnotation:

		if (FTtpPsel(psel))
			return fFalse;
		break;

	case bcmInsColumnBreak:
		pdod = PdodDoc(psel->doc);
		if (!FSelWithinTable(psel) && ((*hwwdCur)->wk == wkOutline ||
				pdod->fShort))
			{
			return fFalse;
			}
		break;

	case imiPasteLink:
		if (!FScrapFull(fTrue) || FTtpPsel(psel))
			return fFalse;
		break;

	case imiGlossary:
		if (FTtpPsel(psel))
			return fFalse;

		if ((psel->fTable ? !FWholeRowsPsel(psel) :
				(psel->sk & skNotGlsy)) &&
				!FHasGlsys(docGlobalDot) &&
				!FHasGlsys(DocDotMother(psel->doc)))
			{
			return fFalse;
			}
		break;

	case imiViewFootnote:
	case imiViewAnnotation:
		if (!FHasRefs(DocMother(psel->doc), 
				bcm == imiViewFootnote ? edcDrpFtn : edcDrpAtn))
			{
			return fFalse;
			}
		break;

	case imiInsField:
	case bcmInsFieldChars:
		if (psel->doc == docNil ||
				PdodDoc(psel->doc)->fLockForEdit ||
				(psel->sk != skIns && psel->sk != skSel) ||
				psel->fTable || FTtpPsel(psel))
			{
			return fFalse;
			}
		break;

	case bcmInsBookmark:
		if (psel->fTable && !FWholeRowsPsel(psel))
			return fFalse;
		if (psel->sk == skBlock)
			return fFalse;
		break;

	case bcmHdrLinkPrev:
		return FCanLinkPrev(wwCur);

	case imiReplace:
	case imiHyphenate:
	case imiRevMarking:
	case imiSpelling:
	case bcmSpellSelection:
	case bcmChangeCase:
LNoBlock:
		if (psel->fBlock)
			return fFalse;
		break;

	case imiInsTable:
		if (psel->fWithinCell || FTableInPca(&psel->ca))
			return fFalse;
		goto LNoBlock;

	case bcmTableToText:
		if (!FWholeRowsPsel(psel))
			return fFalse;
		break;

	case bcmFormatTable:
		if (!psel->fWithinCell && !psel->fTable)
			return fFalse;

		if (FTtpPsel(psel))
			return fFalse;
		break;

	case imiEditTable:
	case bcmNextCell:
	case bcmPrevCell:
		if (!psel->fWithinCell && !psel->fTable)
			return fFalse;

		break;

	case bcmTopColumn:
	case bcmSelectTable:
		if (!FInTableDocCp(psel->doc, psel->cpFirst))
			return fFalse;
		break;

	case bcmBottomColumn:
		if (!FInTableDocCp(psel->doc, psel->cpLim))
			return fFalse;
		break;

	case imiPosition:
		if (vlm == lmPreview && (vpvs.iobj - iobjMinAbs < 0))
			return fFalse;
		break;

	case imiPicture:
		if (!psel->fGraphics)
			return fFalse;
		break;

	case bcmRecorder:
	case bcmEditMacro:
	case bcmChangeKeys:
	case bcmAssignToMenu:
		if (PdodDoc(docGlobalDot)->fLockForEdit &&
				FDisableTemplate(psel->doc))
			{
			return fFalse;
			}
		break;

	case bcmExpand:
	case bcmCollapse:
	case bcmShowToLevel1:
	case bcmShowToLevel2:
	case bcmShowToLevel3:
	case bcmShowToLevel4:
	case bcmShowToLevel5:
	case bcmShowToLevel6:
	case bcmShowToLevel7:
	case bcmShowToLevel8:
	case bcmShowToLevel9:
	case bcmExpandAll:
	case bcmToggleEllip:
		if (!(*hwwdCur)->fOutline)
			return fFalse;
		break;
		}

	/* check for in extremis (extremely low memory or no scratch file
	space left) -- just let them save or shutdown */
	Assert(cHpFreeze==0);
	if (vmerr.fDiskEmerg || 
			(vmerr.hrgwEmerg3 == hNil && 
			(vmerr.hrgwEmerg3 = HAllocateCw(cwEmerg3)) == hNil) ||
			(vmerr.hrgwEmerg2 == hNil && 
			(vmerr.hrgwEmerg2 = HAllocateCw(cwEmerg2)) == hNil) ||
			(vmerr.hrgwEmerg1 == hNil && 
			(vmerr.hrgwEmerg1 = HAllocateCw(cwEmerg1)) == hNil))
		{
		vmerr.mat = matNil;  /* Do not report errors from above allocations */
		switch (bcm)
			{
		case bcmOK:
		case bcmCancel:
		case bcmMenuMode:
		case bcmSaveDot:
		case bcmSave:
		case bcmExit:
		case imiSaveAs:
		case imiFileClose:
		case imiSaveAll:
		case bcmCloseWnd:
		case bcmNextMwd:
		case bcmPrevMwd:
		case bcmWndCache:
#ifdef DEBUG
		case imiDebugFailures:
		case imiDbgMemory:
		case bcmEatMemory:
		case bcmFreeMemory:
#endif /* DEBUG */
			return fTrue;
		case bcmPageView:
			Assert(wwCur != wwNil);
			/* allow pageview to be turned off can improve low memory situation */
			return (PwwdWw(wwCur)->fPageView);
		default:
			return (bcm >= imiWnd1 && bcm <= imiWnd9);
			}
		}


	return fTrue;
}



/* Prepare for execution of the given command by terminating
any appropriate modes we might be in.  It is assumed that the
command has been fetched. */
/* %%Function:PrepExecBcm %%Owner:bradch */
PrepExecBcm(bcm)
int bcm;
{
	/* We do not just fetch the commands here because when EL calls
		this, it uses FetchSy() instead of FetchCm(). */

	/* bcmHelp is a special case - we do NOT want to cancel block
		mode or extend selection mode if the user calls help.  */
	if (vfBlockSel && !vpsyFetch->fBlockMode && bcm != bcmHelp)
		BlockModeEnd();

	if (vfExtendSel && !vpsyFetch->fExtendMode && bcm != bcmHelp)
		ExtendSelEnd();

	if (vssc != sscNil)
		switch (bcm)
			{
			extern int vfAwfulNoise;

		case bcmCancel:
			vfAwfulNoise = fTrue;  /* prevent beep in this case */
			/* fall thru intentional */
		default:
			if (vpsyFetch->mct != mctEl || !vpsyFetch->fDyadic)
				CancelDyadic();
			break;

		case bcmTraceMacro:
		case bcmStepMacro:
		case bcmContinueMacro:
		case bcmAnimateMacro:
		case bcmOK:
		case bcmOtherPane:
		case bcmPgvNextPage:
		case bcmPgvPrevPage:
		case bcmNextMwd:
		case bcmPrevMwd:
		case bcmNextCell:
		case bcmPrevCell:
		case bcmBeginRow:
		case bcmEndRow:
		case bcmTopColumn:
		case bcmBottomColumn:

/* bobz bcmGoto would be nice, but it is hardcoded for selCur,
   and we would have to change it all all its called routines to
   use PSelActive().
*/
		case imiWnd1:
		case imiWnd2:
		case imiWnd3:
		case imiWnd4:
		case imiWnd5:
		case imiWnd6:
		case imiWnd7:
		case imiWnd8:
		case imiWnd9:
			break;
			}
}


PrepAgainBcm(bcm)
int bcm;
{
	if (bcm != bcmRepeat)
		{
		if (FRepeatableBcm(bcm))
			{
			if (!FSpecialBcmInDialog(bcm))
				SetAgain(bcmNil);
			}
		else  if (FNeedsDocBcm(bcm))
			{
/* for a non-repeatable command, blow away repeat of some
	fragile commands, like dyad move/copy and insert text  bz
	This may be overkill, but easier that checking every
	non-repeatable command. We can safely ignore those non-
	repeatables that don't require a doc, since they won't
	change anything. bz [bc]
*/
			switch (bcm)
				{
			case bcmGoto:
			case bcmSearch:
			case bcmNextCell:
			case bcmPrevCell:
			case bcmBeginRow:
			case bcmEndRow:
			case bcmTopColumn:
			case bcmBottomColumn:
			case imiWnd1:
			case imiWnd2:
			case imiWnd3:
			case imiWnd4:
			case imiWnd5:
			case imiWnd6:
			case imiWnd7:
			case imiWnd8:
			case imiWnd9:
			case bcmFindNext:
			case bcmNextField:
			case bcmPreviousField:
			case bcmNextMwd:
			case bcmPrevMwd:
			case bcmOtherPane:
			case bcmPgvPrevPage:
			case bcmPgvNextPage:
			case bcmNextDr:
			case bcmPrevDr:
			case bcmArrangeWnd:
			case bcmSplit:
			case bcmSizeWnd:
			case bcmMoveWnd:
			case bcmZoomWnd:
			case bcmRestoreWnd:
			case bcmShowAll:
			case bcmSelectAll:
			case bcmExtendSel:
			case bcmShrinkExtend:
				break;

			default:
				TestInvalAgain();
				}
			}
		}
}


/* C M D  R U N  A T M  O N  D O C */
/* %%Function:CmdRunAtmOnDoc %%Owner:bradch */
CmdRunAtmOnDoc(atm, doc)
uns atm;
{
	extern BOOL vfDisableAutoMacros;
	extern HPSY HpsyFromBcmElg();
	extern HWND vhwndStartup;
	extern HPSYT vhpsyt;
	extern int mwCur;
	BCM bcm;
	char stAtm [ichMaxAtm];

	Assert(atm < atmMax);


	/* Skip auto macro if shift key is down */
	if (vfDisableAutoMacros || (GetAsyncKeyState(VK_SHIFT) & 0x8000))
		return cmdOK;
	

	/* Figure out name of macro */
	CopyCsSt(stAuto, stAtm);
	CatCsSt(mpatmst[atm], stAtm);


	/* See if macro exists for this doc */
	if ((bcm = BcmOfSt(stAtm)) == bcmNil || 
			HpsyFromBcmElg(vhpsyt, bcm, ElgFromDoc(doc), 0) == hpNil)
		return cmdOK;


	/* Kill the copyright box if necessary... */
	if (atm == atmExec && vhwndStartup != NULL)
		EndStartup();


	/* Make the document current so the macro can run on it */
	if (selCur.doc != doc)
		{
		Assert(doc != docNil);
		if (PdodDoc(doc)->wwDisp == wwNil)
			return cmdOK;
		SetMw(PwwdWw(PdodDoc(doc)->wwDisp)->mw);
		}


	/* Finally, run the macro */
	return CmdExecBcmKc(bcm, kcNil);
}



