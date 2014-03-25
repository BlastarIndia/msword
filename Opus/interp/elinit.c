/* elinit.c -- OpEL initialization (and cleanup) functions */

#include <qwindows.h>
#include <qsetjmp.h>
#define ELMAIN		/* omit global declarations from "el.h" */
#include <el.h>
#include "eltools.h"
#include <uops.h>
#include <rerr.h>

/* Definitions of variables available outside the EL system.
* The declarations are in "el.h".
*/
BOOL fElActive = FALSE;

#define EXTERN			/* global variable defs go here */
#include "priv.h"
#include "sym.h"
#include "exp.h"
#include "_exec.h"

#ifdef DEBUG
#define BATCH
#endif


#define LibCur()	(Global(libBufStart) + Global(ibBufCur))


VOID RtError();
VOID StopHeli();
VOID CleanupEl();

unsigned CbOfSsbPenv(ENV *);
int DpbExitCoroutine();
int ResumeCoroutine(struct SSB huge *);



/* H E L I  N E W */
/* Allocates and initializes an ELI structure (in sbTds) and returns a huge
* pointer to it, or returns 0 if it is not possible to create an ELI now.
* An ELI can not be created while another ELI has been suspended.
*/
/* %%Function:HeliNew %%Owner:bradch */
ELI **HeliNew(elg, elm, pfnCchReadSource, pfnDebug, pfnGetInfoElx, w, l)
ELG elg;
ELM elm;
WORD (*pfnCchReadSource)();
VOID (*pfnDebug)();
VOID (*pfnGetInfoElx)();
WORD w;
LONG l;
{
	ELI ** heli, * peli;

	if (sbTds == 0)
		{
		/* Create sbTds and allocate a new global block. */

		Assert(sbNtabFirst == 0);

		if ((sbTds = SbAllocEmmCb(cbHeapInit)) == 0)
			goto LFailRet;

		CreateHeap(sbTds);

		helgdGlobal =
				HpOfSbIb(sbTds, PpvAllocCb(sbTds, CbOfElgdCev(cevQuantum)));

#ifdef DEBUG
		ClearTestOptions();
		ElGlobal(cadAllocated) = ElGlobal(csdAllocated) = 0;
#endif

		/* Indicate that no ELI's currently exist (at least until we
			create one). */
		ElGlobal(heliFirst) = (ELI **) 0;	/* no ELI's active */

		/* Initialize the new sbTds. */
		if (!FInitExpTds() || !FInitExecTds() || !FInitAdsd())
			goto LFailRet;

		InitEldiCache();
		}
#ifdef DEBUG
	else
		{
		ELI huge * hpeli;
		/* Make sure we can create a new ELI now. */
		hpeli = HpeliFirst();
		Assert(IbOfHp(hpeli) == 0 || !hpeli->fSuspended);
		}
#endif /* DEBUG */

	/* Create the new ELI.
		*/
	if ((heli = PpvAllocCb(sbTds, sizeof(ELI))) == 0)
		goto LFailRet;

	/* Initialize the new ELI.
		*/
	SetSbCur(sbTds);

	peli = *heli;
	peli->heliNext = ElGlobal(heliFirst);
	peli->fStarted = peli->fSuspended = FALSE;
	peli->elg = elg;
	peli->elm = elm;
	peli->pfnCchReadSource = pfnCchReadSource;
	peli->pfnDebug = pfnDebug;
	peli->pfnGetInfoElx = pfnGetInfoElx;
	peli->w = w;
	peli->l = l;

	ResetSbCur();

	ElGlobal(heliFirst) = heli;

	return heli;

LFailRet:
	CleanupEl();
	return NULL;
}


