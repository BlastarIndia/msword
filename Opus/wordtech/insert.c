
#ifdef MAC
#define EVENTS
#include "toolbox.h" /* ObscureCursor, SystemTask, GetNextEvent */
#endif

#define RSHDEFS
#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "doc.h"
#include "disp.h"
#include "props.h"
#include "fkp.h"
#include "file.h"
#include "format.h"
#include "sel.h"
#include "ch.h"
#include "cmd.h"
#include "inter.h"
#include "prm.h"
#include "field.h"
#include "screen.h"
#include "border.h"

#ifdef MAC
#include "mac.h"
#else /* WIN */
#include "winkeys.h"
#include "idle.h"
#include "keys.h"
#include "status.h"
#include "profile.h"
#include "cmdtbl.h"
#include "recorder.h"
#include "help.h"
#endif
#include "layout.h"
#include "insert.h"
#include "debug.h"

/* G L O B A L S */
struct FKPD     vfkpdChp;
struct FKPDP    vfkpdPap;
struct FKPDT    vfkpdText;
struct SELS     rgselsInsert[iselsInsertMax];   /* prev insertion point queue */
BOOL            vfInsertMode = fFalse;
BOOL            vfAbortInsert;              /* signals exit insert mode */
CP              cpFirstNotInval;
CP              cpLimNotInval;
CP              cpInsert = cpNil; /* Beginning cp of insert block */
int             vidrInsert;


/* E X T E R N A L S */
#ifdef WIN
extern IDF              vidf;
extern struct LCB       vlcb;
extern FC               vfcSpec;
extern MSG              vmsgLast;
extern struct SCI       vsci;
extern BOOL             vfHelp;
extern HWND             vhwndCBT;
extern struct RAC       vrac;
extern int		docSeqCache;
extern CP		cpSeqCache;
extern int		vdocFetchVisi;
extern CP		vcpFetchVisi;
extern CP		vcpFetchFirstVisi;
#endif /* WIN */

#ifdef MAC
extern BOOL             vftcSave;
extern struct EVENT     event;
extern struct LLC       vllc;
extern struct AAB       vaab;
#endif /* MAC */

extern struct WWD       **hwwdCur;
extern int              wwCur;
extern struct FLS       vfls;
extern struct FMTSS     vfmtss;
extern struct UAB       vuab;
extern struct DOD       **mpdochdod[];
extern struct FCB       **mpfnhfcb[];
extern struct CA        caPage;
extern struct CA        caPara;
extern struct CA        caSect;
extern struct CA        caTap;
extern char             rgchInsert[];
extern int              ichInsert; /* Number of chars used in rgchInsert */
extern struct FKPD      vfkpdChp;
extern struct FKPD      vfkpdPap;
extern struct FKPDT     vfkpdText;
extern struct FLI       vfli;
extern struct SEL       selCur;
extern struct PAP       vpapFetch;
extern struct CHP       vchpFetch;
extern struct TCC		vtcc;
extern CP               vcpFetch;
extern int              vccpFetch;
extern char HUGE        *vhpchFetch;
extern int		vdocFetch;
extern int              vstcpMapStc;
extern int              vfSeeSel;
extern int              vfLastCursor;
extern struct MERR      vmerr;
extern struct CHP       vchpStc;
extern struct PREF      vpref;
extern struct ITR       vitr;
extern BOOL		vfAsynchronousUndo;
extern BOOL		vfInsertMode;
extern BOOL		vfRecording;
extern BOOL		vfRecordNext;
extern int		vflm;
extern int		vfFocus;
extern char		rgchEop[];
extern int		vifldInsert;
extern int		vfOvertypeMode;
extern CP		cpFirstNotInval;
extern CP		cpLimNotInval;
extern CP		cpInsert; /* Beginning cp of insert block */
extern int		vidrInsert;
extern CP 		vcpFirstTablePara;


/* B E G I N  I N S E R T */
/* open up rgchInsert array.  if successful, vfInsertMode is set. */
/* %%Function:BeginInsert %%Owner:bryanl */
BeginInsert(ptlv)
TLV *ptlv;
{
	int ww;
	int dl;
	struct PLDR **hpldr;
	int idr;
	struct CA caT;
	struct CHP chp;

#ifdef WIN
	if (vfRecording && vrac.racop != racopInsert)
		FlushVrac();
#endif /* WIN */

	vfInsertMode = fTrue;
	cpInsert = selCur.cpFirst;
	Assert(!selCur.chp.fSpec);
	Assert(!selCur.chp.fRMark);
	Assert(!selCur.chp.fStrike);
	chp = selCur.chp;
	Win( chp.fRMark = PdodMother(selCur.doc)->dop.fRevMarking );

#ifdef WIN
	if (PdodDoc (selCur.doc)->hplcfld != hNil)
		/*	what field instructions, if any, are we inserting to */
		vifldInsert = IfldInsertDocCp (selCur.doc, selCur.cpFirst);
#endif /* WIN */

/* Insert the speeder-upper rgchIns insert block. */
	if (!FNewChpIns(selCur.doc, cpInsert, &chp, stcNil) ||
			!FReplace(PcaPoint(&caT, selCur.doc, cpInsert), 
			fnInsert, fc0, (FC) cchInsertMax))
		{
		vfInsertMode = fFalse;
		cpInsert = cpNil;
		Win( vifldInsert = ifldNil );
		return;
		}
	AdjustEdlDcp(selCur.doc, cpInsert, (CP)cchInsertMax, cp0);

	Scribble(ispInsertMode,'I');

	InvalFli();
	Win( selCur.fUpdateChp = fFalse );  /* compensate for FReplace */
	Assert(!selCur.fUpdateChp);
	ichInsert = 0;
/* insertion cursor will be kept at the end of the rgchInsert block */
	selCur.cpLim = selCur.cpAnchor = (selCur.cpFirst += cchInsertMax);
	vidrInsert = IdrFromHpldrDocCp(hwwdCur, wwCur, selCur.doc, 
			cpInsert, fFalse, fFalse);
/* note: vidrInsert may be nil, and then its effect is as if dl were nil */
	ptlv->ichFill = 0;
/* lim case is possible eg. when appending to suppressed outline text */
#ifdef MAC
	ptlv->fInsEnd = selCur.cpLim < CpMacDoc(selCur.doc) &&
			ChFetch(selCur.doc, selCur.cpLim, fcmChars) == chSect;
#else
		{
		char ch;
		ptlv->fInsEnd = selCur.cpLim < CpMacDoc(selCur.doc) &&
				((ch = ChFetch(selCur.doc, selCur.cpLim, fcmChars)) == chSect
				|| ch == chColumnBreak);
		}
#endif
}


/* E N D  I N S E R T */
/* empties out rgchInsert to scratch file, removes the fnInsert piece
from the document. Used to end insertion (fEnd) or just to empty rgchInsert
when it is full (!fEnd) */
/* %%Function:EndInsert %%Owner:bryanl */
EndInsert(fEnd,ptlv)
BOOL fEnd;
TLV *ptlv;
{
	struct CA caT;
	FC fc;
	CP cpLim;
	CP dcp;
	BOOL fSkipInval = fFalse;
#ifdef WIN
	int cchInsert;
#else /* MAC */
#define cchInsert cchInsertMax
#endif

#ifdef RSH
	if (ichInsert > 0)
		LogUa(uaInsert);
#endif /* RSH */

	fc = FcAppendRgchToFn(fnScratch, rgchInsert, ichInsert);
	if (!vfInsertMode)
		return; /* nothing to do */

	Win( cchInsert = ptlv->fInsertLoop ? cchInsertMax : 0; )
#ifdef WIN
			if (selCur.chp.fBold || selCur.chp.fItalic)
		{
		FReplace(PcaSetDcp(&caT, selCur.doc, cpInsert,
				(CP) (fEnd ? cchInsert : 0)),
				fnScratch, fc, (FC) ichInsert);
		if (fEnd)
			{
			cpInsert = cpNil;
			vfInsertMode = fFalse;
			Win ( vifldInsert = ifldNil ) ;
			}
		else
			{
			cpInsert += ichInsert;
			selCur.cpAnchor = selCur.cpLim = selCur.cpFirst = cpInsert + cchInsert;
			}
		fSkipInval = fTrue;
		if (ptlv->fAddRun)
			goto LAddRun;
		goto LSkipInval;
		}

	/* Adjust rgwSeqCache */
	if (selCur.doc == docSeqCache)
		AdjustSingleCp(&cpSeqCache, cpInsert, ichInsert);

	/* Adjust FetchCpPccpVisible cache */
	if (selCur.doc == vdocFetchVisi)
		{
		AdjustSingleCp(&vcpFetchVisi, cpInsert, ichInsert);
		AdjustSingleCp(&vcpFetchFirstVisi, cpInsert, ichInsert);
		}

	/* Adjust vcaCell cache */
	/* Note: don't call AdjustCa because it doesn't exist in the
		fast version. */
	if (vcaCell.doc == selCur.doc)
		{
		AdjustSingleCp(&vcaCell.cpFirst, cpInsert, ichInsert);
		AdjustSingleCp(&vcaCell.cpLim, cpInsert, ichInsert);
		}
#endif

/* replace without invalidation */
	FRepl1(PcaSetDcp(&caT, selCur.doc, cpInsert,
			(CP) (fEnd ? cchInsert : 0)),
			fnScratch, fc, (FC) ichInsert);

	if (ptlv->fAddRun)
		{
LAddRun:
		if (!FAddRun(fnScratch, vfkpdText.fcLim, ptlv->ppapx, ptlv->cchPapx,
				&vfkpdPap, fTrue /* para run */, 
				fTrue /* alloc at fcMac */, fTrue /* hplcbtePap must expand */,
				fFalse /* not Word 3 format */))
			{
			return;
			}
		ptlv->fAddRun = fFalse;	/* AddRun completed sucessfully */
		if (fSkipInval)
			goto LSkipInval;
		}

	if (vpref.fBkgrndPag)
		DirtyPageInsert();

/* invalidate sequential fetch */
	vcpFetch = cpMax;

/* if the vanished run disappears, vfli will be invalid and pedl->dcp will
have to be updated */
	if (fEnd)
		{
		int ww = wwCur;

		caPara.doc = docNil;

		cpInsert += ichInsert;
		dcp = ichInsert - cchInsert;
		AdjustEdlDcp(selCur.doc, cpInsert, dcp, dcp);
		AdjustCp(PcaSetDcp(&caT, selCur.doc, cpInsert, (CP)(cchInsert - ichInsert)), cp0);
		InvalFli();
		cpInsert = cpNil;
		vfInsertMode = fFalse;
		Win ( vifldInsert = ifldNil ) ;
		Scribble(ispInsertMode, ' ');
		}
	else
		{
/* here the vfli and pedl are independently invalidated and recomputed */
		cpInsert += ichInsert;
		AdjustCp(PcaPoint(&caT, selCur.doc, cpInsert), (CP) ichInsert);
		AdjustEdlDcp(selCur.doc, cpInsert, (CP)ichInsert, cp0);
		selCur.cpAnchor = selCur.cpLim = selCur.cpFirst = cpInsert + cchInsert;
		}

#ifdef WIN
LSkipInval:
	if (vfRecording && ptlv->fInsertLoop)
		RecordInsert(ichInsert, ptlv);
	if (vfRecordNext && (ptlv->fRecordBksp || fEnd))
		{
		EndRecordNextCmd();
		ptlv->fRecordBksp = fFalse;
		ptlv->fInvalAgain = fTrue;
		}
#endif /* WIN */

	ichInsert = 0;
	ptlv->ichFill = 0;
}


