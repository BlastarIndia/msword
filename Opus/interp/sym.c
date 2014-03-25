/* sym.c: top level procedures for SYM subsystem.
*/
#include <qwindows.h>
#include <qsetjmp.h>
#include <el.h>
#include <uops.h>
#include "eltools.h"
#include <rerr.h>

#define EXTERN	extern	/* disable global defs */
#include "priv.h"
#include "sym.h"
#include "_sym.h"

#include "ibcm.h"


extern WORD PASCAL mpsbps [];

SB SbFindNtab();

csconst BYTE mpelvcbVar[] =
{
	0, cbNumVar, cbIntVar, cbSdVar, cbAdVar, cbFormalVar, cbEnvVar};


#define CbVarFromElv(elv)	(FRecordElv(elv) ? cbRecordVar		\
							: mpelvcbVar[elv])

/* %%Function:FInitSym %%Owner:bradch */
BOOL FInitSym()
{
	SB sbNtab;

	Global(fEnableDefine) = TRUE;

	/* Look for a name table segment associated with this macro.
		*/
	if ((Global(sbNtab) = SbFindNtab(HpeliFirst()->elg, 
			HpeliFirst()->elm, TRUE)) == 0)
		{
		return FALSE;
		}

	/* If the frame stack segment doesn't exist, we will create it.
		*/
	if (sbFrame == 0)
		{
		int cb;

		if ((sbFrame = SbAllocEmmCb((cb = cbHash + cbSymQuantum))) == 0)
			return FALSE;
		
		*HpFrame(ibMax) = cb;
		*HpFrame(ibMac) = &FrameVar(rgb);
		*HpFrame(pfhLast) = pfhNil;	/* no frames defined yet */
		}
	Global(pfhCurFrame) = pfhNil;
	
	return TRUE;
}


/* %%Function:FreeSymbolTables %%Owner:bradch */
FreeSymbolTables()
	/* Frees all NTAB segments that currently exist.
	*/
{
	while (sbNtabFirst != 0)
		{
		SB sbNext;
		
		sbNext = ((struct NTAB huge *)HpOfSbIb(sbNtabFirst, 0))->sbNext;
		FreeNtab(sbNtabFirst);
		sbNtabFirst = sbNext;
		}
}


/* %%Function:FTouchElgElm %%Owner:bradch */
FTouchElgElm(elg, elm)
/* Called when a macro is modified and re-tokenized.  If a symbol table segment
* is allocated for the macro, frees it and returns TRUE; otherwise returns
* FALSE.
*/
ELG elg;
ELM elm;
{
	SB sbNtab, sbPrev;

	sbNtab = sbNtabFirst;
	sbPrev = 0;
	while (sbNtab != 0)
		{
		struct NTAB huge *hpntab = HpOfSbIb(sbNtab, 0);

		if (hpntab->elg == elg && hpntab->elm == elm)
			{
			/* Delete this SB from the linked list, then free
				* it and return.
				*/
			if (sbPrev == 0)
				sbNtabFirst = hpntab->sbNext;
			else
				{
				((struct NTAB huge *)HpOfSbIb(sbPrev, 0))
					->sbNext = hpntab->sbNext;
				}
			FreeNtab(sbNtab);
			return TRUE;
			}
		sbPrev = sbNtab;
		sbNtab = hpntab->sbNext;
		}

	/* Didn't find it in the linked list.
		*/
	return FALSE;
}


/* %%Function:FreeNtab %%Owner:bradch */
FreeNtab(sbNtab)
SB sbNtab;
{
	int ipsy;
	struct SY * psy;
	HANDLE hLib;

	/* walk the ntab freeing dkd stuff... */
	SetSbCur(sbNtab);
	for (ipsy = 0; ipsy < cwHash; ipsy += 1)
		{
		for (psy = NtabVar(rgpsyHash)[ipsy]; psy != 0; 
			psy = psy->psyNext)
			{
			if (psy->syt == sytDkd && psy->pdkd->fFreeLib)
				{
				hLib = psy->pdkd->hLib;
				Assert(hLib != NULL);
				ResetSbCur();
				FreeLibrary(hLib);
				SetSbCur(sbNtab);
				}
			}
		}
	ResetSbCur();
	
	FreeEmmSb(sbNtab);
}


