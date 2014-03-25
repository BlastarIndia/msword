#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "doc.h"
#include "props.h"
#include "sel.h"
#include "disp.h"
#include "ch.h"
#include "prm.h"
#include "props.h"
#include "inter.h"
#include "debug.h"
#include "ruler.h"
#include "cmd.h"
#include "error.h"
#include "border.h"

#ifdef WIN
#include "keys.h"
#include "message.h"
#include "prompt.h"
#endif


#ifdef PROTOTYPE
#include "selectsp.cpt"
#endif /* PROTOTYPE */

/* E X T E R N A L S */

extern struct SEL       selCur;
extern struct SEL       selDotted;
extern struct PAP       vpapFetch;
extern struct CA        caPara;
extern struct CA        caSect;
extern struct DOD       **mpdochdod[];
extern struct CA		caTap;
extern struct TCC			vtcc;
extern int              vssc;
extern struct PREF      vpref;
extern int              wwCur;
extern struct WWD       **hwwdCur;
extern BOOL             vfSeeSel;

extern int              vdocTemp;
extern struct UAB       vuab;
extern struct AAB	vaab;
extern struct CHP       vchpFetch;
extern struct RULSS     vrulss; /* ruler sprm state */
extern struct MERR      vmerr;


#ifdef MAC
extern int              vwwHdr;
#endif /* MAC */



/* F  B E G I N  S S C */
/* enter into ssc mode. save primary selection, change highlight
to secondary.
vssc has already been set to the particular ssc operation.
*/
/*  %%Function:  FBeginSsc  %%Owner:  bobz       */

FBeginSsc()
{

	if (selCur.fColumn)
		SelectRow(&selCur, selCur.cpFirst, selCur.cpLim);
	else  if (WinMac(0,vpref.fVol1) || selCur.fBlock)
		{

#ifdef MAC
ErrMsg:
		ErrorEid(eidInvalidSelection, "FBeginScc");
ErrRet:
		UnprotectLlc();
#else
ErrMsg:
ErrRet:
		ErrorEid(eidInvalidSelection, "FBeginScc");
#endif /* MAC */

		vssc = sscNil;
		return fFalse;
		}

	if (vssc == sscMove)
		{
		int fRet = FDelCkBlockPsel(&selCur, rpkNil, NULL,
				fTrue /*fReportErr*/, fFalse /* fSimple */);
		TurnOnSel(&selCur);
		if (!fRet)
			goto ErrRet;
		}

/* make sure that insert line is on */

	ShowInsertLine(&selCur);

	Assert(selDotted.fNil);
/* this is to establish a default starting place for cursor movement */
	selDotted = selCur;
	selDotted.pa = paDotted;
	selDotted.sk = skNil;

#ifdef MAC
/* handle llc */
	AppendLlcSz(selCur.fIns ?
			SzFrameKey(" from", fromLLC) : 
			SzFrameKey(" to", toLLC) );
	DrawLlc();
#endif /* MAC */


	return fTrue;
}


/* E N D  S S C */
/* test for correct termination of ssc mode. Perform operation and exit
mode.
*/
#ifdef WIN

/*  %%Function:  CmdEndSsc  %%Owner:  bobz       */

CmdEndSsc(fDoIt, pcmb)
int fDoIt;
CMB * pcmb;

#else

EndSsc(fDoIt)
int fDoIt;