#ifdef WIN
/* %%Function:AdjustSingleCp %%Owner:bryanl */
AdjustSingleCp(pcp, cpInsert, ichInsert)
CP *pcp;
CP cpInsert;
int ichInsert;
{
	if (*pcp >= cpInsert)
		*pcp += ichInsert;
}
#endif /* WIN */

/* A D J U S T  E D L  D C P */
/* Description: Adjusts edl.dcp's for edits which don't require dirtying
/* the EDLs. The adjustment occurs at [doc,cp].
/*    dcpAdjust is the amount by which the (inner) edl.dcp should be adjusted.
/*    dcpPending is the size of a pending adjustment, which is used to correct
/*        cp's used to calculate the correct adjust for any outer EDLs
/**/
/* %%Function:AdjustEdlDcp %%Owner:tomsax */
AdjustEdlDcp(doc, cp, dcpAdjust, dcpPending)
int doc;
CP cp, dcpAdjust, dcpPending;
{
	int ww, dl, idr;
	CP cpLim;
	struct PLDR **hpldr;
	struct PLCEDL **hplcedl;
	struct EDL edl;
	struct DRF drfFetch;

	for (ww = WwDisp(doc, wwNil, fTrue); ww != wwNil; 
			ww = WwDisp(selCur.doc, ww, fTrue))
		{

		dl = DlWhereDocCp(ww, doc, cp, fFalse, &hpldr, &idr, NULL, NULL, 2/*kludge*/);
		if (dl == dlNil)
			continue;

		Assert(dl >= 0);
		hplcedl = PdrFetch(hpldr, idr, &drfFetch)->hplcedl;
		GetPlc(hplcedl, dl, &edl);
		edl.dcp += dcpAdjust;
		while (fTrue)
			{
			if (!PwwdWw(ww)->fOutline || CpPlc(hplcedl, dl) + edl.dcp > cp)
				PutPlcLast(hplcedl, dl, &edl);

			FreePdrf(&drfFetch);
			if ((*hpldr)->hpldrBack == hNil)
				break;

			if (dcpPending)
				{
				hplcedl = PdrFetch(hpldr, (*hpldr)->idrMac-1, &drfFetch)->hplcedl;
				cpLim = CpMacPlc(hplcedl) + dcpPending;
				FreePdrf(&drfFetch);
				}

			idr = (*hpldr)->idrBack;
			hpldr = (*hpldr)->hpldrBack;
			hplcedl = PdrFetch(hpldr, idr, &drfFetch)->hplcedl;
			dl = IInPlc(hplcedl, cp);

			GetPlc(hplcedl, dl, &edl);
			edl.dcp = dcpPending ? cpLim - CpPlc(hplcedl,dl) : edl.dcp + dcpAdjust;
			Assert(dl >= 0);
			}
		}
}

/* I N S E R T  L O O P  C H */
/* Alpha mode works by inserting a block of cchInsertMax cp's at the
insertion point. We AdjustCp for this block as though it contained
cchinsBlock cp's even though it is initially "empty".

When a character is typed, it is inserted at rgchInsert[ichInsert++].
When rgchInsert is full, it is written to the scratch file, and Replace'd
with a new insertion block.

Backspace is handled similarly.

Procedure exits when it encounters an event/message that it cannot handle. It
cleans up writing the half finished insertion block out and returns. Event is
left in global event for MAC or global vmsgLast for WIN.

Notes for undo:
Undo is based on a part of the document being saved in docUndo, and a
corresponding part designated for replacement at SetUndoAfter.
Both start at cpInsertLow.
docUndo will contain:
	1. blocks of characters read into rgchInsert when backspacing. These
	are put here by prepending to existing contents.
	2. initial deletion of selected stuff, not including Eop disallowed
	by FDeleteCheck.
	3. possibly a single Eop following the insertion which is modified
	by the Next Style code. This is appended to the existing contents.
	Note that only the first modification is significant.
The replacement part comprises the corresponding text in the apres-command
cp coordinate system, starting at cpInsertLow, and ending at selCur.cpFirst
+ cchEop*(# of kcmOpenInsert's) + (cchEop if the Eop mentioned in point 3
above was included in docUndo)

Plan:

InsertLoopCh(ch)

1. delete selection, if any
2. BeginInsert()
This sets up cpInsert.
We will maintain the following invariances:
cpInsert: cp of rgchInsert[0]. Hence current ins point is cpInsert+ichInsert.
cpInsertLow: the smallest cp we fetched into rgchInsert and saved in the
undo document in preparation for backspace.

at any point we will have to update other windows from cpFirstNotInval to
cpLimNotInval.

3. for all chars to be handled in ch do
		if com-shift (look) shifted: (mac only)
			EndInsert() - closes up rgchInsert.
			KeyCmdLook(ch) - applies props to selCur.chp
			BeginInsert() - opens up rgchInsert, calls FNewChpIns.
			goto LIdle.
		switch on ch
		backspace:
			if prev char is in rgchInsert
				remove it from there
			else
				if at cp0, cant'backspace: beep, get next evt.
				if at a para bound and para props change.
					cant'bs
				if after the first char of footnote text
					cant'bs
				we are going to go back:
					at least 1 char
					at most cchInsertMax
					at most after preceding para bound
					at most after preceding fnt ref.
					at most after 1st char of footnote txt
					at most to prev run bound
				we went back to cpNew/fetched rgchInsert.
				if this expands undo territory
					remember text fetched for undo
				Delete text that was fetched. This is since
				a copy of the text (except its last character)
				is sitting in rgchInsert, and rgchInsert itself
				is still in the doc, and since another copy of
				the original is in the undo doc. This delete takes
				care of releasing referenced footnotes, sections, etc.
				Set chp's corresponding to the last run.
			update low point
			goto LInvalWw;
		non-breaking space:
			update ch
			goto LInsertCh
		non-required hyphen:
			update ch, etc.
		new line:
			update ch, etc.
		page break:
			update ch, etc.
		end of paragraph:
			if (rgchInsert is full) EndInsert(just empty).
			store chEop in rgchInsert.
			CachePara(cpInsert..)
			compute papx from vpapFetch (see FInsertRgch)
			jam stcNext into papx
			now we have desired papx, cchPapxc.
			EndInsert();
			BeginInsert();
			AddRun papx to scratch file/see FInsertRgch.
			goto LInvalWw.
		default:
4. now we insert printable character ch
LInsertCh:
		if rgchInsert is full
			EndInsert(just empty rgchInsert)
			this will adjust all cp's.
			in particular, selCur's cp's will be incremented.
		insert ch at next place in rgchInsert

5. now invalidate the window incrementally
LInvalWw:
		update cp*NotInval
		find dl of cpInsert+ichInsert.
		check previous line/fCpBad invalidation
		set window dirty
		window is now properly invalidated.
		if no event waiting
			if (fcpBad) UpdateWw(..)
			else
				find first invalid line.
				if not on screen
					get it on screen (and update)
				else if in last line
					scroll up 2 lines (and update)
				else
6. common case: do a mini /UpdateWw().
					format the line
					display the line
					clear line dirty.
					if next line on display exists
						and (invalid or not following
						in y space or cp space)
						UpdateWw()
					else
						clean dirty window.
		else
			goto LHaveEvent;
7. inserted character may have been displayed
LIdle:
		dally for a short period
		if no event waiting
			InvalOtherWws()
			Idle()
			if no event waiting
				SystemTask()
			loop until event arrives
8. get next event, loop if still in insert mode.
LNextMsg:
	get next event
	set ch
	if not key event break.
	if mem alert then break.
	if not one of the above ch's break.
	ie not:
		non-breaking space:
		non-required hyphen:
		new line:
		page break:
		end of paragraph:
end loop for all alpha mode ch's

9. wrap up after termination of alpha mode.
	EndInsert()
	InvalOtherWws()
	SetUndo(...)
	UpdateWw()
	end of InsertLoopCh



*/