/* %%Function:PsyLookupSt %%Owner:bradch */
struct SY *PsyLookupSt(st, fLabel)
/* Looks up a counted string in the symbol table; returns an SY pointer.  If
* not found, the SY is created.  If not fLabel, and the identifier has no
* type-suffix, "#" will be appended to the identifier name argument.
*
* This routine also converts the source string to all-uppercase, and therefore
* should be used with caution.
*/
unsigned char *st;
BOOL fLabel;
{{ /* NATIVE - PsyLookupSt */
	unsigned wHash = 0, cch, ich;
	char *pch;
	char far *lpchWanted = (char far *)st;
	struct SY *psyNew, **ppsyCur;
	BOOL fAppendedType;
	unsigned chSave;
	ELV elv;

	/* Append a type-character to the string if necessary.  This is done
		* by actually altering the source string, and then fixing it after
		* the symbol lookup is complete.
		*/
	cch = (unsigned)*st;
	Assert(cch != 0);
	Assert(fLabel || st[1] > ' ');
	fAppendedType = FALSE;
	switch (st[cch])	/* look at last char of string */
		{
	case '%':
		/* Error: integer variables NYI */
		RtError(rerrAdvancedFeatureUnavailable);
		elv = elvNil;
		break;
	default:
		if (!fLabel)
			{
			chSave = st[++cch];
			st[cch] = '#';		/* default to elvNum */
			st[0]++;		/* increment string count */
			fAppendedType = TRUE;
			}
		/* fall through */
	case '#':
		elv = elvNum;
		break;
	case '$':
		elv = elvSd;
		break;
		}

	/* Find the string's hash value (arcane formula stolen from CS).
		*/
	pch = st + 1;
	for (ich = 0; ich < cch; ich++)
		{
		char ch = *pch++;

		if (FLower(ch))
			*(pch - 1) = ch = ChUpper(ch);
		wHash = (wHash<<5) + (wHash>>11) + ch;
		}

	SetSbCur(Global(sbNtab));				/* /SB\ */
	ppsyCur = &NtabVar(rgpsyHash)[(wHash & 0x7fff) % cwHash];
	while (*ppsyCur != 0)
		{
		char far *lpchFound = (char far *)((*ppsyCur)->st);

		if (*lpchFound == cch)
			{
			ResetSbCur();				/* \SB/ */
			SetSbCur(Global(sbNtab));		/* /SB\ */

			for (ich = 0; ich < cch; ich++)
				{
				if (lpchWanted[ich + 1] != lpchFound[ich + 1])
					break;
				}
			if (ich >= cch)
				{
				psyNew = *ppsyCur;
				ResetSbCur();			/* \SB/ */

				/* If we munged the source string, fix it.
					*/
				if (fAppendedType)
					{
					st[cch] = chSave;	/* fix */
					st[0]--;
					}

				return psyNew;		/* found it */
				}
			}

		/* Didn't find it this time.
			*/
		ppsyCur = &(*ppsyCur)->psyNext;
		}

	/* The symbol isn't in the symbol table, so we will create it.
		*/
	psyNew = (struct SY *)NtabVar(ibMac);

	if ((NtabVar(ibMac) = (unsigned)psyNew + CbOfSy(cch + 1)) >
		NtabVar(ibMax))
		{
		ResetSbCur();					/* \SB/ */
		GrowSb(Global(sbNtab));
		SetSbCur(Global(sbNtab));			/* /SB\ */
		}

	/* Initialize the new SY, and link it into the hash chain.
		*/
	(*ppsyCur = psyNew)->psyNext = (struct SY *)0;
	psyNew->plabFirst = (struct LAB *)0;
	psyNew->syt = sytNil;
	psyNew->elv = elv;
	psyNew->pvarArray = psyNew->pvarScalar = (struct VAR *)0;
	BLTBX(lpchWanted, LpOfHp(HpOfSbIb(SbCur(), psyNew->st)),
			cch + 1);

	ResetSbCur();						/* \SB/ */

	/* If we munged the source string, fix it.
		*/
	if (fAppendedType)
		{
		st[cch] = chSave;		/* fix */
		st[0]--;
		}

#ifdef DEBUG
	if (fToSym)
		ToPsy(psyNew);
#endif
	return psyNew;
}}