#endif
{
	BOOL fMove;
	struct SEL selFrom;
	struct SEL selTo;
	int fSaveChp;
	struct DOD *pdod;
	CP cp;
	int cmd = cmdOK;

#ifdef WIN
	int fAutoDelete = vpref.fAutoDelete;
#else
	int fAutoDelete = fTrue;
#endif

#ifdef WIN
	extern KMP **vhkmpDyadic;
	extern KMP  **hkmpCur;

	if (pcmb != NULL && pcmb->fRepeated)
		{
		/* Opus repeats to selCur from:
			vaab.caAgain for copy or move ( not vuab.ca like mac ),
			vaab.hgrpprl for CopyLooks (hard coded).
			(copylooks repeat handled in cmdrepeat)
		
		Note that selDotted must always represent the current doc and
		window, which will be in selCur, so PSelActive will work.
		So, in case vaab.caAgain is in another doc/ww, we do this hack:
		make selDotted a copy of selCur. Fill up selFrom with repeat
		data but give it wwNil so noone tries to use the ww, which
		may be unknown, and so we don't try to erase the sel
		later. Check at turnoffsel for selDotted so we don't turn
		it off, ie on when we turn off selCur.
		
		Set up selFrom
			enough so we can get through this for repeat.
		*/

		selTo = selCur;
		selDotted = selCur;
		selFrom.ww = wwNil;  /* since source ww unknown, may have changed */
		selFrom.ca = vaab.caAgain;
		selFrom.sk = (selFrom.ca.cpFirst == selFrom.ca.cpLim) ?
				skIns : skSel;
		selFrom.cpAnchor = selFrom.cpFirst;
		SetSelCellBits(&selFrom);
		}
	else
#endif /* WIN */
			{

#ifdef MAC   /* WIN will just reject the move later anyway */
			if (selDotted.fColumn)
				SelectRow(&selDotted, selDotted.cpFirst, selDotted.cpLim);
#endif /* MAC */


			if (selCur.fIns
#ifdef WIN
					/* Opus mouse copy looks always goes from selDotted to selCur 
				but keyboard form, which sets up the keymap, is the normal order
					*/
			|| (vssc == sscCopyLooks && (vhkmpDyadic == hNil))

#endif /* WIN */

					)
				{
				selFrom = selDotted;
				selTo = selCur;
				}
			else
				{
				selFrom = selCur;
				selTo = selDotted;
				}
			}
	selFrom.fHidden = fTrue;
	selTo.fHidden = fTrue;

/* end the vssc state in which selCur.ww may not be == wwCur */

#ifdef WIN
	if (pcmb == NULL || !pcmb->fRepeated)
#endif /* WIN */
		TurnOffSel(&selDotted);

	TurnOffSel(&selCur);
	if (selDotted.fNil)
		goto LBeep;
	Assert(selDotted.ww == wwCur);

	fMove = fFalse;

#ifdef WIN
	/* Mac is letting these go through after turning column sels
		into rows. The lower lvel code doesn't really handle tables
		and messes us up for fields. Charles suggested just restricting
		it again rather than figuring out how to make the copy/move stuff
		work with tables. Mac will put up with the move swallowing
		chars into the table. bz
	*/
	if (selFrom.fTable || selTo.fTable)
		{
		selDotted.sk = skNil;  /* can't reset before test above */
		goto LBeep;
		}
#endif
	selDotted.sk = skNil;

#ifdef WIN
	/* can't do dyadic ops if target doc is locked for editing */
	if (PdodDoc(selTo.doc)->fLockForEdit)
		goto LBeep;
#endif /* WIN */

	/* bz - we allow overlaps during dyadic operations until the operation
		is actually performed, when we complain and throw out the 2ndary
		selection. This helps the case of page-upping into a large
		selection
	*/
	/* cannot move/copy into itself */

#ifdef MAC
	if (FOverlapCa(&selTo.ca, &selFrom.ca))
#else /* WIN - do in line here because this is the only NON-DEBUG caller */
		if (selTo.ca.doc == selFrom.ca.doc &&
				selFrom.ca.cpLim > selTo.ca.cpFirst &&
				selFrom.ca.cpFirst < selTo.ca.cpLim)
#endif /* WIN */
			{
			goto LBeep;
			}

#ifdef WIN
	/* punted for Mac, bug 4797 */
	pdod = PdodDoc(selFrom.doc);
	if (pdod->dk == dkDispHdr && selTo.doc == pdod->doc)
		{
		/* prevent wiping out a header display doc by copy/move over
			the section mark that owns the header */
		cp = CpPlc(PdodDoc(selTo.doc)->hplcsed, pdod->ised + 1) - ccpSect;
		if (selTo.cpFirst <= cp && cp < selTo.cpLim)
			goto LBeep;
		}
#endif
	if (!fDoIt)
		{
LBeep:
		if (!fDoIt)
			Beep();
		else
			{
			ErrorEid(eidInvalidSelection, "EndScc");
			cmd = cmdError;
			}
		}
	else
		{
		switch (vssc)
			{
		case sscMove:
			if (!FDelCkBlockPsel(&selFrom, rpkNil, NULL, fTrue, fFalse))
				goto LBeep;
			fMove = fTrue;
		case sscCopy:
			/* prohibit move of entire table into a cell, but allow
				text within cells to be copied. Valid for mac and opus bz
			*/
			if (selTo.fWithinCell && (FTableInPca(&selFrom.ca) && !selFrom.fWithinCell))
				goto LBeep;
			if (!fAutoDelete)
				{
				/* just shrink to to a point. Do before
				CmdMoveCopy so FDelCk... will be done
				properly
				*/
				selTo.cpLim = selTo.cpFirst;
				}

			cmd = CmdMoveCopy(fMove, &selFrom.ca, &selTo);
			break;
		case sscCopyLooks:
/* make destination window current amd set selCur to selTo */
			/* Opus note: selCur is always == selto for
			mouse copy formatting.
			for kb processing, they are different if !selCur.fIns.
			bz
			*/

			if (FNeRgch (&selCur.ca, &selTo.ca, sizeof (struct CA))
					|| selTo.ww != wwCur)
				{
#ifdef MAC
				ActivateWw(selTo.ww);
#endif
				NewCurWw(selTo.ww, fTrue);
				SetSelWw(&selCur, wwCur);
				SetSelCurSels(&selTo);
				}

#ifdef WIN
			if ((*hwwdCur)->wk == wkMacro)
				goto LBeep;
#endif

			CopyLooksDyadic(fFalse /* fCab */, &selFrom);
			}
		}
/* end mode */
	if (selCur.ww != wwCur)
		{
#ifdef MAC
		ActivateWw(selCur.ww);
#endif
		NewCurWw(selCur.ww, fFalse);
		vuab.selsAfter = *((struct SELS *)&selCur);
		vuab.wwAfter = wwCur;
		}

	/* TurnonSel will wipe out transient char props put by
			sscCopyLooks onto an insert point. Save and restore chp
		*/

	if (fSaveChp = (vssc == sscCopyLooks && selCur.fIns))
		blt (&selCur.chp, &selFrom.chp, cwCHP);
	vssc = sscNil;
	TurnOnSel(&selCur);
	if (fSaveChp)
		{
		blt (&selFrom.chp, &selCur.chp, cwCHP);
		selCur.fUpdateChp = fFalse;
		}
	/* Win code for repeat case bz */
	if (selFrom.ww != wwCur Win(&& selFrom.ww != wwNil))
		{
/* adjust ins point left behind the eod eop. This really belongs to AdjustSels,
but doing it here is more conservative from the testing standpoint. 
test case: move last para of a doc to doc in different ww. */
		int ww;
		for (ww = WwDisp(selFrom.doc, wwNil, fTrue); ww != wwNil;
				ww = WwDisp(selFrom.doc, ww, fTrue))
			{
			struct SELS *psels = &PwwdWw(ww)->sels;
			if (psels->fIns && psels->cpFirst > CpMacDocEdit(psels->doc))
				psels->cpLim = psels->cpFirst = CpMacDocEdit(psels->doc);
			}
		}
	vfSeeSel = fTrue;
	UnprotectLlc();
	InvalLlc();
	return (cmd);
}