/* %%Function:RerrRunHeli %%Owner:bradch */
RERR RerrRunHeli(heli)
/* Starts an initialized ELI running at the start, or wherever the previous
* invocation left off.  This function will return when the macro reaches its
* end, or StopHeli() is called.
*/
ELI **heli;
{
	extern RERR vrerr;
	ELI huge *hpeli = HpeliOfHeli(heli);
	RERR rerr;

	vrerr = rerrNil;

	Assert(heli == ElGlobal(heliFirst) &&
			(!hpeli->fStarted || hpeli->fSuspended));

	if (hpeli->fStarted)
		{
		/* The ELI has been activated before, so we will resume it.
			*/
#ifdef DEBUG
		if (fToMain)
			PrintT("MAIN: resuming a coroutine ...\n", 0);
#endif
		hpeli->fSuspended = FALSE;

		ResumeCoroutine(HpOfSbIb(sbTds,
				*HpOfSbIb(sbTds, ElGlobal(hssb))));

		/* we get back if the coroutine stack moved */
		return rerrInternalError;
		}

	/* The ELI hasn't been started yet. */
	if (hpeli->heliNext == 0)
		{
		ENV envSav;
		ENV envT;

		/* We're starting the top-level ELI, so we should be able to
			resume if necessary. */

		envSav = ElGlobal(env);
		if (SetJmp(&envT) != 0)
			{
			/* The macro was suspended. */
			heli = ElGlobal(heliFirst);

			hpeli = HpeliOfHeli(heli);	/* re-validate */

			if (!hpeli->fSuspended)
				ElGlobal(env) = envSav;

			return hpeli->rerr;
			}

		ElGlobal(env) = envT;
		}

	rerr = RerrActivateHeli(heli);

	return rerr;
}


/* S T O P  H E L I */
/* Stops execution of the currently running ELI.  If "rerr" is rerrSuspend
* or rerrStop, the ELI may be continued.  If this happens, StopHeli() will
* return.
*
* If StopHeli() is called with any other RERR value, it will never return.
*
* This may be called from statement handlers to cause an error, or by the
* *pfnDebug function in case the BREAK key is pressed, etc.
*/
/* %%Function:StopHeli %%Owner:bradch */
VOID StopHeli(heli, rerr)
ELI **heli;
RERR rerr;
{
	ELI huge *hpeli;

	Assert(heli == ElGlobal(heliFirst));

	switch (rerr)
		{
	default:
LDefault:
		/* We will stop the current EL invocation permanently.  Put
			* the "rerr" value where it can be seen, and jump to envError.
			* This will go back to RerrActivateHpeli().
			*/
		(hpeli = HpeliOfHeli(heli))->rerr = rerr;
		DoJmp(&Global(envError), 1);
		Assert(FALSE); /* NOT REACHED */

	case rerrSuspend:
	case rerrStop:
		if (!FOnlyHeli(heli))
			goto LDefault;
			{
			int cbSsb, dpb;
			struct SSB **hssb;
			BOOL fElActiveSav;
			ENV envT;

		/* Create an SSB to hold the saved stack.
			*/
			envT = ElGlobal(env);
			cbSsb = CbOfSsbPenv(&envT);

			if ((ElGlobal(hssb) = hssb = PpvAllocCb(sbTds, cbSsb)) == 0)
				{
				rerr = rerrOutOfMemory;
				goto LDefault;
				}

		/* Suspend macro execution, then continue when it resumes.
			*/
			(hpeli = HpeliOfHeli(heli))->rerr = rerr;
			hpeli->fSuspended = TRUE;

			fElActiveSav = fElActive;
			fElActive = FALSE;

			dpb = DpbExitCoroutine(&envT, rerr,
					HpOfSbIb(sbTds, *HpOfSbIb(sbTds, hssb)));

			fElActive = fElActiveSav;

			FreePpv(sbTds, hssb);

			if (dpb != 0)
			/* Error: coroutine stack moved */
				RtError(rerrInternalError);
			break;
			}
	/* end switch (rerr) */
		}
}