/* %%Function:FRecordPsy %%Owner:bradch */
BOOL FRecordPsy(psy, fArray)
struct SY *psy;
BOOL fArray;
{
	struct VAR * pvar;

	if (psy == psyNil)
		return FALSE;
	
	pvar = *HpOfSbIb(Global(sbNtab),
		fArray ? &psy->pvarArray : &psy->pvarScalar);

	return (pvar != 0 &&
		((struct FH *)pvar > Global(pfhCurFrame) ||
		(struct FH *)pvar < Global(pfhFirstProcFrame) &&
		HpvarOfPvar(pvar)->fShared) &&
		FRecordElv(HpvarOfPvar(pvar)->elv));
}


/* %%Function:PvarFromPsyElx %%Owner:bradch */
struct VAR * PvarFromPsyElx(psy, elv, fArray, elx)
/* Gets the currently-scoped VAR definition of this SY.  If there is no such
* definition, a new VAR is created.
*
* If psy == psyNil, a temporary VAR is created, not attached to any SY.
* If elv != elvNil, elv over-rides the elv implied by the symbol name.  The
*	only such valid elv values are elvFormal and elvRecord*.
*/
struct SY *psy;
ELV elv;
BOOL fArray;
ELX elx;
{
	struct SY huge * hpsy = HpOfSbIb(Global(sbNtab), psy);
	struct VAR * pvar;
	struct VAR * huge * hppvarLink;
	struct VAR huge * hpvar;

	int cbNeeded;

	/* Assert a bunch of stuff, just on general principles
		*/
	Assert(!(psy == psyNil && elv == elvNil) &&
			(elv == elvNil || FRecordElv(elv) ||
			elv == elvFormal || psy == psyNil));
	Assert(elv != elvEnv || psy == psyNil);

	if (elv == elvNil)
		elv = hpsy->elv;

	if (psy != psyNil)
		{
		/* Make sure we are looking at a variable, so that pvarScalar is the
			right item of the struct (i.e. not a pproc or pdkd).  Allow a
			non-variable sy if we're looking at the array pointer, because it
			is unique in the structure and is tested against zero elsewhere.
		*/
		if (!fArray && hpsy->syt != sytNil)	/* Make sure sy is a variable */
			RtError(rerrSyntaxError);

		hppvarLink = HpOfSbIb(Global(sbNtab),
						fArray ? &psy->pvarArray
							: &psy->pvarScalar);
		}

	/* See if we should return the address of an existing VAR.
		*/
	if (psy != psyNil && elv != elvFormal)
		{
		pvar = *hppvarLink;

		/* Look for a VAR with the appropriate type, that we are in
			* the scope of.
			*/
		if (pvar != 0 &&
			((struct FH *)pvar > Global(pfhCurFrame) ||
				(struct FH *)pvar < Global(pfhFirstProcFrame) &&
				HpvarOfPvar(pvar)->fShared))
			{
			/* If we expected a record, make sure we got one.
				*/
			if (elv == elvNilRecord &&
				!FRecordElv(HpvarOfPvar(pvar)->elv))
				/* Error: Type mismatch error (record expected) */
				RtError(rerrTypeMismatch);

			return pvar;
			}
		}
	if (!Global(fEnableDefine))
		return (struct VAR *)0;

	if (elv == elvNilRecord)		/* want generic record var? */
		/* Error: undefined record variable (if so, fail) */
		RtError(rerrSyntaxError);

	/* Create a new VAR.
		*/
	cbNeeded = CbVarFromElv(fArray ? elvArray : elv);
	if (*HpFrame(ibMac) + cbNeeded > *HpFrame(ibMax))
		GrowSb(sbFrame);
	pvar = (struct VAR *)*HpFrame(ibMac);
	*HpFrame(ibMac) += cbNeeded;

	/* Initialize the new VAR.
		*/
	hpvar = HpvarOfPvar(pvar);
	BLTBCX((BYTE)0, LpOfHp(hpvar), cbNeeded);
	hpvar->psy = psy;
	if (fArray)
		{
		hpvar->elv = elvArray;
		hpvar->ai.elv = elv;
		}
	else
		{
		hpvar->elv = elv;
		if (FRecordElv(elv))
			hpvar->elx = elx;
		}

	/* Link the new VAR into the list headed by the SY.
		*/
	if (psy != psyNil)
		{
		hpvar->pvarPrev = *hppvarLink;
		*hppvarLink = pvar;
		}

#ifdef DEBUG
	if (fToVar)
		OutPvar(pvar);
	if (fToSym)
		DumpFrame();
#endif
	return pvar;
}