/* %%Function:InsertLoopCh %%Owner:bryanl */
InsertLoopCh(ch, ucm)
int ch;
{
	int i;
	int cch;
	BOOL fReturn = fFalse;
	int fDirty, fRMBackSpace = fFalse;
	BOOL fSpecialEop;
	BOOL fCpEndOutline = fFalse;
	CP dcpRM, cpNew, cpEndOutline;
	struct CA caT;
	TLV tlv;
	struct CA caRM;
	struct SELS selsBefore;

#ifdef WIN
	/* Document is locked (annotations feature) */
	if (PdodDoc(selCur.doc)->fLockForEdit)
		{
		Beep();
		return;
		}


	/* If CBT is running, and wants to abort, don't enter insert loop */
	if (vhwndCBT)
		if (!SendMessage(vhwndCBT, WM_CBTSEMEV, smvBeginTyping, 0L))
			return;
#endif	/* WIN */

#ifdef INEFFICIENT
	tlv.fNextEop = fFalse;
	tlv.fFtnDel = fFalse;
	tlv.fReturn = fFalse;
	tlv.ccpEopOpen = 0;
#else
	SetWords(&tlv, 0, cwTLV);
#endif

	Win( tlv.fInsertLoop = fTrue; )
			tlv.fInvalPgdOnInsert = fTrue;

#ifdef WIN
	if (vfHelp)
		CancelContextHelp();
#endif

	if ((*hwwdCur)->fDirty)
		UpdateWw(wwCur, fFalse);
/* check for special insertion at suppressed text in outline mode */
	if ((*hwwdCur)->fOutline)
		{
		struct PLC **hplcpad;
		int ipad, ipadMac;
		struct PAD pad;

		if (FOutlineEmpty(wwCur, fTrue))
			goto LSignalReturn;

#ifdef WIN
		if (ch != chReturn)
#else
			if (ch != chEop || ucm == ucmOpenInsert)
#endif
				goto LEndOutline;

		hplcpad = PdodDoc(selCur.doc)->hplcpad;
		ipad = IInPlc(hplcpad, selCur.cpFirst);
		ipadMac = IpadMac(hplcpad);
		if (CpPlc(hplcpad, ipad + 1) == selCur.cpFirst + ccpEop && 
				ipad + 1 < ipadMac)
			{
			GetPlc(hplcpad, ipad + 1, &pad);
			if (!pad.fShow)
				{
LSkipInvis:			
				ipad++;
/* start first visible char following ipad | or cpMac */
				while (ipad < ipadMac)
					{
					GetPlc(hplcpad, ipad, &pad);
					if (pad.fShow)
						break;
					ipad++;
					}
/* ipad points to the char in front of which the Eop should go */
				TurnOffSel(&selCur);
				cpEndOutline = CpPlc(hplcpad, ipad);
				fCpEndOutline = fTrue;
				ucm = ucmOpenInsert;
				goto LEndOutline;
				}
			}
		GetPlc(hplcpad, ipad, &pad);
		if (pad.fBody && WinMac((*hwwdCur)->fEllip, !vpref.fOtlShowBody) )
			{
			FormatLine(wwCur, selCur.doc, 
					CpPlc(hplcpad, ipad));
			if (vfli.cpMac <= selCur.cpFirst && !pad.fInTable)
				goto LSkipInvis;
			}
LEndOutline:	
		;
		}


/* We will not delete if the EOP is selected and we are doing backspace
or fail FDeleteCheck.
*/
	if (!selCur.fIns &&
			ch == chBS /* back space */ &&
			(selCur.cpFirst >= CpMacDocEdit(selCur.doc) || (selCur.fBlock && !selCur.fTable)))
		{
		if (CmdExecUcm(ucmClear, hNil) == cmdError) Beep();
		goto LSignalReturn;
		}

	/* Save this because it gets adjusted in FDeleteCheck */
	selsBefore = *((struct SELS *)&selCur);
/* Always FDeleteCheck first no matter fAutoDelete or !fIns. */

	if (!FDeleteCheck(fFalse, rpkText, NULL))
		goto LSignalReturn;

/* Delete the selection, and make an insert point selection in its stead */
/* the following code prevents the deletion of a trailing Eop in common
situations, where this would not be desirable */
	if (!selCur.fIns &&
			ch != chBS /* back space */ &&
			ch != chCRJ /* new line */ &&
			ch != WinMac(chReturn, chEop) &&
#ifdef MAC
			ch != chEnter &&
#else
			ch != chColumnBreak && 
#endif
			ch != chSect /* new page */ )
		{
		CachePara(selCur.doc, selCur.cpLim - 1);
		if (caPara.cpLim == selCur.cpLim && !selCur.fTable && !vpapFetch.fTtp)
			{
			int cchT = cchEop;
			CHAR chFetch = ChFetch(selCur.doc, selCur.cpLim - 1, fcmChars);

/* FUTURE: (cc) there is a FOneCharEop routine we can use here */
			if (chFetch == chSect)
				cchT = cchSect;
#ifdef CRLF
			else if (chFetch != chEop || selCur.cpLim < ccpEop || 
				ChFetch(selCur.doc, selCur.cpLim - ccpEop, fcmChars) != chReturn)
				cchT = 1;
#endif
			Select(&selCur, selCur.cpFirst, selCur.cpLim - cchT);
			}
		if (selCur.sty == styWord)
			{
			CP cpT;

			cpT = CpLimNoSpaces(&selCur.ca);
			if (cpT != selCur.cpLim)
				Select(&selCur, selCur.cpFirst, cpT);
			}
		}

	if (!FSetUndoBefore(ucmTyping, uccInsert))
		{
LSignalReturn:
/* return after getting the next event (can't return directly because
the convention is that an event has to be present when returning)
*/
		fReturn = fTrue;
		goto LNextMsg;
		}
	vuab.selsBefore = selsBefore;
	if (fCpEndOutline)
		SelectIns(&selCur, cpEndOutline);

	dcpRM = 0;
	if (!selCur.fIns)
		{
		if (CmdAutoDelete(&caRM) != cmdOK)
			goto LSignalReturn;
		/* take care of dead fields & tables */
		AssureNestedPropsCorrect(&caRM, fFalse);
		dcpRM = DcpCa(&caRM);
		if (ch == chBS)
			{
			/* selection was deleted */
			SetUndoAfter(PcaSetDcp(&caT, selCur.doc, selCur.cpFirst, dcpRM));
			InvalAgain();
			goto LSignalReturn;
			}
		}
	Win( GetSelCurChp(fFalse) );


#ifdef WIN
/* BEGIN QUICK-AND-DIRTY OVERTYPE */
	if (vfOvertypeMode)
		{
		FC fcT;
		CP cpDelTo;

		InvalAgain();
		/* Some characters do not overtype... */
		switch (ch)
			{
		case chSect:
		case chBS:
		case chColumnBreak:
		case chCRJ:
		case chReturn:
			goto LNoOT;
			}

		TurnOffSel(&selCur);
		FNewChpIns(selCur.doc, selCur.cpFirst, &selCur.chp, stcNil);
		fcT = FcAppendRgchToFn(fnScratch, &ch, 1);
		cpDelTo = CpLimSty(selCur.ww, selCur.doc,  
				selCur.cpFirst, styChar, fFalse );
		CachePara( selCur.doc, selCur.cpFirst );
		if (cpDelTo >= caPara.cpLim)
			goto LNoOT;

		SetUndoNil();   /* only do this if really overtyping */

		if (!FLegalSel (selCur.doc, selCur.cpFirst, cpDelTo))
			Beep();
		else
			{
			PdodDoc(selCur.doc)->fDirty = fTrue;
			FReplace(PcaSet(&caT, selCur.doc, 
					selCur.cpFirst, cpDelTo),
					fnScratch, fcT, (FC) 1 );
			SelectIns(&selCur, selCur.cpFirst + 1);
			UpdateWw( wwCur, fFalse /* fAbortOk*/);
			if (vfRecording)
				FRecInsRgch (&ch, 1);
			}
		fReturn = fTrue;

		goto LNextMsg;
LNoOT:
		PdodDoc(selCur.doc)->fDirty = fTrue;
		}
/* END QUICK-AND-DIRTY OVERTYPE */
#endif /* WIN */

	BeginInsert(&tlv);

	if (!vfInsertMode)
		return fFalse;

/* initialize cp* */
	cpFirstNotInval = tlv.cpStart = cpLimNotInval = tlv.cpLow = 
			tlv.cpInsertLow = cpInsert;

	for (;;)
		{
		/* tlv.fNoIdle = fFalse; - ignored */
		tlv.cpInsertLine = selCur.cpFirst;

LInsertCh:

#ifdef RSH
		SetUatMode(uamTyping);
		StopUatTimer();
#endif /* RSH */

		switch (ch)
			{
			BSV bsv;

		case chBS:
			if ((bsv = BsvBackSpace(&tlv)) == bsvError)
				goto LNextMsg;

			if (bsv == bsvRevMark)
				{
				vuab.selsBefore = selsBefore;
				fRMBackSpace = fTrue;
				goto LEndInsert;
				}
			if (tlv.fInsEnd)
				{
				/*tlv.ch = ch*/
				InvalInsert(&tlv);
				goto LSplatExit;
				}
			break;

		case WinMac(chReturn, chEop):
#ifdef MAC
		case chEnter:
#endif /* MAC */
			tlv.ucm = ucm;
			tlv.ch = ch;
/* this is to make sure that Eop+Splat does not have vanished run in
between in pageview because Format can't handle that. should be really fixed
in format. */
			fSpecialEop = fFalse;
			if ((*hwwdCur)->fPageView && !PdodDoc(selCur.doc)->fShort)
				{
				CacheSectSedMac(selCur.doc, cpInsert);
				if (caSect.cpLim == selCur.cpFirst + 1)
					fSpecialEop = fTrue;
				}
			InsertEopEtc(&tlv);
			if (fSpecialEop)
				{
LSplatExit:			
				EndInsert(fTrue, &tlv);
				Mac( FNextMsgForInsert(&tlv) );
				goto LEndInsert;
				}
			break;
		case chSect:
			/*InvalPageView(selCur.doc);*/
			/* FALL THROUGH */
		/*WIN?case chColumnBreak:*/
			/* Some characters can't be used in a plain doc... */
			PdodDoc(selCur.doc)->fFormatted = fTrue;
			/* FALL THROUGH */
		default:
			if ((uns) ch <= 0xff)
				{

#ifdef MAC
				/* The next test is to determine if we need
				/* to substitute a "smart" quote (either open or closing)
				/* for a dumb quote (generic) quote. The condition is somewhat
				/* redundant so that the more common cases test out quickly.
				/**/
				if (ch<=chSDumbQuote && vpref.fSmartQuotes
						&& (ch==chDDumbQuote || ch==chSDumbQuote) )
					{
					int chPrev, fOpenQuote;

					if (ichInsert > 0 )
						chPrev = rgchInsert[ichInsert-1];
					else  if (cpInsert == cp0)
						{
						fOpenQuote = fTrue;
						goto LSetQuote;
						}
					else
						{
						FetchCp(selCur.doc,cpInsert-1,fcmChars);
						chPrev = vhpchFetch[0];
						}

					/* Fast and dirty test for white space, not the most
					/* thorough, but is good enough for this purpose.
					/**/
					fOpenQuote = fFalse;
					switch (chPrev)
						{
					default:
						fOpenQuote = chPrev <= chSpace;
						break;
					case chDOpenQuote:
					case chSOpenQuote:
					case chSDumbQuote:
					case chDDumbQuote:
					case '(':
					case '[':
					case '{':
					case '=':
					case '>':
					case '<':
						fOpenQuote = fTrue;
						break;
						}
LSetQuote:
					ch = (ch == chDDumbQuote)
							? (fOpenQuote?chDOpenQuote:chDCloseQuote)
: 
							(fOpenQuote?chSOpenQuote:chSCloseQuote);
					}
#endif /* MAC */
				if (ichInsert == cchInsertMax)
					EndInsert(fFalse, &tlv);
				rgchInsert[ichInsert++] = ch;
				}
			else
				Beep();
			} /* switch (ch) */

LInvalWw:
/* invalidate the current window */
		tlv.ch = ch;
		InvalInsert(&tlv);
/* ran out of memory; exit insert */
		if (vmerr.fMemFail || !vfInsertMode)
			goto LEndInsert;

LNextMsg:
/* wait till next message is received. */

		tlv.fReturn = fReturn;
		if (!FNextMsgForInsert(&tlv))
			{
			if (fReturn)
				return;
			break;
			}
		ch = tlv.ch;
		ucm = ucmNil;
		}

LEndInsert:
	if (vidrInsert != idrNil)
		{
		struct DRF drfFetch;

		if (cpInsert < PdrFetchAndFree(hwwdCur, vidrInsert, &drfFetch)->cpFirst)
			ClearSccs();
		}
	EndInsert(fTrue, &tlv);

#ifdef WIN
	if (fRMBackSpace)
		{
		selCur.cpFirst--;
		selCur.cpLim--;
		InvalCp(PcaSetDcp(&caT, selCur.doc, selCur.cpFirst, (CP) 1));
		dcpRM += 1;
		tlv.cpInsertLow = selCur.cpFirst;
		}
#endif
/* fix up fInsEnd after Eop's, before splats. fInsEnd was in an incorrect state 
because of vanished runs, selection was drawn as if fInsEnd were off - since
the selection was not thought to be on line boundaries - how lucky. Now
that the vanished run is gone, we need to reflect the current display state 
back in the incorrect selCur setting. */
	if (tlv.fInsEnd && selCur.cpFirst != cp0)
		{
		int ch = ChFetch(selCur.doc, selCur.cpFirst - 1, fcmChars);
		if (ch == chEop || ch == chCRJ || ch == chSect 
#ifdef WIN
				|| ch  == chColumnBreak
#endif
				)
			selCur.fInsEnd = fFalse;
		}

/* remember insertion point in buffer */
	PushInsSelCur();
	vfLastCursor = fFalse; /* since we are implicitly selecting */

	InvalOtherWws(fTrue);

	SetUndoAfter(PcaSet(&caT, selCur.doc, tlv.cpInsertLow,
			selCur.cpFirst + dcpRM + (tlv.ccpEopOpen + (tlv.fNextEop * cchEop))));
	Mac( SetAgainUcmAccAsc(ucmTyping, accInsert, ascEdit); )
#ifdef WIN
	if (tlv.fInvalAgain)
		{
		InvalAgain();
		tlv.fInvalAgain = fFalse;
		}
	else
		{
		SetAgain(bcmTyping);	/* Handled in SetUndoAfter() for Mac */
		SetAgainCp(selCur.doc, tlv.cpStart, tlv.cpLow, caT.cpLim - dcpRM);
		}
#endif
}