/* C M D  M O V E  C O P Y */
/* move or copy with replacing destination.
This includes the checks, undo, and selection after the destination.
*/
/*  %%Function:  CmdMoveCopy  %%Owner:  bobz       */

CmdMoveCopy(fMove, pcaFrom, pselTo)
int fMove; 
struct CA *pcaFrom;
struct SEL *pselTo;
{
	struct CA caFrom, caTo;
	CP cpRef, dcpFrom, cpT;
	struct CA caT, caT2;
	struct SELS selsT;
	int cchInsertedEop;

	caFrom = *pcaFrom;
	if (pselTo->fBlock || pselTo->fTable ||
			!FDelCkBlockPsel(pselTo, rpkCa, pcaFrom, fTrue, fFalse))
		{
		ErrorEid(eidCantUseSel,"CmdMoveCopy");
		return cmdError;
		}
	caTo = pselTo->ca;


/* *************** bz 8-22-88 ********************** */
	/* Chrism and I agreed that these restrictions are mostly still needed.
	For dyadic move, you don't want to move a section mark into a
	header doc, because, if the section mark is for that header, the act
	of deleting the section mark deletes the header text where the
	section mark was being moved. Similarly, for a footnote reference
	you could move it into text that would be deleted because the reference
	mark is no longer in the main doc.
		
	This tests may still be somewhat more restrictive than needed, but we
	didn't think it worth the effort to narrow them down further.
	*/

#ifdef MAC    /* replace whole section for WIN */
	/* change mostly because we needed to do the same thing
		for annotations and footnotes, so I reordered WIN
		code. bz
	*/