/* %%Function:PvarFromPsy %%Owner:bradch */
struct VAR *PvarFromPsy(psy, elv, fArray)
struct SY *psy;
ELV elv;
BOOL fArray;
{
	return PvarFromPsyElx(psy, elv, fArray, (ELX) 0);
}



/* %%Function:PenvGetCur %%Owner:bradch */
ENV *PenvGetCur(fCreate)
/* Returns a pointer (into sbFrame) to the ON ERROR environment defined for
* the current procedure invocation.  If none exists, a new one is created
* (if fCreate) or 0 is returned (if !fCreate).
*/
BOOL fCreate;
{
	struct FH huge *hpfhCur = HpOfSbIb(sbFrame, Global(pfhCurFrame));
	ENV *penvFrame;

	if ((penvFrame = hpfhCur->penv) != 0)
		return penvFrame;
	else
		{
		struct VAR *pvarNew;

		/* No ENV currently exists.
			*/
		if (!fCreate)
			return (ENV *)0;

		pvarNew = PvarFromPsy(psyNil, elvEnv, FALSE);
		return (hpfhCur->penv = &pvarNew->env);
		}
	/*NOTREACHED*/
}


/* %%Function:PprocFromPsy %%Owner:bradch */
struct PROC *PprocFromPsy(psy, carg)
/* Gets a pointer to the PROC associated with the SY.
* If the PROC does not exist, and carg >= 0, the PROC is created with room
* for carg arguments.
*/
struct SY *psy;
int carg;
{
	struct SY huge *hpsy = HpOfSbIb(Global(sbNtab), psy);

	switch (hpsy->syt)
		{
	default:
		Assert(FALSE);
	case sytNil:
		{
		struct PROC *pproc;

		if (carg < 0)			/* create a new one? */
			return (struct PROC *)0;	/* no */

		/* Create a new PROC record.
			*/
		hpsy->syt = sytProc;
		hpsy->pproc = pproc = *HpNtab(ibMac);

		if ((*HpNtab(ibMac) = (unsigned)pproc + CbOfProcCarg(carg)) >
			*HpNtab(ibMax))
			{
			GrowSb(Global(sbNtab));
			}

		SetSbCur(Global(sbNtab));
		pproc->carg = carg;
		pproc->lib = libNil;	/* indicate "not yet defined" */
		pproc->psy = psy;
		ResetSbCur();
		}
		/* fall through */
	case sytProc:
		return hpsy->pproc;
		}
}