/* %%Function:RerrActivateHeli %%Owner:bradch */
RerrActivateHeli(heli)
ELI **heli;
{
	int icsrStackPtrSav, ieltStackPtrSav, ievStackPtrSav;
	struct ELLD elld;
	struct ELLD * pelldSav;
	ELI huge * hpeli;

	Assert(ElGlobal(heliFirst) == heli);

	hpeli = HpeliOfHeli(heli);

	if (!hpeli->fStarted)
		hpeli->fStarted = TRUE;
	else
		{
		Assert(hpeli->fSuspended);
		hpeli->fSuspended = FALSE;
		}

	/* Set up the interpreter data block (allocated on the stack). */
	pelldSav = pelldGlobal;
	pelldGlobal = &elld;

	/* Save where we are on the stack, so we can set an upper limit on
		stack usage. */
	Global(wStackStart) = &heli;

	/* Save some global state information. */
	icsrStackPtrSav = ElGlobal(icsrStackPtr);
	ieltStackPtrSav = ElGlobal(ieltStackPtr);
	ievStackPtrSav = ElGlobal(ievStackPtr);

	/* Set an ENV to handle run-time errors. */
	if (SetJmp(&elld.envError) != 0)
		{
		/* The macro aborted with an error; restore the global state.
			*/
		ElGlobal(icsrStackPtr) = icsrStackPtrSav;
		ElGlobal(ieltStackPtr) = ieltStackPtrSav;
		if (ElGlobal(ievStackPtr) != ievStackPtrSav)
			PopToIev(ievStackPtrSav);
		EndAllProcedures();		/* free frames */

		fElActive -= 1;
		pelldGlobal = pelldSav; /* BAC */

		hpeli = HpeliOfHeli(heli);	/* re-validate */
		return hpeli->rerr;
		}

	/* Subsystem initializations. */
	if (!FInitSym())
		goto LFailRet;
	InitExecGlobals();
	InitExp(HpeliFirst()->heliNext);

	if (CbStackAvailable(&heli) < cbElStackNeeded)
		goto LFailRet;


	/* Motivate! */

	fElActive += 1;
	BeginPproc(0);
	ReadGlobalDims();

	if (EldExecutePproc(PprocFindPsy(PsyLookupSt(StCode("MAIN"),
			FALSE)), 0, eviNil, hpNil) != eldNormalStep)
		{
		Assert(FALSE);
		}

	/* Verify that the macro's state has been properly cleaned up. */
	Assert(icsrStackPtrSav == ElGlobal(icsrStackPtr));
	Assert(ieltStackPtrSav == ElGlobal(ieltStackPtr));
	Assert(ievStackPtrSav == ElGlobal(ievStackPtr));

	EndProcedure();		/* free module-level frame */
	pelldGlobal = pelldSav; /* BAC */

	fElActive -= 1;
	return rerrNil;
	
LFailRet:
	return rerrOutOfMemory;
}


/* %%Function:FreeHeli %%Owner:bradch */
VOID FreeHeli(heliFree)
/* Deallocates an ELI.  The ELI must be the most recent one created, and must
* not be currently running.
*/
ELI **heliFree;
{
	ELI huge *hpeliFree = HpeliOfHeli(heliFree);
	ELI **heliNext;

	Assert(heliFree == ElGlobal(heliFirst));

	FTouchElgElm(hpeliFree->elg, hpeliFree->elm);

	heliNext = hpeliFree->heliNext;
	FreePpv(sbTds, heliFree);

	if ((ElGlobal(heliFirst) = heliNext) == NULL)
		{
		FreeDialogs();
		ElClose(0);
		}
}


/* %%Function:SetDebug %%Owner:bradch */
VOID SetDebug(elg, elm, pfnDebug)
/* Locates an ELI matching (elg,elm), and sets its debug function to
* pfnDebug.
*/
ELG elg;
ELM elm;
VOID (*pfnDebug)();
{
	ELI **heli = ElGlobal(heliFirst);

	while (heli != 0)
		{
		ELI huge *hpeli = HpeliOfHeli(heli);

		if (hpeli->elg == elg && hpeli->elm == elm)
			{
			hpeli->pfnDebug = pfnDebug;
			return;
			}
		heli = hpeli->heliNext;
		}

	Assert(FALSE);	/* didn't find it -- maybe we should be nicer */
}