	if (fMove)
		{
		BOOL fHdr = fFalse;
		if (!PdodDoc(caFrom.doc)->fShort)
			{
/* check moving sections into header/footer document */
			if ((vwwHdr != wwNil && DocBaseWw(vwwHdr) == caTo.doc) ||
					PdodDoc(caTo.doc)->fDispHdr)
				{
				CacheSectCa(pcaFrom);
				if (caSect.cpLim <= pcaFrom->cpLim &&
						pcaFrom->cpLim < CpMacDoc(caFrom.doc))
					{
					ErrorEid(eidIllegalMoveToHeadFoot,"CmdMoveCopy");
					return cmdError;
					}
				fHdr = fTrue;
				}
/* check for illegal move of text with footnote refs to footnote text */
			if (PdodDoc(caFrom.doc)->docFtn != docNil &&
					(fHdr || PdodDoc(caTo.doc)->fFtn))
				{
/* we are moving text from doc with footnotes to footnote text or header.
Determine if there are any references in the source */
				CpFirstRef(caFrom.doc, pcaFrom->cpFirst,
						&cpRef, edcDrpFtn);
				if (cpRef < pcaFrom->cpLim)
					{
					ErrorEid(eidIllegalFNPlacement,"CmdMoveCopy");
					return cmdError;
					}
				}
			}
		}
/* check for moving or copying from footnotes over footnote references in
	same doc */
/* target is docTo, source is docFrom */
	if (PdodDoc(caFrom.doc)->fFtn && PdodDoc(caFrom.doc)->doc == caTo.doc)
		{
		CpFirstRef(caTo.doc, pselTo->cpFirst, &cpRef, edcDrpFtn);
		if (cpRef < pselTo->cpLim)
			{
			ErrorEid(eidCantReplaceFN,"CmdMoveCopy");
			return cmdError;
			}
		}

#else     /* WIN */

	if (!FValidMomSub(&caFrom, &caTo, fMove))
		return cmdError;

#endif /* ~MAC */



	/* note ucmMove used explicitly here even if ucmMovesp is
		define because undo special cases ucmMove (caMove gets
		the source and undo has special code) 
	*/
	if (fMove)
		{
		if (!FSetUndoB1(ucmMove, uccMove, &caTo /* ca will be replaced in FsetundoDelete, so any ca is ok here bz  */) ||
				(DcpCa(&caTo) != cp0 && !FSetUndoDelete(&caTo, fFalse)))
			return cmdError;
		}
/* ca is for CopySp's benefit. */
	else  if (!FSetUndoB1(ucmCopySp, uccPaste, &(pselTo->ca)))
		return cmdError;

/* if inserting a table, make sure it's at a para bound */
	cchInsertedEop = 0;
	if (FInTableDocCp(caFrom.doc, caFrom.cpFirst))
		{
		CacheTc(pselTo->ww, caFrom.doc, caFrom.cpFirst, fFalse, fFalse);
		if (caFrom.cpLim >= vtcc.cpLim)
			{
			/* not just text within table; row will be copied */
			cpT = caTo.cpFirst;
			if (!FEopBeforePca(&caTo))
				return cmdNoMemory;
			if (cpT != caTo.cpFirst)
				{
				cchInsertedEop = cchEop;
				if (caTo.doc == caFrom.doc && 
						caFrom.cpFirst >= caTo.cpFirst - ccpEop)
					{
					caFrom.cpFirst += ccpEop;
					caFrom.cpLim += ccpEop;
					}
				}
			}
		}

	dcpFrom = DcpCa(&caFrom);
	if (!FMoveCopyFromTo(fMove, fWin /* fRM */, &caFrom, &caTo, fFalse))
		return cmdNoMemory;

#ifdef MAC
	ActivateWw(pselTo->ww);
#endif
	NewCurWw(pselTo->ww, fTrue);
	caTo.cpFirst -= cchInsertedEop;
	SetSelsIns(&selsT, caTo.doc, caTo.cpFirst);
	PushInsSels(&selsT);
	selCur.ww = wwCur;
	selCur.doc = caTo.doc;
	SelectPostDelete(caTo.cpLim);
	PushInsSelCur();

	SetUndoAfter(&caTo);
/* this is for move's benefit */
	if (fMove)
		SetUndoAfterMove(&caFrom, pselTo->ww);

	if (cchInsertedEop)
		{
		DirtyOutline(caTo.doc);
		caTo.cpFirst += cchInsertedEop;
		}
#ifdef WIN
	vaab.caAgain = caTo;
	vaab.caAgain.cpLim = caTo.cpFirst + dcpFrom;
	SetAgain (fMove ? bcmMoveSp: bcmCopySp);
#else
	SetAgainCa(fMove ? ucmDiaMove : ucmDiaCopy, accDiadic, ascCopy, &caTo);
#endif

	return cmdOK;
}



#ifdef WIN

/*  %%Function:  FValidMomSub  %%Owner:  bobz       */