/* %%Function:PdkdFromPsy %%Owner:bradch */
DKD * PdkdFromPsy(psy, idktMacParam, hLib, dktRet, lpproc, rgdktParam, fFree)
/* Gets a pointer to the DKD associated with the SY.
* If the DKD does not exist, and idktMacParam >= 0, the DKD is created 
* with room for idktMacParam arguments.
*/
struct SY * psy;
int idktMacParam;
HANDLE hLib;
DKT dktRet;
FARPROC lpproc;
DKT * rgdktParam;
BOOL fFree;
{
	struct SY huge *hpsy = HpOfSbIb(Global(sbNtab), psy);

	switch (hpsy->syt)
		{
	default:
		Assert(FALSE);
	case sytNil:
		{
		int idkt;
		DKD * pdkd, huge * hpdkd;

		if (idktMacParam < 0)		/* create a new one? */
			return (struct PROC *)0;	/* no */

		/* Create a new DKD record.
			*/
		hpsy->syt = sytDkd;
		hpsy->pdkd = pdkd = *HpNtab(ibMac);

		if ((*HpNtab(ibMac) = (unsigned)pdkd + 
			CbOfDkdIdktMac(idktMacParam)) > *HpNtab(ibMax))
			{
			struct SSEG huge * hpsseg = HpOfSbIb(Global(sbNtab), 0);

			if ((hpsseg->ibMax = CbReallocSb(Global(sbNtab), 
					hpsseg->ibMax + cbSymQuantum, HMEM_MOVEABLE)) == 0)
				{
				if (fFree)
					FreeLibrary(hLib);
				/* Error: out of memory */
				RtError(rerrOutOfMemory);
				}
			}

		SetSbCur(Global(sbNtab));
		pdkd->hLib = hLib;
		pdkd->fFreeLib = fFree;
		pdkd->dktRet = dktRet;
		pdkd->idktMacParam = idktMacParam;
		pdkd->lppasproc = lpproc;
		ResetSbCur();
		hpdkd = HpOfSbIb(Global(sbNtab), pdkd);
		for (idkt = 0; idkt < idktMacParam; idkt += 1)
			hpdkd->rgdkt[idkt] = *rgdktParam++;
		}
		/* FALL THROUGH */
	
	case sytDkd:
		return hpsy->pdkd;
		}
}


/* %%Function:PlabFromPsy %%Owner:bradch */
struct LAB *PlabFromPsy(psy, ln, pproc)
/* Gets a pointer to the LAB defined for the SY in the specified procedure.  If
* one does not exist, a new one is created (and marked as "new" by setting
* the tsp (token stream position) to a null value).  If psy is NULL, ln
* (label number) is used instead.
*/
struct SY *psy;
WORD ln;
struct PROC *pproc;	/* what procedure the label is associated with */
{
	struct LAB *plab;
	unsigned char rgchT[3];
	SB sbNtab = Global(sbNtab);

	if (psy == 0)
		{
		/* Get a psy representing a line number.
			*/
		rgchT[0] = sizeof(WORD); 	/* char count */
		*(WORD *)&rgchT[1] = ln;	/* 2-byte line number */

		psy = PsyLookupSt(rgchT, TRUE);
		}

	SetSbCur(sbNtab);
	for (plab = psy->plabFirst; plab != 0; plab = plab->plabNext)
		{
		if (plab->pproc == pproc)
			{
			ResetSbCur();
			return plab;
			}
		}

	/* Create a new LAB record.
		*/
	plab = NtabVar(ibMac);
	if ((NtabVar(ibMac) = (unsigned)plab + sizeof(struct LAB)) >
		NtabVar(ibMax))
		{
		ResetSbCur();
		GrowSb(sbNtab);
		SetSbCur(sbNtab);
		}
	plab->lib = libNil;		/* initialize */

	/* Link it in
		*/
	plab->pproc = pproc;
	plab->plabNext = psy->plabFirst;
	psy->plabFirst = plab;

	ResetSbCur();

#ifdef DEBUG
	if (fToSym)
		OutPlab(plab, psy);
#endif
	return plab;
}


/* %%Function:BeginPproc %%Owner:bradch */
VOID BeginPproc(pproc)
/* Sets up a new frame for the specified procedure.  If pproc is 0, the new
* frame is for module-level variables.
*/
struct PROC *pproc;
{
	struct FH *pfhPrev = Global(pfhCurFrame);
	struct FH *pfhNew;
	ELG elg;
	ELM elm;

	/* We just have to add a new frame header (1 word), but grow the
		* frame stack if we have to.
		*/
	if (*HpFrame(ibMac) + sizeof(struct FH) > *HpFrame(ibMax))
		GrowSb(sbFrame);