/* ElvEnumerateVar defined in SYM subsystem.
*/


/* %%Function:RtError %%Owner:bradch */
VOID RtError(rerr)
RERR rerr;
{
	ENV *penv, huge *hpenv;
	extern RERR vrerr;
#ifdef BATCH
	extern BOOL vfBatchMode;
#endif /* BATCH */

	Assert(fElActive);

	vrerr = rerrNil;
	switch (rerr)
		{
	default:
		/* Trappable error.  We jump to the error handler if an ENV
			* has been defined and is valid (i.e. no "ON ERROR GOTO 0")
			* and the ERR# variable is zero (so we're not already in an
			* error handler).
			*/
		if ((penv = PenvGetCur(FALSE)) != 0 &&
				((hpenv = HpOfSbIb(sbFrame, penv))->fPcodeEnv ||
				hpenv->snEnv != 0) &&
				FSetErrRerr(rerr))
			{
			ENV env;

			ClearAbortKeyState();

			BLTBH(hpenv, (char huge *)&env, sizeof(ENV));
			DoJmp(&env, 1);
			/*NOTREACHED*/
			}
		/* Can't jump to an error handler, so stop the macro.
			*/
		/* FALL THROUGH */

	case rerrHalt:
	case rerrSuspend:
	case rerrStop:
	case rerrOutOfMemory:
	case rerrSyntaxError:
#ifdef BATCH
		/* "report" runtime errors when in batchmode */
		if (vfBatchMode)
			BatchModeError(SzCode("Macro runtime error encountered."), 
					NULL, rerr, 0);
#endif /* BATCH */
		StopHeli(ElGlobal(heliFirst), rerr);
		}
}


/* %%Function:CleanupEl %%Owner:bradch */
VOID CleanupEl()
	/* Uninitializes the EL system and frees all allocated memory.  If HeliNew()
	* is called after calling CleanupEl(), the EL system will be re-initialized.
	*/
{
	extern WORD PASCAL mpsbps [];

	if (sbTds == 0)
		{
		Assert(sbFrame == 0 && sbStrings == 0 && sbArrays == 0);
		return;		/* not initialized anyway */
		}

	FreeEmmSb(sbTds);
	
	if (sbFrame != 0)
		FreeEmmSb(sbFrame);
	if (sbStrings != 0)
		FreeEmmSb(sbStrings);
	if (sbArrays != 0)
		FreeEmmSb(sbArrays);
	
	sbTds = sbFrame = sbStrings = sbArrays = 0;

	FreeSymbolTables();
	Assert(sbNtabFirst == 0);
}


typedef WORD MERR;
/* %%Function:ElMathError %%Owner:bradch */
ElMathError(merr)
	/* Called from the application's MathError() handler when "fElActive" is set;
	* indicates a math error while evaluating a BASIC operator.
	*/
{
	switch (merr)
		{
	case 4:	/* numbers not yet defined by math pack API */
		RtError(rerrDivisionByZero);

	case 1:
	case 2:
		RtError(rerrOverflow);

	default:
		/* Error: math error #merr */
		RtError(rerrInternalError);
		}
}


#ifdef REMOVE
#ifndef DEBUG
/* Null definition of SetTestOptions().  If DEBUG, this procedure is defined
* in "to.c".
*/
/* %%Function:SetTestOptions %%Owner:NOTUSED */
SetTestOptions(sz)
char *sz;
{
}


#endif
#endif

/* ---------------------------------------------------------------------- */
/* ELDI cache maintenance
/* ---------------------------------------------------------------------- */

