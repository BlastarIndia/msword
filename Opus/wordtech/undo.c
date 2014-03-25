#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "props.h"
#include "sel.h"
#include "doc.h"
#include "disp.h"
#include "debug.h"
#include "cmd.h"
#include "ch.h"
#ifdef MAC
#define WINDOWS
#include "toolbox.h"
#include "prm.h"
#include "ruler.h"
#include "dlg.h"
#endif


#ifdef PROTOTYPE
#include "undo.cpt"
#endif /* PROTOTYPE */

extern struct UAB	vuab;
extern struct SAB	vsab;
extern struct SEL	selCur;
extern int		wwCur;
extern struct PAP	vpapFetch;
extern struct CA	caTap;
extern struct WWD       **mpwwhwwd[];
extern struct DOD       **mpdochdod[];
extern struct WWD	** hwwdCur;

extern int		vfSeeSel;
extern struct MERR	vmerr;

extern CP					CpFirstTap();
extern struct AAB	vaab;

#ifdef MAC
extern struct RULSS	vrulss;
#endif

#ifdef SUBDRAW
extern int	fSubdraw;
#endif

/*
Here is a list of the various uac values and what info is stored in uab.

uab.ca
	this range is replaced by: docScrap if uacScrap, docUndo if uacUndo
	if uacScrap or uacCopy, undo => scrap.
	if uacMove, range is copied to caMove.doc & caMove.cpFirst and then
	it is replaced by docUndo.

uab.caMove
	if fRedo uacMove, this is the source, ca is the destination.
	if !fRedo uacMove, caFirst is used as above.

fRedo: uacCopy and uacDel are redone by selecting the operand and calling
CmdCopy or CmdCut.
uacMove and uacUndo are redone by explicit code.

*/