	if (pproc == 0)
		{
		elg = HpeliFirst()->elg;
		elm = HpeliFirst()->elm;
		}

	SetSbCur(sbFrame);

	pfhNew = FrameVar(ibMac);
	pfhNew->pfhPrev = pfhPrev;
	pfhNew->pproc = pproc;
	pfhNew->penv = (ENV *)0;
	if (pproc == 0)
		{	/* -- first frame created in this module -- */
		pfhNew->elg = elg;
		pfhNew->elm = elm;
		}
	else
		{	/* -- inherit (elg, elm) from previous frame -- */
		pfhNew->elg = pfhPrev->elg;
		pfhNew->elm = pfhPrev->elm;
		}

	FrameVar(ibMac) += sizeof(struct FH);

	ResetSbCur();

	Global(pfhCurFrame) = *HpFrame(pfhLast) = pfhNew;
	if (Global(pprocCur) == 0)
		{
		Global(pfhGlobalFrame) = pfhNew;
		Global(pfhFirstProcFrame) = pfhNew;
		}
	Global(pprocCur) = pproc;
}


/* %%Function:EndProcedure %%Owner:bradch */
VOID EndProcedure()
	/* Tears down the frame for the current procedure.  If "fAll", tears down
	* all frames in the current EL invocation.
	*/
{
	struct FH huge *hpfhCur = HpOfSbIb(sbFrame, Global(pfhCurFrame));
	struct VAR huge *hpvar;
	unsigned ibMac = *HpFrame(ibMac);

	/* Walk the procedure's frame and unlink all VAR's found.
		*/
	for (hpvar = &hpfhCur->rgvar; IbOfHp(hpvar) < ibMac;
			hpvar = (char huge *)hpvar + CbVarFromElv(hpvar->elv))
		{
		struct VAR *huge*hppvarLink;
		struct SY *psy = hpvar->psy;
		ELV elv = hpvar->elv;

		/* Change the psy's link field so it points to the pvar
			* which was shadowed by this one.
			*/
		elv = hpvar->elv;

		if (psy != psyNil)
			{
			hppvarLink = HpOfSbIb(Global(sbNtab),
						(elv == elvArray) ? &psy->pvarArray :
						&psy->pvarScalar);
			*hppvarLink = hpvar->pvarPrev;
			}

		/* Free array or string data associated with the pvar
			*/
		switch (elv)
			{
		default:
			if (FRecordElv(elv) && hpvar->rek.hcab != 0)
				{
				FreeRek(hpvar->rek);
				hpvar->rek.hcab = 0;
				}
			break;
		case elvSd:
			FreeSd(hpvar->sd);
			break;
		case elvArray:
			if (hpvar->ai.elv == elvSd)
				FreeStringsAd(hpvar->ai.ad);
			FreeAd(hpvar->ai.ad);
			break;
			}
		}

	/* Reset the current allocated-size in the frame segment, so that new
		* variables created will go on the end of the previous frame.
		*/
	*HpFrame(ibMac) = IbOfHp(hpfhCur);

	/* Remember the previous frame location.  This may be pfhNil if we just
		* freed the top-level frame in a module.
		*/
	if ((Global(pfhCurFrame) = hpfhCur->pfhPrev) == pfhNil)
		Global(pprocCur) = 0;
	else
		Global(pprocCur) = HpfhFromPfh(Global(pfhCurFrame))->pproc;

	/* Update the "last frame" pointer.
		*/
	*HpFrame(pfhLast) = Global(pfhCurFrame);

#ifdef DEBUG
	if (fToSym && Global(pfhCurFrame) != pfhNil)
		DumpFrame();
#endif
}


/* %%Function:EndAllProcedures %%Owner:bradch */
VOID EndAllProcedures()
	/* Tears down all frames in the current EL invocation.
	*/
{
	while (Global(pfhCurFrame) != pfhNil)
		EndProcedure();
}


/* %%Function:PvarBeginTemps %%Owner:bradch */
struct VAR *PvarBeginTemps()
{
	return (struct VAR *)*HpFrame(ibMac);
}