/* %%Function:InitEldiCache %%Owner:bradch */
InitEldiCache()
{
	int iedc;

	SetSbCur(sbTds);

	/* Mark all the entries as unused, and clear the next-to-flush index.
		*/
	for (iedc = 0; iedc < cedcCache; iedc++)
		TdsVar(rgedc)[iedc].ieldi = ieldiNil;
	TdsVar(iedcNextFree) = 0;

	ResetSbCur();
}


/* %%Function:HeldiFromIeldi %%Owner:bradch */
ELDI **HeldiFromIeldi(ieldi)
/* Returns a handle to a record in sbTds containing the appropriate ELDI
* record.
* SHAKES: sbTds
*/
WORD ieldi;
{
	int iedc;
	struct EDC *pedc;
	ELDI **heldiNew;
	WORD cbEldi;

	SetSbCur(sbTds);
	pedc = TdsVar(rgedc);

	/* Look for the item in the cache.
		*/
	for (iedc = 0; iedc < cedcCache; iedc++, pedc++)
		{
		if (pedc->ieldi == ieldi)
			{
			ELDI **heldiT = pedc->heldi;
			ResetSbCur();
			return heldiT;
			}
		}

	/* Didn't find it ... free an item if necessary.
		*/
	iedc = TdsVar(iedcNextFree);
	TdsVar(iedcNextFree) = (iedc + 1) % cedcCache;

	pedc = &TdsVar(rgedc)[iedc];
	if (pedc->ieldi != ieldiNil)
		{	/* -- free the existing cache entry -- */
		ELDI **heldiT = pedc->heldi;

		ResetSbCur();
		FreePpv(sbTds, heldiT);
		}
	ResetSbCur();

	cbEldi = CbGetInfoIeldi(ieldi, hpNil);
	if ((heldiNew = PpvAllocCb(sbTds, cbEldi)) == 0)
		RtError(rerrOutOfMemory);
	CbGetInfoIeldi(ieldi, HpeldiFromHeldi(heldiNew));

	SetSbCur(sbTds);
	pedc = &TdsVar(rgedc)[iedc];
	pedc->ieldi = ieldi;
	pedc->heldi = heldiNew;
	ResetSbCur();

	return heldiNew;
}


/* ---------------------------------------------------------------------- */
/* Accepting dialogs from SDM
/* ---------------------------------------------------------------------- */

/* %%Function:ExpectHeldi %%Owner:bradch */
VOID ExpectHeldi(heldi)
/* Sets up the EL state so that "FSaveElHcab()" can be called (once) to pass
* a CAB from SDM to the EL system.
*/
ELDI **heldi;
{
	SetSbCur(sbTds);					/* /SB\ */
	TdsVar(hidSave) = (*heldi)->hid;	/* ok since sbTds */
	TdsVar(hcabSave) = (HCAB) 0;		/* no CAB yet */
	ResetSbCur();						/* \SB/ */
}


/* %%Function:FSaveElHcab %%Owner:bradch */
BOOL FSaveElHcab(hcab, hid, tmc)
HCAB hcab;
HID hid;
TMC tmc;
{
	Assert(hid == ElGlobal(hidSave) && ElGlobal(hcabSave) == (HCAB) 0);

	ElGlobal(hcabSave) = hcab;
	ElGlobal(tmcSave) = tmc;

	return TRUE;
}


/* %%Function:FRunningElgElm %%Owner:bradch */
FRunningElgElm(elg, elm)
ELG elg;
ELM elm;
{
	ELI ** heli, * peli;

	if (sbTds == 0)
		return FALSE;

	SetSbCur(sbTds);

	for (heli = ElGlobal(heliFirst); heli != 0; heli = peli->heliNext)
		{
		peli = *heli;
		if (peli->elg == elg && peli->elm == elm)
			{
			ResetSbCur();
			return TRUE;
			}
		}

	ResetSbCur();

	return FALSE;
}