FValidMomSub(pcaFrom, pcaTo, fMove)
struct CA *pcaFrom;
struct CA *pcaTo;
BOOL fMove;
{

	struct DOD *pdod;

/* *************** bz 8-22-88 ********************** */
	/* Chrism and I agreed that these restrictions are mostly still needed.
	For dyadic move, you don't want to move a section mark into a
	header doc, because, if the section mark is for that header, the act
	of deleting the section mark deletes the header text where the
	section mark was being moved. Similarly, for a footnote reference
	you could move it into text that would be deleted because the reference
	mark is no longer in the main doc.
		
	This tests may still be somewhat more restrictive than needed, but we
	didn't think it worth the effort to narrow them down further.
	*/

	/* What is happening here:
		You cannot move into a header/footer, footnote, or annotation
		subdoc either section marks, footnote references or
		annotation references.
	
		1. Check if From doc is a mother doc. If not, will not
			have section marks or text with references no need to check
			most of this stuff.    .
		2. If To doc is a subdoc (header, ftn, atn),
			check for moving section breaks
			check for footnote/annotation refs
	*/

	if (fMove && !PdodDoc(pcaFrom->doc)->fShort)  /* from main doc? */
		{
		pdod = PdodDoc(pcaTo->doc);

		if (pdod->fDispHdr || pdod->fHdr)
			{      /* Dest is header doc */
/* check moving sections into sub document */
			CacheSectCa(pcaFrom);
			if (caSect.cpLim <= pcaFrom->cpLim &&
					pcaFrom->cpLim < CpMacDoc(pcaFrom->doc))
				{
				ErrorEid(eidCantMoveSections,"FValidMomToSub");
				return fFalse;
				}
			}
/* check for illegal move of text with footnote/ annotation
refs to sub doc */
		if (!FValidSub(pdod, pcaFrom, edcDrpFtn, eidCantMoveFNANNRefs))
			return (fFalse);
		if (!FValidSub(pdod, pcaFrom, edcDrpAtn, eidCantMoveFNANNRefs))
			return (fFalse);
		}   /* from main doc, move */



/* check for moving or copying from ftn/atns over ftn/atn references in
same doc. Problem is deleting the references in the to doc */
/* target is docTo, source is docFrom */

	if ((vpref.fAutoDelete || fMove)  &&
			(pdod = PdodDoc(pcaFrom->doc))->doc == pcaTo->doc)
		{
		if (!FValidSub(pdod, pcaTo, edcDrpFtn, eidCantReplaceFNANNRefs))
			return (fFalse);
		if (!FValidSub(pdod, pcaTo, edcDrpAtn, eidCantReplaceFNANNRefs))
			return (fFalse);
		}
	return fTrue;

}


/*  %%Function:  FValidSub  %%Owner:  bobz       */

FValidSub(pdodBase, pca, edc, eid)
struct DOD *pdodBase;
struct CA *pca;
int edc, eid;
{
	CP cpRef;
	int docSub;

	if (edc == edcDrpFtn)
		{
		if (!pdodBase->fFtn)
			return fTrue;
		docSub = PdodDoc(pca->doc)->docFtn;
		}
	else
		{
		if (!pdodBase->fAtn)
			return fTrue;
		docSub = PdodDoc(pca->doc)->docAtn;
		}

	if (docSub != docNil)
		{

/* we are moving text from doc with footnotes/annotations to subdoc.
Determine if there are any references in the source */

		CpFirstRef(pca->doc, pca->cpFirst,
				&cpRef, edc);

		if (cpRef < pca->cpLim)
			{
			ErrorEid(eid,"FValidSub");
			return fFalse;
			}
		}

	return fTrue;
}


#endif /* WIN */






/* F  M O V E  C O P Y  F R O M  T O */
/* core of move with replace.
Returns true iff not out of memory.
Does not disturb docUndo, although it uses it.
Destination will be deleted.
pcaFrom is updated to point to the place where source came from
pcTo is updated to point to destination (both in post-move cp space)
in outline move, table bits are not copied.
*/
/*  %%Function:  FMoveCopyFromTo  %%Owner:  bobz       */