/* C M D  U N D O */
/* %%Function:CmdUndo %%Owner:davidlu */
CMD CmdUndo(pcmb)
CMB *pcmb;
{
	struct DOD *pdod;
	BOOL f = fFalse;
	int ww = wwNil;  /* to avoid unit var when jumping to LRet bz */
	int fSaveHiddenUndoSed;
	BOOL fOutlineDirty;
	struct SELS *psels;
	CP dcp;
	CMD cmd = cmdOK;
	struct CA caUndo, caT, caT2, caDel;

#ifdef MAC
	if (FSystemEdit(0)) return cmdCancelled;
#endif /* MAC */

#ifdef SUBDRAW
	if (fSubdraw)
		{
		SubdrawCmdUndo();
		return cmdOK;
		}
#endif /* SUBDRAW */

	if (vuab.uac == uacNil)
		{
		Beep();
		return cmdError;
		}

#ifdef MAC
	if (vrulss.caRulerSprm.doc != docNil)
		FlushRulerSprms();
#endif

	TurnOffSel(&selCur);
#ifdef MAC
	if (vaab.acc == accDiadic || vaab.acc == accInsert)
		InvalAgain();
#endif

#ifdef WIN
	/* dyadic move/copy saves the ca that text was moved to for repeat. Undo
		causes that ca to be invalid, so kill repeat in that case. Insert/
		typing is similar.
	*/
	TestInvalAgain();
#endif

LRestart:
	PcaSetWholeDoc(&caUndo, docUndo);

	if (!vuab.fRedo)
		{
		switch (vuab.uac)
			{
		case uacScrap:
			PcaSetWholeDoc(&caUndo, docScrap);
LRepl:                  
			FReplaceCps(&vuab.ca, &caUndo);
			if (vuab.selsBefore.fWithinCell)
				ApplyTableProps(PcaSetDcp(&caT, vuab.ca.doc, vuab.ca.cpFirst, DcpCa(&caUndo)), fTrue);
			if (FInTableDocCp(vuab.ca.doc, vuab.ca.cpFirst))
				/* Need to do some invalidations in this case */
				{
				caTap.doc = docNil;
				CpFirstTap(vuab.ca.doc, vuab.ca.cpFirst);
				InvalTableCp(vuab.ca.doc,caTap.cpFirst,DcpCa(&caUndo));
				}
			break;
		case uacMove:
/* adjust destination point, given a replace at vuab.ca by caUndo (undo) */
LUacMove:		
			Assert(vuab.cpFrom != cpNil);
			caUndo.cpLim = vuab.cpFrom;
			AdjustCaReplace(&vuab.caMove, &vuab.ca, &caUndo,fFalse);
			if (f)
				{
				vuab.uac = uacMoveNil;
				}
			goto LUacUndo;
		case uacMoveNil:
LUacMoveNil:		
			vuab.uac = uacMove;
			f = fTrue;
			goto LUndoNil;
		case uacUndoNil:
/* clear undo buffer, otherwise same as uacUndo. We could have cleared
undo in SetUndo*, but this saves a tiny bit of time. Redo is done as for
uacUndo. */
			vuab.uac = uacUndo;
LUndoNil:		
			PcaSetWholeDoc(&caT, docUndo);
			if (vuab.cpFrom != cpNil)
				{
				Assert(vuab.uac == uacMove ||
						vuab.uac == uacMoveNil);
				caT.cpLim = vuab.cpFrom;
				vuab.cpFrom = cp0;
				}
			if (!FDelete(&caT))
				goto LRetNoMem;
			goto LRestart;
		case uacUndoInval:
LUacUndoInval:          
			InvalDoc(vuab.ca.doc);
		case uacUndo:
/* exchange undo and vuab.ca */
LUacUndo:
/* this is probably not worth fixing but worth noting: because we are placing
	both versions of the text in the same doc, if the version in docUndo is 
	formatted and vuab.ca.doc is unformatted, after the second FReplaceCps call
	both versions will be formatted. Also true of fMayHavePic status. */
			fSaveHiddenUndoSed = (PdodDoc(docUndo)->fSedMacEopCopied &&
					vuab.ca.cpLim >= CpTail(vuab.ca.doc) -
					((!PdodDoc(vuab.ca.doc)->fGuarded) ? ccpEop : cp0) &&
					vuab.ca.cpFirst < vuab.ca.cpLim);
			if (!FReplaceCps(PcaPoint(&caT, vuab.ca.doc, vuab.ca.cpFirst),
					&caUndo))
				goto LRetNoMem;
			if (fSaveHiddenUndoSed && !FReplaceCps(PcaSetWholeDoc(&caT,docScratch),
					PcaSetDcp(&caT2, docUndo, CpMac1Doc(docUndo) - ccpEop, ccpEop)))
				goto LRetNoMem;
			caDel = vuab.ca;
			caDel.cpFirst += (dcp = DcpCa(&caUndo));
			caDel.cpLim += dcp;
			if (vuab.uac == uacMoveNil || vuab.uac == uacMove)
				{
				FReplaceCps(&caUndo, &caDel);
				vuab.cpFrom = DcpCa(&caDel);
				vuab.uac = uacMove;
				vuab.fDelete = fTrue;
				if (FInTableDocCp(vuab.caMove.doc, vuab.caMove.cpFirst))
					/* Need to do some invalidations in this case */
					{
					caTap.doc = docNil;
					CpFirstTap(vuab.caMove.doc, vuab.caMove.cpFirst);
					InvalTableCp(vuab.caMove.doc,caTap.cpFirst,CpMax(DcpCa(&vuab.caMove),DcpCa(&caTap)));
					}
				}
			else
				{
				SetWholeDoc(docUndo, &caDel);
				}
			if (vmerr.fMemFail || !FDelete(&caDel))
				goto LRetNoMem;
			if (fSaveHiddenUndoSed && !FReplaceCps(PcaSetDcp(&caT, vuab.ca.doc, CpMac1Doc(vuab.ca.doc) - ccpEop,
					 ccpEop),
					PcaSetDcp(&caT2, docScratch, cp0, ccpEop)))
				goto LRetNoMem;

			vuab.ca.cpLim = vuab.ca.cpFirst + DcpCa(&caUndo);
			if (FInTableDocCp(vuab.ca.doc, vuab.ca.cpFirst))
				/* Need to do some invalidations in this case */
				{
				caTap.doc = docNil;
				CpFirstTap(vuab.ca.doc, vuab.ca.cpFirst);
				InvalTableCp(vuab.ca.doc,caTap.cpFirst,DcpCa(&caUndo));
				}

			if (vuab.fOutlineInval)
				{
				/* special outline inval needed */
				caT = vuab.ca;
				ExpandOutlineCa(&caT, fTrue);
				InvalCp(&caT);
				}
			}

		switch (vuab.uac)
			{
		case uacCopy:
		case uacScrap:
/* copy undo to scrap, clear undo */
			PcaSetWholeDoc(&caUndo, docUndo);
			if (vuab.fExt)
/* scrap used to be in external form before being clobbered, so instead
of restoring it, we just re-establish its externalness. Zeroing is
done to free up space. */
				PcaSetNil(&caUndo);
			SetWholeDoc(docScrap, &caUndo);
			if (vmerr.fMemFail)
				goto LRetNoMem;
			Win( ChangeClipboard() );
			SetWholeDoc(docUndo, PcaSetNil(&caUndo));
			vsab.sabt = vuab.sabt;
			break;
		case uacMoveNil:
		case uacMove:
/* move undo document to caMove.cpFirst, and set up caMove for Redo */
			PcaSetWholeDoc(&caUndo, docUndo);
			caUndo.cpFirst = vuab.cpFrom;
			AdjustCaReplace(&vuab.ca, &vuab.caMove, &caUndo, fTrue);
			PcaPoint(&caT, vuab.caMove.doc, vuab.caMove.cpFirst);
			if (!FReplaceCps(&caT, &caUndo))
				goto LRetNoMem;
			if (FInTableDocCp(caT.doc, caT.cpFirst))
				{
				caTap.doc = docNil;
				CpFirstTap(caT.doc, caT.cpFirst);
				InvalTableCp(caT.doc,caTap.cpFirst,DcpCa(&caTap));
				}
			caDel = vuab.caMove;
			caDel.cpFirst += (dcp = DcpCa(&caUndo));
			caDel.cpLim += dcp;
			if (!FReplaceCps(&caUndo, &caDel) || !FDelete(&caDel))
				goto LRetNoMem;
			vuab.caMove.cpLim = vuab.caMove.cpFirst+DcpCa(&caUndo);
			}
		}
	else
		{
/* redo actions */
		Assert (vuab.wwBefore != wwNil); /* should be caught by now bz */
		NewCurWw(vuab.wwBefore, fFalse);
		switch (vuab.uac)
			{
#ifdef DEBUG
		default:/* case uacNil: */
			Assert(fFalse);
#endif
		case uacCopy:
			SetSelCurSels(&vuab.selsBefore);
			cmd = WinMac(CmdCopy(NULL), CmdExecUcm(ucmCopy, hNil));
			vuab.fRedo = fFalse;
			goto LRet;

		case uacScrap:
			SetSelCurSels(&vuab.selsBefore);
			cmd = WinMac(CmdDelCut(NULL), CmdExecUcm(ucmCut, hNil));
			vuab.fRedo = fFalse;
			goto LRet;

		case uacUndoInval:
			goto LUacUndoInval;
		case uacUndo:
			goto LUacUndo;
		case uacMoveNil:
			goto LUacMoveNil;
		case uacMove:
			goto LUacMove;
			}
		}

	vuab.fRedo = !vuab.fRedo;

/* previous/after dirty/formatted state of a document was saved in vuab.
Swap that state */
	pdod = PdodDoc(vuab.ca.doc);
	fOutlineDirty = pdod->fOutlineDirty;
	f = pdod->grpfDirty;	/* includes fFormatted */
	pdod->grpfDirty = vuab.grpfDirty;
	pdod->fOutlineDirty = fOutlineDirty;
	vuab.grpfDirty = f;

#ifdef WIN
	if (hwwdCur != hNil && (*hwwdCur)->wk == wkMacro)
		pdod->grpfDirty |= dimDoc;
#endif

	if (vuab.ca.doc == docScrap)
		{
		f = vsab.fMayHavePic;
		vsab.fMayHavePic = vuab.fMayHavePic;
		}
	else
		{
		f = PdodMother(vuab.ca.doc)->fMayHavePic;
		PdodMother(vuab.ca.doc)->fMayHavePic = vuab.fMayHavePic;
		}
	vuab.fMayHavePic = f;

	if (vmerr.fMemFail)
		{
LRetNoMem:
		SetUndoNil();
		return cmdNoMemory;
		}

/* header display docs are dirtied by undo/redo so the change will be 
   recorded; we must do this BEFORE NewCurWw or UpdateWw can kill hdr */
	pdod = PdodDoc(vuab.ca.doc);
	if (pdod->dk == dkDispHdr)
		{
		pdod->fDirty = fTrue;
		pdod->fHdrSame = fFalse;
		}
	if (vuab.uac == uacMove || vuab.uac == uacMoveNil && vuab.caMove.doc != docNil)
		{
		pdod = PdodDoc(vuab.caMove.doc);
		if (pdod->dk == dkDispHdr)
			{
			pdod->fDirty = fTrue;
			pdod->fHdrSame = fFalse;
			}
		}

/* reset to original ww and selection */
	if (vuab.fRedo)
		{ /* after undo */
		ww = vuab.wwBefore;
		psels = &vuab.selsBefore;
		}
	else
		{
		ww = vuab.wwAfter;
		psels = &vuab.selsAfter;
		}
	if (mpwwhwwd[ww] == hNil || !FDocInWw(psels->doc, ww))
		ww = wwNil;

#ifdef MAC
	if (ww != wwNil && (WwFrontWindow() != wwChange || vuab.ucm != ucmChange))
#else
	if (ww != wwNil)
#endif
		{
		Mac(SelectWindow(PwwdWw(ww)->wwptr));
		NewCurWw(ww, fFalse);
		/* it's possible for header docs to have gone away */
		if (mpdochdod[psels->doc] != hNil)
			SetSelCurSels(psels);
		Win(MakeSelCurVisi(fFalse/*fForceBlockToIp*/));
		GetSelCurChp(fTrue);
		}
	vfSeeSel = fTrue;
#ifdef MAC
	NotifyModeless();
#endif
#ifndef JR
	if (vuab.fOutlineDirty)
		DirtyOutline(vuab.ca.doc);
#endif /* JR */

LRet:
	if (vuab.docInvalPageView != docNil)
		InvalPageView(vuab.docInvalPageView);
	InvalCaFierce();
#ifdef WIN
	if (ww != wwNil)
		{
/* moved from above because it depends on InvalPageView to cause ruler 
to be updated, especially if ruler hold on to non exist idr. 
Yoshi put this call in to fix a bug, not sure why it is needed when vfSeeSel is 
already set.
*/
		Win( NormCp(ww, selCur.doc, selCur.cpFirst, ncpHoriz, 0,
				selCur.fInsEnd); )
		}
#endif
	return cmd;
}