/* Initializes EXEC stuff in the global-variable block. */
/* %%Function:InitExecGlobals %%Owner:bradch */
InitExecGlobals()
{
	Global(fExpectLabel) = FALSE;
	Global(fEnableCall) = TRUE;

	/* Setup the buffer-reading functions. */
	Global(libBufStart) = Global(libBufEnd) = 0L;
	Global(ibBufCur) = Global(ibBufLim) = 0;
}


/* Called when a new sbTds is being created; initializes the CSR stack. */
/* %%Function:FInitExecTds %%Owner:bradch */
FInitExecTds()
{
	ElGlobal(icsrStackPtr) = -1;

	return (ElGlobal(hcsrStack) = PpvAllocCb(sbTds, 
			ccsrStackMax * sizeof (struct CSR))) != 0;
}


/* Parses the global DIM statements at the beginning of the module.  This is
	the only place where data is defined which is global to all procedures.
	This also defines all SUBs and FUNCTIONs and deals with DECLAREs */
/* %%Function:ReadGlobalDims %%Owner:bradch */
ReadGlobalDims()
{
	ELT elt;

	/* First define a variable to hold temporary records for this macro
		* (used when making a dialog call).  The ELV doesn't matter as long
		* as it's a record ELV other than elvNilRecord.
		*/
	Global(pvarTmpRek) = PvarFromPsy(psyNil, ElvFromIeldi(0), FALSE);

	/* Now read the global definitions in the macro. */
	EltGetCur();		/* pre-fetch */
		{{ /* NATIVE - ReadGlobalDims */
		for (;;)		/* until EOF */
			{
			while (Global(eltCur) == eltLineNolabel)
				EltGetCur();

			HpeliFirst()->lib = LibCur() - 1;

			switch (Global(eltCur))
				{
			case eltDim:
				ExecDim(FALSE);
				break;

			case eltRedim:
				ExecDim(TRUE);
				break;

			case eltSub:
			case eltFunction:
				DefProcedure();
				FSkipToLine(eltEnd, 0);	/* skip procedure body */
				if (LibCur() >= *(LIB huge *)HpNtab(libScan))
					*HpNtab(pprocScan) = (struct PROC *)0;
				EltGetCur();
				break;

			case eltEof:
				*(LIB huge *)HpNtab(libScan) = 0;
				return;

			case eltDeclare:
				DoElDeclare();
				break;

			default:
			/* Error: Invalid global data declaration */
				RtError(rerrSyntaxError);
				}
			}
		}}
}



/* I N I T  A D S D */
/* Initializes the string and array sub-system. */
/* %%Function:FInitAdsd %%Owner:bradch */
FInitAdsd()
{
	if (sbStrings == 0)
		{
		if ((sbStrings = SbAllocEmmCb(cbHeapInit)) == 0)
			return FALSE;

		CreateHeap(sbStrings);
		}

	if (sbArrays == 0)
		{
		if ((sbArrays = SbAllocEmmCb(cbHeapInit)) == 0)
			return FALSE;

		CreateHeap(sbArrays);
		}

	return TRUE;
}


/* %%Function:FSetErrRerr %%Owner:bradch */
BOOL FSetErrRerr(rerr)
/* Attempts to set the variable "ERR#" to rerr.  Fails if the current value
* of ERR# is non-zero.
*/
RERR rerr;
{
	struct VAR *pvar, huge *hpvar;
	NUM numT;
	MTC mtc;

	pvar = PvarFromPsy(PsyLookupSt(StCode("ERR"), FALSE), elvNil, FALSE);
	if (pvar < Global(pfhCurFrame))
		/* Error: illegal to define ERR at module-level */
		RtError(rerrSyntaxError);

	BLTBH(&HpvarOfPvar(pvar)->num, HpOfSbIb(1, &numT), sizeof(NUM));

	LdiNum(&numT);
	CNumInt(0);
	mtc = CmpNum();
	if (rerr == rerrNil || mtc.w2 == 0)
		{
		/* ERR is zero -- set it to the specified value.
			*/
		CNumInt(rerr);
		StiNum(&numT);
		BLTBH(HpOfSbIb(1, &numT), &HpvarOfPvar(pvar)->num, sizeof(NUM));
		return TRUE;
		}
	else
		return FALSE;	/* already non-zero */
}