/* I N V A L  O T H E R  W W S */
/* %%Function:InvalOtherWws %%Owner:bryanl */
InvalOtherWws(fEnd)
BOOL fEnd;
{
	int ww = WwDisp(selCur.doc, wwNil, fTrue);
	struct CA caT;
/* call InvalCp if there are more than one ww's showing doc */
	if (ww != wwNil && WwDisp(selCur.doc, ww, fTrue) != wwNil && cpInsert != cpNil)
		{
		CP dcp;
		if ((dcp = cpLimNotInval - cpFirstNotInval) != cp0 || fEnd)
			InvalCp(PcaSetDcp(&caT,selCur.doc,cpFirstNotInval,dcp));
		cpFirstNotInval = cpLimNotInval = cpInsert + ichInsert;
		}
}


/* C P  B A C K S P A C E  D O C  C P  D C P */
/* Given a document, a location, and a number of character, returns
	the last character we can "legally" backspace over */
/* %%Function:CpBackspaceDocCpDcp %%Owner:bryanl */
CP CpBackspaceDocCpDcp(doc, cpStart, dcp)
int doc;
CP cpStart, dcp;
{
	struct DOD **hdod;
	CP cpMin;
	CP cp;
	int ccpBS;
	uns papT[cwPAPBase];

	hdod = mpdochdod[doc];

	for ( cpMin = CpMax( cp0, (cp = cpStart) - dcp); cp > cpMin ; cp -= ccpBS )
		{
		ccpBS = 1;

		CachePara(doc, cp-1);
		if (caPara.cpLim == cp)
			{
		/* Now check if paragraph properties have changed */
			blt(&vpapFetch, &papT, cwPAPBase);
			CachePara(doc, cp);
			((struct PAP *)papT)->phe = vpapFetch.phe;
			if (FNeRgw(&vpapFetch, &papT, cwPAPBase))
				break;	/* disallow backspace over EOL if properties differ */

#ifdef CRLF	
			FetchCp(selCur.doc, cp - 1, fcmChars);
			if ((*vhpchFetch == chEol || *vhpchFetch == chTable) && cp > (CP)1)
				{
				FetchCp(selCur.doc, cp - 2, fcmChars);
				if (*vhpchFetch == chReturn)
					ccpBS++;
				}
#endif

		/* Don't allow end of table marks to be deleted */
			if (FInTableDocCp(doc, cp-1))
				{
				extern CP vcpFirstTablePara;
				CacheTc(wwNil, doc, vcpFirstTablePara, fFalse, fFalse);
				if (vtcc.cpLim == cp)
					break;
				}
			}
		if (((*hdod)->fFtn || (*hdod)->fAtn) && FNoBackSpaceRef(hdod, cp))
			break;
		}

	return(cp);
}


/* I N S E R T  P O S T  U P D A T E */
/* actions which need to be performed after an UpdateWw */
/* %%Function:InsertPostUpdate %%Owner:bryanl */
InsertPostUpdate()
{
	int idr = IdrFromHpldrDocCp(hwwdCur, wwCur, selCur.doc,
			cpInsert + ichInsert, fFalse, fFalse);
#ifdef WIN
	if (idr != vidrInsert)
		selCur.fUpdateRuler = fTrue;
#endif /* WIN */
	vidrInsert = idr;
	if (vidrInsert != idrNil)
		{
		struct DRF drfFetch;

		if (cpInsert < PdrFetchAndFree(hwwdCur, vidrInsert, &drfFetch)->cpFirst)
			ClearSccs();
		}

}


#ifdef WIN
/*  LimitCpBkmk
	Limits pcpRet to be no less than the greatest bookmark end (cpFirst
	or cpLim) which is <= cp.  Called from insert code for a BS.
*/

LimitCpBkmk (pcpRet, cp)
CP *pcpRet, cp;