/* %%Function:EndTemps %%Owner:bradch */
VOID EndTemps(pvarSaved)
struct VAR *pvarSaved;
{
	struct VAR *pvarT, huge *hpvarT, *pvarMac = *HpFrame(ibMac);

	/* Free array and string data associated with temporary variables.
		*/
	for (pvarT = pvarSaved; pvarT < pvarMac;
			pvarT = (char *)pvarT + CbVarFromElv(hpvarT->elv))
		{
		switch ((hpvarT = HpvarOfPvar(pvarT))->elv)
			{
		default:
			break;
		case elvSd:
			FreeSd(hpvarT->sd);
			break;
			}
		}

	/* Free the space the temps were stored in.
		*/
	*HpFrame(ibMac) = pvarSaved;
}


/* An API procedure
*/
/* %%Function:ElvEnumerateVar %%Owner:bradch */
ELV ElvEnumerateVar(iFrame, iLocal, st, pelg, pelm, phpval)
WORD iFrame;
WORD iLocal;
char *st;
ELG *pelg;
ELM *pelm;
VOID huge **phpval;
{
	struct FH *pfhCur, *pfhNext;
	struct VAR *pvar, huge *hpvar;
	ELG elg;
	ELM elm;
	
	/* Prevent hang when no macro is suspended */
	if (sbFrame == 0)
		return elvEndStack;

	SetSbCur(sbFrame);					/* /SB\ */
	pfhCur = *HpFrame(pfhLast);
	pfhNext = pfhNil;
	while (pfhCur != pfhNil && iFrame > 0)
		{
		pfhNext = pfhCur;
		pfhCur = pfhCur->pfhPrev;
		iFrame--;
		}
	elg = pfhCur->elg;
	elm = pfhCur->elm;
	ResetSbCur();						/* \SB/ */

	if (pfhCur == pfhNil || iFrame > 0)
		return elvEndStack;

	*pelg = elg;
	*pelm = elm;

	if (pfhNext == pfhNil)
		pfhNext = *HpFrame(ibMac);
	pvar = &pfhCur->rgvar;
	while (pvar < (struct VAR *)pfhNext && iLocal > 0)
		{
		pvar = (char *)pvar + CbVarFromElv(HpvarOfPvar(pvar)->elv);
		iLocal--;
		}
	if (pvar >= (struct VAR *)pfhNext || iLocal > 0)	
		{
		return (pfhCur == *HpFrame(pfhLast)) ? elvEndModuleFrame
								: elvEndProcFrame;
		}

	/* Found a variable.
		*/
	hpvar = HpvarOfPvar(pvar);
	if (hpvar->psy == 0)
		BLTBX(LstCode("(temp)"), (char far *)st, 7);
	else
		{
		unsigned cchCopied;
		struct SY huge *hpsyVar;
		struct PROC *pproc;
		struct SY huge *hpsyProc;
		unsigned sbVarNtab;

		if ((sbVarNtab = SbFindNtab(elg, elm, FALSE)) == 0)
			RtError(rerrOutOfMemory);

		hpsyVar = HpOfSbIb(sbVarNtab, hpvar->psy);
		pproc = HpfhFromPfh(pfhCur)->pproc;
		hpsyProc = HpOfSbIb(sbVarNtab,
					((struct PROC huge *)
						HpOfSbIb(sbVarNtab, pproc))->psy);

		cchCopied = 0;
		if (pproc != 0)
			{
			struct PROC huge *hpproc;
			struct SY huge *hpsyProc;
			
			hpproc = HpOfSbIb(sbVarNtab, pproc);
			hpsyProc = HpOfSbIb(sbVarNtab, hpproc->psy);

			BLTBH(&hpsyProc->st[1], (char huge *)&st[1],
					cchCopied = hpsyProc->st[0]);
			}
		st[cchCopied++ + 1] = '!';
		BLTBH(&hpsyVar->st[1], (char huge *)&st[cchCopied + 1],
				hpsyVar->st[0]);
		cchCopied += hpsyVar->st[0];

		st[0] = cchCopied;
		}

	*phpval = HpOfSbIb(sbFrame, &pvar->num);
	return HpvarOfPvar(pvar)->elv;
}