/* A D J U S T  C A  R E P L A C E */
/* adjust a ca given a replacement. Used to adjust private ca's which
the general adjustment mechanism does not know about */
/* %%Function:AdjustCaReplace %%Owner:davidlu */
AdjustCaReplace(pcaAdjust, pcaDelete, pcaInsert, fGE)
struct CA *pcaAdjust, *pcaDelete, *pcaInsert;
int fGE;
{
	if (pcaAdjust->doc == pcaDelete->doc)
		{
		if (pcaAdjust->cpFirst > pcaDelete->cpFirst ||
				(fGE && pcaAdjust->cpFirst == pcaDelete->cpFirst))
			{
			CP dcp = DcpCa(pcaInsert) - DcpCa(pcaDelete);
			pcaAdjust->cpFirst += dcp;
			pcaAdjust->cpLim += dcp;
			Assert(pcaAdjust->cpFirst >= pcaDelete->cpFirst);
			}
		}
}


/* F  D O C  I N  W W */
/* Returns true if the given document is displayed in the window */
/* %%Function:FDocInWw %%Owner:davidlu */
FDocInWw(doc, ww)
int doc, ww;
{
	struct DR *pdr;
	int idr;
	struct WWD *pwwd;

	pwwd = PwwdWw(ww);
	for (pdr = pwwd->rgdr, idr = pwwd->idrMac; idr-- > 0; ++pdr)
		if (pdr->doc == doc)
			return(fTrue);
	return(fFalse);
}