{
	struct PLC **hplcbkf = PdodDoc (selCur.doc)->hplcbkf;
	struct PLC **hplcbkl = PdodDoc (selCur.doc)->hplcbkl;
	int i, iMac = IMacPlc(hplcbkf);

	Assert (PdodDoc (selCur.doc)->hplcbkf != hNil);

	if (CpPlc(hplcbkf, 0) > cp ||
			CpPlc( hplcbkl, iMac-1) <= *pcpRet)
		{
		return;
		}

	i = IInPlcMult (hplcbkf, cp);
	Assert (CpPlc( hplcbkf, i ) <= cp);
	if (CpPlc( hplcbkf, i) > *pcpRet)
		*pcpRet = CpPlc( hplcbkf, i);

	if (CpPlc( hplcbkl,0 ) > cp)
		return;

	i = IInPlcMult (hplcbkl, cp);
	Assert (CpPlc( hplcbkl, i) <= cp);
	if (CpPlc( hplcbkl, i) > *pcpRet)
		*pcpRet = CpPlc( hplcbkl, i);
}


#endif


/* I N V A L  I N S E R T */
/* Invalidate appropriate things while in insert mode */
/* %%Function:InvalInsert %%Owner:bryanl */
InvalInsert(ptlv)
TLV * ptlv;
{
	int dl;
	CP cpInsNew,cpMinInval;
	int idrInner, idrT;
	struct PLDR **hpldrInner;
	struct DR *pdrT;
	struct PLCEDL ** hplcedl;
	int fNewPage, cMemErr;
#ifdef WIN
	int dlBack;
	BOOL fWasRMarked;
#endif
	struct CA caT;
	struct EDL edl, edlT;
	struct DRF drfFetch;

	InvalFli();
#ifdef MAC
	ObscureCursor();
#else
	SetCursor( (HANDLE) NULL );
	vlcb.ca.doc = docNil;
#endif
	cpMinInval = cpInsNew = cpInsert + ichInsert;
	if (ptlv->ch != chBS)
		cpMinInval--;
	cpFirstNotInval = CpMin(cpFirstNotInval, cpInsNew);
	cpLimNotInval = CpMax(cpLimNotInval, cpInsNew);

#ifdef WIN	/*  must ensure that any field we are inserting in
	is invalidated properly. */
			if (vifldInsert != ifldNil)
		{
		struct PLC **hplcfld = PdodDoc(selCur.doc)->hplcfld;
		struct FLD fld;

		GetPlc( hplcfld, vifldInsert, &fld );
		fld.fDirty = fTrue;
		PutPlcLast( hplcfld, vifldInsert, &fld );

		if ((uns)(fld.flt-fltSeqLevOut)<=(fltSeqLevNum-fltSeqLevOut))
			/* invalidate auto numbered fields */
			vidf.fInvalSeqLev = PdodDoc(selCur.doc)->fInvalSeqLev = fTrue;
		}

	/* Must invalidate active dde links */
	InvalDde (PcaSet(&caT, selCur.doc, cpInsert, selCur.cpLim));
#endif	/* WIN */

/* must invalidate the PLCPGD, PLCPAD, PLCPHE immediately if there is a
	possibility that they were validated in Idle since the last insertion.
	Otherwise invalidation for this insertion will not take place.
	Invalidate from ichInsert-1 to the end of the insert piece, so we
	only have to do this once between calls to FIdle. */

	if (ptlv->fInvalPgdOnInsert)
		{
		struct DOD *pdod = PdodDoc(selCur.doc);
		if (!pdod->fShort)
			{
			CP cpInvalMin = cpInsert + max(ichInsert-1,0);
			CP cpInvalMac = cpInsert + cchInsertMax;

			if (pdod->hplcpgd != hNil)
				SetPlcUnk(pdod->hplcpgd, cpInvalMin, cpInvalMac);
			if (pdod->hplcpad != hNil)
				SetPlcUnk(pdod->hplcpad, cpInvalMin, cpInvalMac);
			if (pdod->hplcphe != hNil)
				SetPlcUnk(pdod->hplcphe, cpInvalMin, cpInvalMac);
			caPage.doc = docNil;
			}

		ptlv->fInvalPgdOnInsert = fFalse;
		}
	cMemErr = 0;
LGetInner:
	hpldrInner = hwwdCur;
	if ((idrInner = vidrInsert) == idrNil)
		{
		InsertPostUpdate();
		idrInner = vidrInsert;
		}
LGetDl:
	if (idrInner == idrNil)
		goto LDrfFree;
	if ((hplcedl = (pdrT = PdrFetch(hpldrInner,
			idrInner, &drfFetch))->hplcedl) == hNil)
		{
		/* probably page view: hplcedl's not allocated due
			to page recalc */
		FreePdrf(&drfFetch);
		if (++cMemErr >= 2) return; /* out of memory */
		if ((*hwwdCur)->fDrDirty)
			{
			EndInsert(fTrue, ptlv);
			UpdateWw(wwCur, fFalse);
			BeginInsert(ptlv);
			}
		else
			UpdateWw(wwCur, fFalse);
		goto LGetInner;
		}
	dl = IInPlcCheck(hplcedl, cpInsNew);
	if (dl >= 0)
		{
		CP cpFirst;
		int docT, cchT, chT, ich, ichMin;
		char HUGE *hpchT;
		struct EDL edl;

		pdrT->fDirty = fTrue;
		GetPlc(hplcedl, dl, &edl);
		if (edl.hpldr != hNil)
			{
			if (edl.fDirty)
				goto LUpdateScroll;
/* table: enter into embedded dr */
			edl.fTableDirty = fTrue;
			PutPlc(hplcedl, dl, &edl);
			FreePdrf(&drfFetch);
			hpldrInner = edl.hpldr;
			idrInner = IdrFromHpldrDocCp(hpldrInner, wwCur, selCur.doc, cpInsNew, fTrue, fFalse);
#ifdef WIN
			dlBack = dl;
#endif
			goto LGetDl;
			}
		edl.fDirty = fTrue;
		PutPlc(hplcedl, dl, &edl);

/* check for auto hyphenation mode */
		cpFirst = CpPlc(hplcedl, dl);
#ifdef MAC
#ifdef AUTOHYPH
		if (/* auto mode && */ !FUpper(ptlv->ch) && !FLower(ptlv->ch))
			{
/* check if there are any non letters in [cpFirst, cpInsert+ichInsert) */
			ichMin = max(0, (int)(cpFirst - cpInsert));
			for (ich = ichInsert - 2; ich >= ichMin; ich--)
				{
				chT = rgchInsert[ich];
				if (!FUpper(chT) && !FLower(chT))
					goto LNoHyphen;
				}
			docT = selCur.doc;
			cpT = cpFirst;
			CachePara(docT, cpT);
			while (cpT < cpInsert)
				{
				FetchCp(docT, cpT, fcmChars + fcmProps);
				docT = docNil;
				cpT += vccpFetch;
				if (vchpFetch.fSysVanish ||
						(vchpFetch.fVanish && !vpref.fSeeHidden)) continue;
				hpchT = vhpchFetch;
				cchT = vccpFetch;
				while (cchT-- > 0)
					{
					chT = *hpchT++;
					if (!FUpper(chT) && !FLower(chT))
						goto LNoHyphen;
					}
				}
/* solid letters in the interval in question, there is a dependency due to
hyphenation */
			if (dl == 0)
				PdrFreeAndFetch(hpldrInner, idrInner, &drfFetch)->fCpBad = fTrue;
			else
				DirtyEdl(hplcedl, dl - 1);
			}
		else
#endif	/* AUTOHYPH */
#endif	/* MAC */
				{
LNoHyphen:
				if (dl == 0)
/* check for fCpBad invalidation */
					{
					if (pdrT->cpFirst + pdrT->dcpDepend > cpMinInval)
						{
						pdrT->fCpBad = fTrue;
						if ((*hwwdCur)->fPageView)
							{
/* invalidate the last few lines of the predecessor dr */
							if (vidrInsert != idrNil &&
									(idrT = IdrPrevFlow(hwwdCur, vidrInsert)) !=
									idrNil)
								{
								pdrT = PdrFreeAndFetch(hwwdCur, idrT, &drfFetch);
								pdrT->fDirty = fTrue;
								DirtyEdl(pdrT->hplcedl, IMacPlc(pdrT->hplcedl) - 1);
								}
							}
						}
					}
				else
/* check previous line invalidation */
					{
					GetPlc(hplcedl,dl - 1, &edl);
					if (cpFirst + edl.dcpDepend > cpMinInval)
						DirtyEdl(hplcedl,dl-1);
					}
				}
		}
	else
		{
		FreePdrf(&drfFetch);
		InvalCp(PcaSet(&caT, selCur.doc, cpMinInval, cpMinInval + 1));
		UpdateWw(wwCur, fFalse);
		InsertPostUpdate();
		return;
		}
	FreePdrf(&drfFetch);
LDrfFree:
	(*hwwdCur)->fDirty = fTrue;
#ifdef MAC
	(*hwwdCur)->fFullSizeUpd = fTrue;

	if (vftcSave != ftcNil)
		{
		/* restore from temporary font change */
		if (selCur.chp.ftc == ftcSymbol)
			{
			EndInsert(fTrue, ptlv);
			selCur.chp.ftc = vftcSave;
			BeginInsert(ptlv);
			}
		vftcSave = ftcNil;
		}
#endif /* MAC */

	if (FMsgPresent( mtyTyping ))	/* HEAP MOVES */
		return; /* goto LNextMsg; */

	if (idrInner == idrNil)
		goto LNormCp;
	if ((pdrT = PdrFetchAndFree(hpldrInner, idrInner, &drfFetch))->fCpBad)
		{
		if (pdrT->fNoPrev)
			{
			/* We will repaginate */
			EndInsert(fTrue, ptlv);
			UpdateWw(wwCur, fFalse);
			BeginInsert(ptlv);
			}
		else
			UpdateWw(wwCur, fFalse);
		InsertPostUpdate();
		}
	else
		{
		if (dl < 0)
			{
			struct WWD *pwwdCur;
LNormCp:
			/* If we are going to a new page we need to 
				End insert and begin it again after we have
				Updated the window. */
			fNewPage = fFalse;
			if ((*hwwdCur)->fPageView)
				{
				if (IdrFromHpldrDocCp(hwwdCur, wwCur, selCur.doc, 
						cpInsNew, fFalse, fFalse) == idrNil)
					{
					fNewPage = fTrue;
/* fEnd is true since sel is at end of insert piece */
					EndInsert(fTrue, ptlv);
					/* have to force header dirty to get it saved
						back to real header doc */
					if (PdodDoc(selCur.doc)->dk == dkDispHdr)
						DirtyDoc(selCur.doc);
					}
				}
			if (selCur.fInsEnd != ptlv->fInsEnd)
				{
				ClearInsertLine(&selCur);
				selCur.fInsEnd = ptlv->fInsEnd;
				}
			pwwdCur = *hwwdCur;
/* FormatLine leaves wrong cpMac for chTerm=-1 lines if vanished
run is at the end of the line. Hence fFalse. */
			NormCp(wwCur, selCur.doc, cpInsNew, 0,
					min((pwwdCur->ywMac - pwwdCur->ywMin) / 2,
					pwwdCur->ywMin + 30), fFalse /*fEnd*/);
			if (fNewPage)
				BeginInsert(ptlv);
			InsertPostUpdate();
			}
		else
			{
			int ypTop, dypLineOld;
			int dypAbove;
			CP cpNextLine;

/* find first invalid dl scanning backwards starting with dl which is
	invalid for sure */
			pdrT = PdrFetch(hpldrInner, idrInner, &drfFetch);
			GetPlc(hplcedl = pdrT->hplcedl, dl, &edl);
			while (dl > 0)
				{
				GetPlc(hplcedl, dl - 1, &edlT);
				if (!edlT.fDirty)
					break;
				edl = edlT;
				if (edl.hpldr != hNil) /* can't cope with tables here */
					goto LUpdateWw;
				dl--;
				}
			dypLineOld = edl.dyp;
			ypTop = edl.ypTop;
			if (YwFromYp(hpldrInner, idrInner, edl.ypTop + edl.dyp) > (*hwwdCur)->ywMac)
				{
				ypTop = edl.ypTop;
				goto LUpdateScroll;
				}
			/* bind page number */
			if ((*hwwdCur)->fPageView)
				{
				int ipgd;
				struct PLCPGD **hplcpgd;
				struct PGD pgd;

				pgd.pgn = 1;	/* punt, just in case */
				if ((hplcpgd = PdodDoc(DocBaseWw(wwCur))->hplcpgd) != hNil
						&& (ipgd = (*hwwdCur)->ipgd) != ipgdNil)
					{
					Assert(0 <= (*hwwdCur)->ipgd && (*hwwdCur)->ipgd < IMacPlc(hplcpgd));
					GetPlc(hplcpgd, ipgd, &pgd);
					}
				vfmtss.pgn = pgd.pgn;
				}
			PwwdSetWw(wwCur, cptDoc);
			FormatLineDr(wwCur, CpPlc(hplcedl, dl), pdrT);
/* take care of line height changes here */
			if ((dl == 0 && pdrT->dypAbove >= vfli.dypLine) ||
					vfli.dypLine != dypLineOld)
				{
LUpdateWw:
				FreePdrf(&drfFetch);
				UpdateWw(wwCur, fFalse);
				InsertPostUpdate();
				return; /* goto LNextMsg; */
				}

			if (selCur.fInsEnd != ptlv->fInsEnd)
				{
				ClearInsertLine(&selCur);
				selCur.fInsEnd = ptlv->fInsEnd;
				}
			cpNextLine = CpPlc(hplcedl, dl + 1);
/* the following DisplayFli is going to overwrite the insertion cursor. if detects
cases where backspace occured at the beginning of a line, recomputing prev line 
but leaving ins point where it was. */
			if (!(selCur.cpFirst == cpNextLine && !selCur.fInsEnd))
				selCur.fOn = fFalse;
#ifdef WIN
			GetPlc(hplcedl, dl, &edl);
			fWasRMarked = edl.fRMark;
#endif

			DisplayFli(wwCur, hpldrInner, idrInner, dl, ypTop + vfli.dypLine);
#ifdef WIN
				{
				int xwLimScroll;

				GetPlc(hplcedl, dl, &edl);
				xwLimScroll = XwFromXp( hpldrInner, idrInner,
						edl.xpLeft + edl.dxp );
				pdrT->xwLimScroll = max( pdrT->xwLimScroll,
						xwLimScroll );
				(*hwwdCur)->xwLimScroll = max(
						xwLimScroll, (*hwwdCur)->xwLimScroll );
				}
#endif
			if (vfli.fSplatBreak && ((*hwwdCur)->fPageView || (*hpldrInner)->hpldrBack != hNil))
				DlkFromVfli(hplcedl, dl);

#ifdef WIN
			if ((fWasRMarked ^ vfli.fRMark) && (*hpldrInner)->hpldrBack != hNil)
				{
				int idrT, dlT;
				BOOL fRMark;
				struct DR *pdrT;
				struct DRF drfT;
				struct EDL edlT;

				if ((fRMark = vfli.fRMark) != 0)
					{
					pdrT = PdrFetch(hpldrInner, idrInner, &drfT);
					pdrT->fRMark = fTrue;
					FreePdrf(&drfT);

					DrawTableRevBar(wwCur, (*hpldrInner)->idrBack, dlBack);
					}
				else
					{
					for (idrT = (*hpldrInner)->idrMac;
							idrT-- != 0 && (!fRMark || idrT >= idrInner); )
						{
						pdrT = PdrFetch(hpldrInner, idrT, &drfT);
						if (idrT == idrInner)
							{
							pdrT->fRMark = fFalse;
							for (dlT = IMacPlc(pdrT->hplcedl); dlT--; )
								{
								GetPlc(pdrT->hplcedl, dlT, &edlT);
								if (edlT.fRMark)
									{
									pdrT->fRMark = fTrue;
									break;
									}
								}
							}
						fRMark |= pdrT->fRMark;
						FreePdrf(&drfT);
						}
					if (!fRMark)
						{
#ifdef	DEBUG
						pdrT = PdrFetch((*hpldrInner)->hpldrBack, (*hpldrInner)->idrBack, &drfT);
						GetPlc(pdrT->hplcedl, dlBack, &edl);
						Assert(edl.fTableDirty);
						FreePdrf(&drfT);
#endif /* DEBUG */
						goto LUpdateWw;
						}
					}
				}
#endif

/* next line is invalid if it exists (<dlMac) and alread invalid or
not following in cp space (yp space already checked above) */
			GetPlc(hplcedl, dl++, &edl);
			edl.fDirty = fFalse;
			PutPlcLast(hplcedl, dl - 1, &edl);

/* next line is invalid if it exists (<dlMac) and
not following in cp space or not following in yp space */
			if (cpNextLine != vfli.cpMac ||
					(dl < IMacPlc(hplcedl) &&
					((GetPlc(hplcedl, dl, &edl), edl.fDirty) ||
					edl.ypTop != ypTop + vfli.dypLine)) ||
					ptlv->fFtnDel)
				{
LUpdateScroll:
				ptlv->fFtnDel = fFalse;
				vfls.ca.doc = docNil;      /* for tables, force new heights */
				FreePdrf(&drfFetch);
				UpdateWw(wwCur, fFalse);
				InsertPostUpdate();
				if (!(*hwwdCur)->fPageView)
					{
					struct DR *pdr;
					hplcedl = (pdr = PdrFetch(hpldrInner, idrInner, &drfFetch))->hplcedl;
					if ((dl = IInPlcCheck(hplcedl, cpInsNew)) >= 0)
						{
						int dyw;
						struct RC rcw;
						struct PT ptT;
						struct PLDR **hpldrT;
						struct SPT spt;
						int dlT, idrT;
/* we want to detect if dl is past the bottom boundary of the DR. if so we
   want to use the last visible dl in the DR for our norming. */
						DrclToRcw(hpldrInner, &pdr->drcl, &rcw);
/* need to free DRF momentarily to allow DlWherePt to call PdrGalley which objects
   to allocated DRFs */
						FreePdrf(&drfFetch);

						ptT.xw = rcw.xwLeft;
						ptT.yw = rcw.ywBottom - 1;

						if (dlT = DlWherePt(wwCur, &ptT, &hpldrT, &idrT, &spt, fTrue, fTrue))
							{
							if (dlT != dlNil && hpldrT == hpldrInner &&
									idrT == idrInner && dlT < dl)
								dl = dlT;
							}
						hplcedl = (pdr = PdrFetch(hpldrInner, idrInner, &drfFetch))->hplcedl;
						GetPlc(hplcedl, dl, &edl);
						FreePdrf(&drfFetch);
/* if the bottom bound of the line containing the insertion point is below
the bottom of the window, scroll the line so it's entirely visible. */
						if ((dyw = YwFromYp(hpldrInner, idrInner, edl.ypTop + edl.dyp) -
								(*hwwdCur)->ywMac) > 0)
							{
							int dywExtra;
							/* don't scroll too much, or NormCp will disagree */
							dywExtra = min(dysMacAveLineSci, DyOfRc(&(*hwwdCur)->rcwDisp)/4);
							ScrollUp(wwCur, 
									dyw, 
									dyw + dywExtra);
							UpdateWw(wwCur, fFalse);
							}
						InsertPostUpdate();
						}
					else
						FreePdrf(&drfFetch);
					}
				else
/* if the line containing the insertion point is no longer visible, branch
	so that NormCp is called to make it visible. */
					goto LNormCp;
				}
			else	
				FreePdrf(&drfFetch);
			(*hwwdCur)->fDirty = fFalse;
			Mac( (*hwwdCur)->fFullSizeUpd = fFalse );
			}
		}
}