FMoveCopyFromTo(fMove, fRM, pcaFrom, pcaTo, fOutline)
BOOL fMove, fRM;
struct CA *pcaFrom, *pcaTo;
/* FReplaceCps can't deal with copies from/to the same doc, so use undo
buffer as intermediate storage.
Plan:
Delete To.
if fMove
	From => suffix undo; suffix undo => To
else
	From => prefix undo; prefix undo => To; delete prefix undo.
if moving:
	delete From.
note: move into a footnote document requires that the source is
	deleted as the last action.
also note: if moving, From remains in undo.  Because of revision marking,
	moving text changes its properties and we need a clean copy for undo.

On return, pcaTo refers to the new text AND what was left after a possible
autodelete w/revision marking.  PcaFrom refers to whatever's left of it
after deletion (because of fMove) with revision marking
		
*/
{

#ifdef WIN
	int fAutoDelete = vpref.fAutoDelete;
#else
	int fAutoDelete = fTrue;
#endif
	struct CA caT, caT1, caT2, caT3, caNil;
	int fInTable = fFalse, fClearTable = fFalse, fMoveToTable = fFalse;
	struct DOD **hdod;
	struct PLC **hplcpad;
	/* since docUndo is used as the source below, and always has
		fformatted set, we need to get the formatted states of the 
		source and dest docs before the replace, so we can set fFormatted
		afterward.
		For WIN we need to include the dest revmarking, because a replace
		could format with revision marks.
	*/

	BOOL fFormatted =
			PdodDoc(pcaFrom->doc)->fFormatted |
			PdodDoc(pcaTo->doc)->fFormatted
			Win(| PdodMother(pcaTo->doc)->dop.fRevMarking);

	PcaSetNil(&caNil);

/* Check for operand overlap for safety */
	Assert (!FOverlapCa(pcaFrom, pcaTo));

	if (FInTableDocCp(pcaTo->doc, pcaTo->cpFirst))
		{
		if (!fAutoDelete)
			fInTable = fTrue;
		else
			{
			CacheTc(wwNil, pcaTo->doc, pcaTo->cpFirst, fFalse, fFalse);
			if (vtcc.cpLim > pcaTo->cpLim)
				fInTable = fTrue;
			}
		CpFirstTap(pcaTo->doc, pcaTo->cpFirst);
		caT2 = caTap;
		InvalCp(&caT2);
		fMoveToTable = !fOutline;
		}

	if (!fInTable && FInTableDocCp(pcaFrom->doc, pcaFrom->cpFirst))
		{
		/* The check for clearing the table is more complex.  We want to know if
			we have only table characters.  Since we do not allow table selections
			to have dyadic operations performed on them, the entire selection
			must reside inside a single row */
		CacheTc(wwNil, pcaFrom->doc, pcaFrom->cpFirst, fFalse, fFalse);
		if (pcaFrom->cpLim < vtcc.cpLim)
			fClearTable = fTrue;
		}

	if (!fAutoDelete)
		{
		/* just shrink to to a point */
		pcaTo->cpLim = pcaTo->cpFirst;
		}
	else
		{
		if (!FSetUndoDelete(pcaTo, fFalse))
			return fFalse;
		caT1 = *pcaTo;
		if (fRM)
			{
			FDeleteRM(pcaTo);
			/* caTo says what's left, adjust caT1 to how much was deleted */
			caT1.cpLim -= DcpCa(pcaTo);
			}
		else
			{
			FDelete(pcaTo);
			pcaTo->cpLim = pcaTo->cpFirst;
			}
		if (vmerr.fMemFail)
			return fFalse;
		AssureNestedPropsCorrect(pcaTo, fFalse);
		AdjustCaReplace(pcaFrom, &caT1, &caNil, fFalse);
		}

	if (FInTableDocCp(pcaTo->doc, pcaTo->cpFirst))
		{
		CpFirstTap(pcaTo->doc, pcaTo->cpFirst);
		caT2 = caTap;
		InvalCp(&caT2);
		}

/* save From in undo prefix */
	if (fMove)
		{
		CP cpT = fAutoDelete ? CpMacDocEdit(docUndo) : cp0;
		FSetUndoDelAdd(pcaFrom, fTrue /* fAppend */);
		/* set vuab.cpFrom after FSetUndoDelAdd because
			FSetUndoDelAdd might whomp on cpFrom! */
		vuab.cpFrom = cpT;
		/* FSetUndoDelAdd doesn't always set fDelete true...
			but we want it true in this case all the time */
		vuab.fDelete = fTrue;

		PcaSetWholeDoc(&caT1, docUndo);
		caT1.cpFirst = vuab.cpFrom;
		}
	else
		{
		FReplaceCps(PcaPoint(&caT1, docUndo, cp0), pcaFrom);
		caT1.cpLim = DcpCa(pcaFrom);
		}

	if (vmerr.fMemFail)
		return fFalse;

/* copy undo prefix (caT1) to To */
/* we clobber the pad info in docUndo so that copies into table will
work ok. UpdateHplcpad will be called later to fix up the pad structure
correctly in the destination doc. we restore the plcpad so undo will work */
	if (fMoveToTable)
		{
		hdod = mpdochdod[docUndo];
		hplcpad = (*hdod)->hplcpad;
		(*hdod)->hplcpad = 0;
		}
	if (fRM)
		FReplaceCpsRM(PcaPoint(&caT2, pcaTo->doc, pcaTo->cpFirst), &caT1);
	else
		FReplaceCps(PcaPoint(&caT2, pcaTo->doc, pcaTo->cpFirst), &caT1);
	if (fMoveToTable)
		(*hdod)->hplcpad = hplcpad;
	if (vmerr.fMemFail)
		return fFalse;
/* delete undo prefix */
	if (!fMove)
		{
		if (!FDelete(&caT1))
			return fFalse;
		}
#ifdef MAC
	CopyStyles(pcaFrom->doc, caT2.doc, caT2.cpFirst, DcpCa(pcaFrom));
#else
	CopyStylesFonts(pcaFrom->doc, caT2.doc, caT2.cpFirst, DcpCa(pcaFrom));
	AssureNestedPropsCorrect(PcaSetDcp(&caT, caT2.doc, caT2.cpFirst, DcpCa(pcaFrom)), fFalse);
#endif

	PcaSetDcp(&caT3, caT2.doc, caT2.cpFirst, DcpCa(pcaFrom));
	if (!fOutline)
		{
		if (fInTable)
			ApplyTableProps(&caT3, fTrue);
		else  if (fClearTable)
			ApplyTableProps(&caT3, fFalse);
		}

/* this accounts for the insert of caT1 */
	/* But not if pcaFrom.cpFirst == caT2.cpFirst */
	if (pcaFrom->doc == caT2.doc && pcaFrom->cpFirst == caT2.cpFirst)
		{
		CP dcp = DcpCa(&caT1) - DcpCa(&caT2);
		pcaFrom->cpFirst += dcp;
		pcaFrom->cpLim += dcp;
		}
	else
		AdjustCaReplace(pcaFrom, &caT2, &caT1, fFalse);
	pcaTo->cpLim = pcaTo->cpFirst + DcpCa(pcaTo) + DcpCa(&caT1);
	if (fMove)
		{
/* delete source text */
		caT2 = *pcaFrom;
		if (fRM)
			{
			FDeleteRM(pcaFrom);
			/* caFrom says what's left, adjust caT2 to how much was deleted */
			caT2.cpLim -= DcpCa(pcaFrom);
			}
		else
			{
			FDelete(pcaFrom);
			pcaFrom->cpLim = pcaFrom->cpFirst;
			}
		if (vmerr.fMemFail)
			return fFalse;
		AssureNestedPropsCorrect(pcaFrom, fFalse);
		AdjustCaReplace(pcaTo, &caT2, &caNil, fFalse);
		}

	/* so text docs do not always get changed to native */
	PdodDoc(pcaTo->doc)->fFormatted = fFormatted;
	return fTrue;
}