/* F R O M  C O R O U T . C */

/* Coroutine mechanism */


struct SSB
	{
	unsigned cbStack;
	char *pchOrig;		/* original location of the stack block */
	char *pchDest;		/* where the stack block is copied to */
	int dcbFrame;		/* for temporary use */
	char rgbStack[];
};
#define CbOfSsb(cbStack)	(sizeof(struct SSB) + (cbStack))

struct QF			/* Qcode Frame */
	{
	WORD fPrev;
	WORD bpc;
	WORD sn;
};
#define PqfNextPqf(pqf)	       ((struct QF *)(((struct QF *)(pqf))->fPrev))

#ifdef DEBUG
extern BOOL fAllowFwdJmp;	/* used to disable checking in WINTER */
#endif

#ifdef HYBRID
extern BOOL fAllowFwdJmp;	/* used to disable checking in WINTER */
#endif


static struct SSB huge *hpssbGlobal;
static ENV envGlobal;


/* %%Function:CbOfSsbPenv %%Owner:bradch */
unsigned CbOfSsbPenv(penv)
/* Called by the coroutine, in preparation for suspending itself.  The return
* value indicates the necessary size of the SSB used to save the stack.
*
* After calling this routine, the coroutine should allocate the block, then
* call DpbExitCoroutine().
*/
ENV *penv;
{
	char *pchLow, *pchHigh;

	/* We want to save stuff starting at the beginning of the caller's
		* frame ...
		*/
	pchLow = (char *)&penv + sizeof(ENV *);

	/* ... and ending at the beginning of the frame containing the ENV
		* (i.e. don't save that frame).
		*/
	/* NOTE: penv->fEnv is really an (int *), this poor hungarian comes
		to you from the standard qsetjmp.h file.  I believe fEnv is a 
		pointer to the current frame at SetJmp time. */
	pchHigh = (unsigned)penv->fEnv;

	return CbOfSsb(pchHigh - pchLow);
}


/* %%Function:DpbExitCoroutine %%Owner:bradch */
int DpbExitCoroutine(penv, wReturn, hpssb)
/* Called by a coroutine when it wants to suspend itself.  The current stack
* and environment are saved in *hpssb, and control is transferred to *penv.
*
* This call is similar to DoJmp(penv) except that the call may return (if
* ResumeCoroutine() is called).  If the call does return, the return value
* indicates how much the stack has moved.  All pointers into the region of
* the stack between penv and the current frame should be incremented by
* the return value.
*/
ENV *penv;
WORD wReturn;
struct SSB huge *hpssb;
{
	char *pchLow, *pchHigh;
	ENV env;	/* temporary near-pointer ENV */

	/* Compute the range of stuff we want to save.  See the comments in
		* the above procedure for the explanation of this.
		*/
	pchLow = (char *)&penv + sizeof(ENV *);
	pchHigh = (unsigned)penv->fEnv;

	/* Copy the saved stack into the destination huge pointer.
		*/
	hpssb->cbStack = pchHigh - pchLow;
	hpssb->pchOrig = pchLow;
	BLTBH((char huge *)pchLow, hpssb->rgbStack, pchHigh - pchLow);

	/* Jump to the ENV which indicates we suspended the coroutine.
		*/
	DoJmp(penv, wReturn);

	/* Control flow will never get here; however, a value is returned to
		* the caller of this procedure.  This happens when ResumeCoroutine()
		* is called and restores the stack frames (including our own return
		* address) which were just saved to far memory.  Are we having fun
		* yet?
		*/
}