/* I N S E R T  E O P  E T C */
/* %%Function:InsertEopEtc %%Owner:bryanl */
InsertEopEtc(ptlv)
TLV * ptlv;
{
	Win( int ichInsertSav = ichInsert; )
			int cchPapx;
	BOOL fResetChpIns, fDirtyBorders;
	CP cp;
	struct PLC **hplcfld;
	struct DOD *pdod;
	struct CA caPrl, caT;
	char rgbPrl[3];
	struct PAP papStd, papNew;
	char papx[cchPapxMax+1];

	selCur.fUpdateChp = fFalse; /* previously valid */
	fDirtyBorders = fFalse;	/* until proven otherwise */

	CachePara(selCur.doc, cpInsert);

	papNew = vpapFetch;

#ifdef WIN
	if (papNew.fInTable && !FInTableDocCp(selCur.doc, cpInsert))
		papNew.fInTable = fFalse;
#endif /* WIN */

/* compute papx from vpapFetch */
	MapStc(PdodDoc(selCur.doc), papNew.stc, 0, &papStd);
	cchPapx = CbGenPapxFromPap(&papx, &papNew, &papStd, fFalse);
	fResetChpIns = fFalse;

/* "next style" field is significant if next char is eop and its style 
	(i.e. the current style is not the same as the desired next style. In 
	that case, send sprmPStc to set the looks of that paragraph (now 
	current, but soon to be next) */

	if (caPara.cpLim == cpInsert + (cchInsertMax + ccpEop) &&
			!(*hwwdCur)->fOutline &&
			(rgbPrl[1] =
			((*(PdodMother(selCur.doc)->stsh.hplestcp))
			->dnstcp)[vstcpMapStc].stcNext) != papNew.stc &&
			/* (Opus does not have this feature) */
	WinMac(1, ptlv->ucm != ucmReturnSame))
		{
		fDirtyBorders = FBorderPap(&papNew);
		fResetChpIns = fTrue;

		rgbPrl[0] = sprmPStc;
		PcaSet(&caPrl, selCur.doc, caPara.cpLim - ccpEop, 
				caPara.cpLim);

/* save Eop in the undo buffer before it is changed */
		if (!ptlv->fNextEop)
			{
			FSetUndoDelAdd(&caPrl, fTrue /* fAppend */);
			ptlv->fNextEop = fTrue;
			}

		rgbPrl[2] = sprmCPlain;
		ApplyGrpprlCa(rgbPrl, 3, &caPrl);

		MapStc(PdodDoc(selCur.doc), rgbPrl[1], 0, &papStd);
		fDirtyBorders |= FBorderPap(&papStd);

		CachePara(selCur.doc, cpInsert);
		if (fDirtyBorders && caPara.cpLim < CpMacDoc(caPara.doc))
			InvalCp(PcaSet(&caT,caPara.doc,caPara.cpLim,caPara.cpLim+1));

		caPara.doc = docNil;
		vdocFetch = docNil;
		}

	Win( if (ptlv->fInsertLoop) )
		{
		if (ichInsert > cchInsertMax - ccpEop)
			EndInsert(fFalse, ptlv);
		}

	bltb(rgchEop, &rgchInsert[ichInsert], (int) ccpEop);
	ichInsert += (int)ccpEop;

	caPara.doc = docNil;
	vdocFetch = docNil;

	ptlv->fAddRun = fTrue;
	ptlv->ppapx = &papx;
	ptlv->cchPapx = cchPapx;

	EndInsert(fTrue, ptlv);
	if (ptlv->fAddRun) /* add run failed? */
		{
		ptlv->fAddRun = fFalse; /* nevermind */
		return;
		}

	if (fResetChpIns)
		GetSelCurChp(fTrue /* fGetInsPtProps*/);

	if (!FNewChpIns(selCur.doc, selCur.cpFirst, &selCur.chp, stcNil))
		return;

	if ((*hwwdCur)->fOutline)
		UpdateHplcpadSingle(selCur.doc, selCur.cpFirst);

/* Open insert creates a new chEop but leaves ins pt preceding it */
	if (ptlv->ucm == ucmOpenInsert)
		{
		selCur.cpFirst -= ccpEop;
		ptlv->cpLow = selCur.cpFirst;
		ptlv->ccpEopOpen += ccpEop;
		GetSelCurChp(fTrue /* fGetInsPtProps*/);
		}
#ifdef MAC
	BeginInsert(ptlv);
#else /* WIN */
	if (vfRecording)
		RecordInsPara(ichInsertSav, ptlv);

	Assert(selCur.cpFirst > cp0);
	if ((pdod = PdodDoc(selCur.doc))->fMother && !pdod->fHasNoSeqLev
			&& (hplcfld = pdod->hplcfld) != hNil)
		/* check for invalidation of autonum fields in para after */
		{
		int ifld;
		struct FLCD flcd;

		CachePara(selCur.doc, selCur.cpFirst);
		ifld = IfldNextField(selCur.doc, caPara.cpFirst);
		if (ifld-- != ifldNil)
			{
			while ((ifld = IfldAfterFlcd(selCur.doc, ifld, &flcd)) != ifldNil
					&& flcd.cpFirst < caPara.cpLim)
				if ((uns)(flcd.flt-fltSeqLevOut)<=(fltSeqLevNum-fltSeqLevOut))
					{
					/* invalidate auto numbered fields */
					vidf.fInvalSeqLev = PdodDoc(selCur.doc)->fInvalSeqLev = fTrue;
					break;
					}
			}
		}

	if (ptlv->fInsertLoop)
		BeginInsert(ptlv);

#endif /* WIN */
}