/* C O P Y  L O O K S  D Y A D I C */
/* apply formatting (paragraph or char depending on pselFrom) to selCur;
Indications for the selections have already been turned off.
*/
/*  %%Function:  CopyLooksDyadic  %%Owner:  bobz       */

CopyLooksDyadic(fCab, pselFrom)
BOOL fCab;
struct SEL *pselFrom;
{
	int cb;
	int stc;
	int idError;
	BOOL fDocSame;
	CP cpLimParaSel;
	struct STTB **hsttbChpeSrc, **hsttbPapeSrc;
	struct STTB **hsttbChpeDest, **hsttbPapeDest;
	char rgstcCopy[1];
	char grpprl[cbMaxGrpprl];
	struct CHP chp;

	if (fCab)
		{
#ifdef MAC
		ApplyGrpprlSelCur(vrulss.grpprl, vrulss.cbGrpprl, fTrue);
#else
		Assert (fFalse); /* for Opus should never get here */
#endif /* MAC */
		}
	else
		{
		FlushRulerSprms();
		CacheParaCa(&selCur.ca);
		if (pselFrom->sty >= styPara )
			{
			char rgb[258];
			struct PAP pap;
			struct STSH stshSrc;
			struct STSH stshDest;
/* copy para looks.
compute difference: pap of 1st para of selCur to pap of 1st para
of pselFrom->
Express difference as grpprl.
Apply grpprl.
*/
			fDocSame = DocMother(selCur.doc) == DocMother(pselFrom->doc);
			cpLimParaSel = caPara.cpLim;
			pap = vpapFetch;  /* selCur pap */
			CacheParaCa(&pselFrom->ca);

/* want to apply sprmPStc + difference from base of sel2nd */
			if (fDocSame)
/* within same document can just use pap for stc of source selection as the
base */
				stc = vpapFetch.stc;
			else
				{
/* cross document, we need to make sure there is an equivalent for the source
style in the destination by looking it up or copying the source style.
FCopyStyleToDestStsh does both */
				stc = pap.stc;
				RecordStshForDoc(DocMother(pselFrom->doc),
						&stshSrc, &hsttbChpeSrc, &hsttbPapeSrc);
				RecordStshForDoc(DocMother(selCur.doc),
						&stshDest, &hsttbChpeDest, &hsttbPapeDest);
				rgstcCopy[0] = vpapFetch.stc;  /* selFrom style */
#ifdef MAC
				if (FCopyStyleToDestStsh( 
						&stshSrc, hsttbChpeSrc, hsttbPapeSrc,
						DocMother(selCur.doc), &stshDest, hsttbChpeDest, hsttbPapeDest, rgstcCopy,
						1, rgb, fFalse, fFalse, fFalse, fTrue))
#else	 /* WIN */
					if (FCopyStyleToDestStsh(docNil, 
							&stshSrc, hsttbChpeSrc, hsttbPapeSrc,
							DocMother(selCur.doc), &stshDest, hsttbChpeDest, hsttbPapeDest, rgstcCopy,
							1, rgb, fFalse, fFalse, fFalse, &idError))
#endif /* WIN */
						stc = rgb[2 + vpapFetch.stc];
				}
			MapStc(PdodDoc(selCur.doc), stc, &chp, &pap);
/* ignore differences in table properties (again) */
			pap.fInTable = vpapFetch.fInTable;
			pap.fTtp = vpapFetch.fTtp;
			cb = CbGrpprlFromPap(fTrue, &grpprl, &vpapFetch, &pap, fFalse);
			bltb(&grpprl[0], &grpprl[2], cb);
			grpprl[0] = sprmPStc;
			grpprl[1] = stc;
			cb += 2;
			}
		else
			{

/* copy char looks.
compute difference: standardchp - chp of 1st char
of pselFrom.
Express difference as grpprl.
Set to to sprmCDefault
Apply grpprl.
*/

			/* apply all the from props regardless of
				props in the 1st run - handles gray to selection
			*/
			StandardChp (&chp);

			CacheParaCa(&pselFrom->ca);
			FetchCp(pselFrom->doc, pselFrom->cpFirst, fcmProps);
			cb = CbGrpprlFromChp(&grpprl[2], &vchpFetch, &chp);
			grpprl[0] = sprmCDefault;  /* clear, then set props */
			grpprl[1] = 0;
			cb += 2;
			}
		if (cb)
			{
			ApplyGrpprlSelCur(grpprl, cb, fTrue);

/* copy grpprl to again structure for "again" */
			FSetAgainGrpprl(grpprl, cb, ucmFormatting);
			}
		}
}