/* %%Function:ResumeCoroutine %%Owner:bradch */
int ResumeCoroutine(hpssb)
/* Resumes a coroutine which has suspended itself.
*
* Although the return-type is WORD, this function never returns to its
* caller.  Due to heavy stack munging, the returned word appears as the
* return value from DpbExitCoroutine() (which was called by the coroutine
* to suspend itself).
*/
struct SSB huge *hpssb;		/* stack block saved by DpbExitCoroutine() */
{
	ENV envT;
	char *pchRestoreMin, *pchRestoreLim;
	char *pchCurEvalStack, *pchNewEvalStack;
	struct QF *pqfCur;
	int cbCurFrame, dcbStack;

#ifdef DEBUG
	fAllowFwdJmp = TRUE;	/* disable checking in WINTER */
#endif

#ifdef HYBRID
	fAllowFwdJmp = TRUE;	/* disable checking in WINTER */
#endif

	/* The idea is to insert the saved frames on the stack under our own
		* frame.  First find out where we want to insert the stuff to ...
		*/
	pqfCur = (char *)&hpssb + sizeof(struct SSB huge *);
	pchRestoreLim = PqfNextPqf(pqfCur);
	pchRestoreMin = pchRestoreLim - hpssb->cbStack;

	/* Increase the current frame size.
		*/
	hpssbGlobal = hpssb;
	if (SetJmp(&envT) == 0)
		{
		cbCurFrame = envT.cbEnv;

		pchCurEvalStack = (char *)pqfCur - cbCurFrame;
		pchNewEvalStack = pchRestoreMin - cbCurFrame;

		envT.cbEnv += (hpssbGlobal->dcbFrame =
				pchCurEvalStack - pchNewEvalStack);
		DoJmp(&envT, 1);
		}

	if ((dcbStack = pchRestoreMin - hpssb->pchOrig) != 0)
		{
		return dcbStack;
		}

	/* BLT the current frame data up to the beginning (toward 0) of the
		* new frame.
		*/
	BLTB(pchCurEvalStack, pchNewEvalStack, cbCurFrame + sizeof(struct QF));

	/* Change the current "f" value so that it points to the new
		* location of the frame.
		*/
	if (SetJmp(&envGlobal) == 0)
		{
		envGlobal.fEnv = (WORD)envGlobal.fEnv - hpssbGlobal->dcbFrame;
		envGlobal.cbEnv -= hpssbGlobal->dcbFrame;
		DoJmp(&envGlobal, 1);
		}

	/* Do pointer fixups in the block containing saved stack frames.
		* "dcbStack" is the displacement which should be added to all the
		* frame links.
		*/
	if ((dcbStack = pchRestoreMin - hpssb->pchOrig) != 0)
		{
		struct QF *pqfT = IbOfHp(hpssb->rgbStack);
		struct QF *pqfLim = (char *)pqfCur + hpssb->cbStack;

		do
			{
			struct QF *pqfNext, huge *hpqfT;

			hpqfT = HpOfSbIb(SbOfHp(hpssb), pqfT);

			pqfNext = IbOfHp(hpssb->rgbStack) +
					(hpqfT->fPrev - (WORD)hpssb->pchOrig);

			hpqfT->fPrev += dcbStack;

			pqfT = pqfNext;
			}
		while (pqfT < pqfLim);
		/* Assert(pqfT == pqfLim);	(Assert not available here) */
		}

	/* BLT the saved stack frames onto the stack.
		*/
	BLTBH(hpssb->rgbStack, (char huge *)pchRestoreMin, hpssb->cbStack);

#ifdef DEBUG
	fAllowFwdJmp = FALSE;
#endif

#ifdef HYBRID
	fAllowFwdJmp = FALSE;
#endif

	/* Return (to the caller of DpbExitCoroutine()) the amount by which
		* the frames moved.
		*/
	return dcbStack;
}


/* %%Function:FOnlyHeli %%Owner:bradch */
FOnlyHeli(heli)
ELI ** heli;
{
	return HpeliOfHeli(heli)->heliNext == NULL && 
			heli == ElGlobal(heliFirst);
}