/* B S V  B A C K  S P A C E */
/* Do one backspace.  BeginInsert must have been called before calling
this and EndInsert must be called after... */
BSV
/* %%Function:BsvBackSpace %%Owner:bryanl */
BsvBackSpace(ptlv)
TLV * ptlv;
{
/* CRLF support: ccpBS is set to ccpEop when we backspace over an
end-of-para.  We make simplification that we don't have to FetchCp
the extra characters in the EOP. (We do not assume all ccpEop
characters will be in the same run). */

	struct DOD ** hdod = mpdochdod[selCur.doc];
	CP dcp, cpNew;
	int cch;
	int ccpBS; /* # of cps to back over */
	struct CA caT;
	uns papT [cwPAPBase];
	CHAR chT;
	BOOL fInTable;
	CP cpFirstPara;

#ifdef RSH
	LogUa(uaBackspace);
#endif /* RSH */

	if (((*hdod)->fFtn || (*hdod)->fAtn)
		/* we are in a footnote/annotation document. Procedure returns true if cp
		is 1st of footnote/annotation text */
	&& FNoBackSpaceRef(hdod, cpInsert+ichInsert))
		goto LNoBackSpace;

	if (ichInsert > 0)
		{
		ichInsert -= 1;

#ifdef WIN /* Command recorder */
		if (ichInsert < ptlv->ichFill)
			{
			if (vfRecording)
				RecordBksp(ptlv);
			ptlv->ichFill = ichInsert;
			}
#endif
		ptlv->cpLow = CpMin(ptlv->cpLow, cpInsert + ichInsert);

		return bsvNormal;
		}

/* Can't delete characters before cp0!  Catch 'em trying here... */
	if (cpInsert == 0L)
		{
LNoBackSpace:
		Beep();
		return bsvError;
		}


/* Need to fetch a new batch into rgchInsert */
	ccpBS = 1; /* will be set to ccpEop if we're deleting an EOP */

	/* FInTableDocCp caches para */
	fInTable = FInTableDocCp(selCur.doc, cpInsert - 1);
	cpFirstPara = fInTable ? vcpFirstTablePara : caPara.cpFirst;

	dcp = CpMin(cpInsert, (CP)cchInsertMax);
	cpNew = CpMax(cpInsert - dcp, cpFirstPara);

/* Check if we're deleting a paragraph mark... */
	if (caPara.cpLim == cpInsert)
		{
/* we are trying to backspace over the end of the previous paragraph.
Check if that is the end of a table cell: if the prev paragraph is in a
table and if the ch at its end is a chTable. */
		if (fInTable &&
#ifdef MAC                        
			ChFetch(selCur.doc, cpInsert - ccpEop, fcmChars) == chTable)
#else /* WIN */
			/* be careful: we could be deleting a section break at cp0 */
			cpInsert >= ccpEop && 
			ChFetch(selCur.doc, cpInsert - ccpEop, fcmChars) == chReturn
					&& vhpchFetch[1] == chTable)
#endif
					goto LNoBackSpace;

#ifdef CRLF
		if (((chT = ChFetch(selCur.doc, cpInsert-1, fcmChars)) == chEol
				|| chT == chTable) && cpInsert > (CP)1)
			{
			FetchCp(selCur.doc, cpInsert - 2, fcmChars);
			if (*vhpchFetch == chReturn)
				ccpBS++;
			}
#endif
		if (cpFirstPara != cpInsert - ccpEop)
			{
			/* check if para props (other than tabs) have changed */
			blt(&vpapFetch, &papT, cwPAPBase);
			CachePara(selCur.doc, cpInsert);
			((struct PAP *)papT)->phe = vpapFetch.phe;
			if (FNeRgw(&vpapFetch, &papT, cwPAPBase))
				goto LNoBackSpace;
			}
		if (!fInTable && FInTableDocCp(selCur.doc, cpInsert))
			goto LNoBackSpace;
		}