/*  %%Function:  SetPromptVssc  %%Owner:  bobz       */

SetPromptVssc()
{
	switch (vssc)
		{

#ifdef DEBUG
	default:
		Assert (fFalse);
#endif /* DEBUG */

	case sscMove:
		SetPromptMst(selCur.fIns ? mstMoveFrom : mstMoveTo, pdcMode);
		break;
	case sscCopy:
		SetPromptMst(selCur.fIns ? mstCopyFrom : mstCopyTo, pdcMode);
		break;
	case sscCopyLooks:
		SetPromptMst(selCur.fIns ? mstFormatFrom : mstFormatTo, pdcMode);
		break;
	case sscNil:
		break;
		}
}


/* S E T  U N D O  A F T E R  M O V E */
/* some commands need to give additional operands to undo.
*/
/*  %%Function:  SetUndoAfterMove  %%Owner:  bobz       */

SetUndoAfterMove(pca, ww)
struct CA *pca; 
int ww;
{
	/* FLegalBcm now check both bcm and uac for undo and redo, so we
	should be OK here, don't believe there is any case where someone
	have SetUndoNil before reaching here, if so, we should find out
	how.  */
	Assert(vuab.bcm != bcmNil);

	if (vuab.fDelete)
		{
		vuab.uac = uacMove;
		/* to dirty the source for dyadic move */
		if (pca->doc != docNil)
			PdodDoc(pca->doc)->fDirty = fTrue;
		}
	else
		vuab.uac = uacMoveNil;

	vuab.caMove = *pca;
	vuab.wwTo = ww;
}