/* Private SYM procedures */


/* %%Function:GrowSb %%Owner:bradch */
GrowSb(sb)
SB sb;
{
	struct SSEG huge * hpsseg = HpOfSbIb(sb, 0);

	if ((hpsseg->ibMax =
			CbReallocSb(sb, hpsseg->ibMax + cbSymQuantum, HMEM_MOVEABLE))
		== 0)
		{
		/* Error: out of memory */
		RtError(rerrOutOfMemory);
		}
}


/* %%Function:SbFindNtab %%Owner:bradch */
SB SbFindNtab(elg, elm, fOkCreate)
/* Locates the name table associated with a macro, or creates one (if
* fOkCreate) if it doesn't exist.
*
* This procedure does not use the ELLD (i.e. "Global(...)"), and thus can be
* called even when no macro is running (e.g. from "ElvEnumerateVars()").
*/
ELG elg;
ELM elm;
BOOL fOkCreate;
{
	SB sbPrev, sbNtab;
	int csbAllocated;

	sbPrev = 0;
	csbAllocated = 0;
	for (sbNtab = sbNtabFirst; sbNtab != 0;
			sbPrev = sbNtab,
			sbNtab = ((struct NTAB huge *)HpOfSbIb(sbNtab, 0))->sbNext)
		{
		struct NTAB huge *hpntab;

		Assert(mpsbps[sbNtab] != 0);
		csbAllocated++;

		hpntab = HpOfSbIb(sbNtab, 0);
		if (hpntab->elg == elg && hpntab->elm == elm)
			break;
		}
	if (sbNtab == 0)
		{
		int cb, iwHash;

		/* Didn't find (elg, elm).
			*/
		Assert(fOkCreate);

		if (csbAllocated + 1 >= csbNtabMax)
			RtError(rerrInternalError);
		
		if ((sbNtab = SbAllocEmmCb(cbHash + cbSymQuantum)) == 0)
			return 0;
		
		if (sbPrev != 0)
			{
			((struct NTAB huge *) HpOfSbIb(sbPrev, 0))->sbNext =
				sbNtab;
			}
		else
			sbNtabFirst = sbNtab;

		SetSbCur(sbNtab);				/* /SB\ */

		/* Initialize segment variables, and clear the hash table.
			*/
		NtabVar(elg) = elg;
		NtabVar(elm) = elm;
		NtabVar(sbNext) = 0;
		NtabVar(ibMax) = cbHash + cbSymQuantum;
		NtabVar(ibMac) = &NtabVar(rgb);
		NtabVar(libScan) = 0L;
		NtabVar(pprocScan) = (struct PROC *)0;
		for (iwHash = 0; iwHash < cwHash; iwHash++)
			NtabVar(rgpsyHash)[iwHash] = (struct SY *)0;

		ResetSbCur();					/* \SB/ */
		}

	return sbNtab;
}


/* C L E A R  U S R  D L G  V A R S */
/* Look at ALL variables for user-defined dialog records and free the
   associated cabs */
ClearUsrDlgVars()
{
	struct FH huge * hpfhCur;
	struct VAR * pvarLim, huge *hpvar;
	
	Assert(sbFrame != 0);

	pvarLim = *HpFrame(ibMac);
	for (hpfhCur = HpOfSbIb(sbFrame, Global(pfhCurFrame)); 
		IbOfHp(hpfhCur) != pfhNil; 
		hpfhCur = HpOfSbIb(sbFrame, hpfhCur->pfhPrev))
		{
		for (hpvar = &hpfhCur->rgvar; IbOfHp(hpvar) < pvarLim; 
				hpvar = (char huge *) hpvar + CbVarFromElv(hpvar->elv))
			{
			if (FRecordElv(hpvar->elv) && hpvar->elx == ibcmUserDialog &&
					hpvar->rek.hcab != 0)
				{
				FreeCab(hpvar->rek.hcab);
				hpvar->rek.hcab = 0;
				}
			}
		
		pvarLim = IbOfHp(hpfhCur);
		}
}