/* limit buffer to current page to avoid invalidating cpFirst of page which
is expensive to recompute. the reason for fetching this buffer is efficiency
after all. */
	if ((*hwwdCur)->fPageView)
		{
		int ipgd = (*hwwdCur)->ipgd;
		CP cpFirstPage;
		struct DOD *pdod = *hdod;
		if (ipgd != ipgdNil && pdod->dk == dkDoc)
			{
			struct PLCPGD **hplcpgd = pdod->hplcpgd;
			if (hplcpgd != hNil)
				{
				cpFirstPage = CpPlc((*hdod)->hplcpgd, ipgd);
				if (cpFirstPage < cpInsert)
					cpNew = CpMax(cpFirstPage, cpNew);
				}
			}
		}

	if (!(*hdod)->fShort)
		{
/* limit cpNew to point after the previous footnote reference < than
cpInsert - 1 */
		if ((*hdod)->docFtn != docNil)
/* flag means footnote reference will be deleted. affects invalidation */
			{
			ptlv->fFtnDel = FLimitFtnAtnRef(&cpNew, cpInsert, edcDrpFtn);
			}

#ifdef WIN
		if ((*hdod)->docAtn != docNil)
/* flag means annotation reference will be deleted. affects invalidation */
			{
			ptlv->fFtnDel |= FLimitFtnAtnRef(&cpNew, cpInsert, edcDrpAtn);
			}
		if ((*hdod)->hsttbBkmk != hNil)
			/* limit cpNew to be no less than
				the greatest bookmark end <= cpInsert. */
			LimitCpBkmk (&cpNew, cpInsert);
#endif
		}

	if (cpNew >= cpInsert)
		cpNew -= ccpBS;

	Assert(cpNew < cpInsert);

/* fetch the characters >= cpNew and <= cpInsert-ccpBS so that these
characters form a contiguous run. */

	FetchCpAndPara(selCur.doc, cpNew, fcmChars + fcmProps);
	for (;;)
		{
		if (vcpFetch + vccpFetch > cpInsert - ccpBS)
			{
			cch = min(vccpFetch, (int)(cpInsert - vcpFetch));
#ifdef MAC
			if (vchpFetch.fSpec)
				{
				bltbh(vhpchFetch + cch - ccpBS, rgchInsert, ccpBS);
				cch = ccpBS;
				}
			else
#endif
				bltbh(vhpchFetch, rgchInsert, cch);
			break;
			}

		CachePara(selCur.doc, vcpFetch + vccpFetch);
		FetchCp(docNil, cpNil, fcmChars + fcmProps);
		}

#ifdef MAC
	if (!vpref.fSeeHidden)
#else		
		if (!((*hwwdCur)->grpfvisi.fSeeHidden ||
				(*hwwdCur)->grpfvisi.fvisiShowAll))
#endif

			if (vchpFetch.fVanish)
				goto LNoBackSpace;

#ifdef WIN	/* you cannot backspace over a fSpec char */
	if (vchpFetch.fSpec)
		goto LNoBackSpace;

	if (vfRecording)
		RecordBksp(ptlv);

	/* if not new text, kill insert loop */
	if (PdodMother(selCur.doc)->dop.fRevMarking && !vchpFetch.fRMark)
		{
		if (!vchpFetch.fStrike)
			{
			char grpprl[2];

			PcaSet(&caT, selCur.doc, cpInsert - 1, cpInsert);
			if (!FSetUndoB1(ucmTyping, uccFormat, &caT))
				goto LNoBackSpace;
			grpprl[0] = sprmCFStrikeRM;
			grpprl[1] = fTrue;
			ApplyGrpprlCa(grpprl, sizeof(grpprl), &caT);
			}

		return bsvRevMark;
		}
#endif /* WIN */

	Assert(cch > 0);
	ptlv->ichFill = ichInsert = cch - ccpBS; /* this does the backspace */
	cpNew = cpInsert - cch;

	if (cpNew < ptlv->cpInsertLow)
		{
		/* expand undo territory */
#ifdef WIN
		if (!FSetUndoDelAdd(PcaSet(&caT, selCur.doc,
				cpNew, ptlv->cpInsertLow), fFalse /* fAppend */))
			goto LNoBackSpace;
#else
		FSetUndoDelAdd(PcaSet(&caT, selCur.doc,
				cpNew, ptlv->cpInsertLow), fFalse /* fAppend */);
#endif
		ptlv->cpInsertLow = cpNew;
		}
	cpInsert = cpNew;


/* Set up new insert point properties */

/* because of restrictions on vanished runs and fSpec chars, we know that
vchpFetch is the chp we want--don't need to call GetSelCurChp() */

	selCur.chp = vchpFetch.fSpec ? vchpStc : vchpFetch;
	Win(Assert(!vchpFetch.fSpec));
	FNewChpIns(selCur.doc, cpInsert, &selCur.chp, stcNil);
	Win(selCur.chp.fRMark = fFalse);
	Win(selCur.chp.fStrike = fFalse);


/* Actualy do the delete */
	FDelete(PcaSetDcp(&caT, selCur.doc, cpNew, (CP)cch));

	/*  in case replace didn't invalidate, we want ribbon to update */
	selCur.fUpdateRibbon = fTrue;
	selCur.fUpdateChp = fFalse;

	ptlv->cpLow = CpMin(ptlv->cpLow, cpInsert + ichInsert);

#ifdef MAC
	if (vftcSave != ftcNil)
		{
		selCur.chp.ftc = vftcSave;
		vftcSave = ftcNil;
		}
#endif /* MAC */

	return bsvNormal;
}


/* D I R T Y   P A G E   I N S E R T */
/* %%Function:DirtyPageInsert %%Owner:bryanl */
DirtyPageInsert()
{
/* before ending insert or calling Idle, make sure the page just edited
	is marked dirty - for background repagination */
	struct DOD *pdod;
	struct PLC **hplcpgd;
	CP cp, CpRefFromCpSub();

	if (cpInsert == cpNil)
		return;
	pdod = PdodDoc(selCur.doc);
	Assert(!pdod->fHdr); /* docHdr is never displayed */
/* have to force header dirty to get it saved back to real header doc */
	if (pdod->dk == dkDispHdr)
		pdod->fDirty = fTrue;
	hplcpgd = pdod->hplcpgd;
	if (pdod->fShort)
		hplcpgd = PdodMother(selCur.doc)->hplcpgd;
	if (hplcpgd == hNil || pdod->fAtn) /* annotation text does not affect page layout */
		return;

/* identify earliest possible cp that is affected */
	if (!pdod->fShort)
		cp = cpInsert;
	else  if (pdod->fFtn)
		cp = CpRefFromCpSub(selCur.doc, cpInsert, edcDrpFtn);
	else  if (pdod->fDispHdr) /* docHdrDisp is what we displayed */
		{
		Assert(pdod->ihdt < ihdtMax);
		cp = pdod->ihdt >= ihdtMaxSep ? cp0 :
				CpPlc(PdodMother(selCur.doc)->hplcsed, pdod->ised);
		}
	SetPlcUnk(hplcpgd, cp, cp + 1);
}


#ifdef BOGUS
/* F  W H O L E  R O W  D I R T Y */
/* Is this whole row of a table dirty? */
/* %%Function:FWholeRowDirty %%Owner:NOTUSED */
FWholeRowDirty(hpldr)
struct PLDR **hpldr;
{
	int idrMac, dlMac;
	struct DR *pdrT;
	struct EDL edl;
	struct DRF drfFetch;

	pdrT = PdrFetch(hpldr, 0, &drfFetch);
	if (pdrT->hplcedl)
		{
		GetPlc(pdrT->hplcedl, 0, &edl);
		if (!edl.fDirty)
			goto LRet;
		}

	idrMac = IMacPlc(hpldr);
	pdrT = PdrFreeAndFetch(hpldr, idrMac - 1, &drfFetch);
	if (pdrT->hplcedl)
		{
		GetPlc(pdrT->hplcedl, IMacPlc(pdrT->hplcedl) - 1, &edl);
		if (!edl.fDirty)
			goto LRet;
		}

	edl.fDirty = fTrue;
LRet:
	FreePdrf(&drfFetch);
	return edl.fDirty;
}


#endif




/* F  I N  T T P */
/* %%Function:FInTtp %%Owner:bryanl */
FInTtp(doc, cp)
int doc;
CP cp;
{

	CachePara(doc, cp);
	return (vpapFetch.fTtp);

}


/* F  L I M I T  F T N  A T N  R E F */
/* used in insert (backspacing) */
/* if cpLim - 1 is a ref, assign it with cpNew and return true.
else if there is a ref < cpLim - 1, max it + 1 with cpNew.
return false;
*/
/* %%Function:FLimitFtnAtnRef %%Owner:bryanl */
FLimitFtnAtnRef(pcpNew, cpLim, edcDrp)
CP *pcpNew, cpLim;
int edcDrp;
{
	int iRef;
	struct PLC **hplcRef;
	struct DRP *pdrp;

	Mac(Assert ( edcDrp == edcDrpFtn ));

	if (cpLim > 0)
		{
		pdrp = ((int *)PdodDoc(selCur.doc)) + edcDrp;
		hplcRef = pdrp->hplcRef;
		if ((iRef = IInPlcCheck(hplcRef, cpLim - 1)) >= 0)
			{
			CP cpT = CpPlc(hplcRef, iRef);
			if (cpT == cpLim - 1)
				{
				*pcpNew = cpT;
				return fTrue;
				}
			*pcpNew = CpMax(*pcpNew, cpT + 1);
			}
		}
	return fFalse;
}


