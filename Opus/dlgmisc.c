/* ****************************************************************************
**
**      COPYRIGHT (C) 1986 MICROSOFT
**
** ****************************************************************************
*
*  Module: dlgmisc.c functions for Opus miscellaneous dialogs
*
*  Functions included:
*            Insert Break
*            Insert Bookmark
*            Go to
*	     Find Next
**
** REVISIONS
**
** Date         Who Rel Ver     Remarks
**
** ************************************************************************* */

#ifdef DEBUG
#ifdef PCJ
/* #define SEEGOTO */
#endif /* PCJ */
#endif /* DEBUG */


#define NONCMESSAGES
#define NOMENUS
#define NOSYSCOMMANDS
#define NOCOLOR
#define NOFONT
#define NOGDI
#define NOHDC
#define NOMENUS
#define NOREGION
#define NOSCROLL
#define NOTEXTMETRIC
#define NOWINOFFSETS

#define NOMINMAX
#define NOBRUSH
#define NOICON
#define NOPEN
#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOSYSMETRICS
#define NOCLIPBOARD
#define NOKEYSTATE
#define NORASTEROPS
#define NOSHOWINDOW
#define NOCREATESTRUCT
#define NODRAWTEXT
#define NOMEMMGR
#define NOMETAFILE
#define NOWH
#define NOWNDCLASS
#define NOREGION
#define NOSOUND
#define NOCOMM
#define NOKANJI

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "doc.h"
#include "props.h"
#include "sel.h"
#include "disp.h"
#include "debug.h"
#include "file.h"
#include "ch.h"
#include "wininfo.h"
#include "prm.h"
#include "keys.h"
#include "message.h"
#include "format.h"
#include "inter.h"
#include "field.h"
#include "print.h"
#define FINDNEXT
#include "search.h"
#include "error.h"
#include "core.h"
#include "prompt.h"

#include "idd.h"

#include "sdmdefs.h"
#include "sdmver.h"
/* returned by prompt if vkcLastCmdKey is pressed again */
#include "tmc.h"
#include "sdm.h"
#include "sdmtmpl.h"

#include "goto.hs"
#include "goto.sdm"
#include "insbreak.hs"
#include "insbreak.sdm"
#include "bookmark.hs"
#include "bookmark.sdm"
#include "insfile.hs"
#include "insfile.sdm"
#include "cust.hs"
#include "username.hs"
#include "username.sdm"

#include "cmd.h"
#include "doslib.h"
#include "layout.h"
#include "resource.h"


#ifdef PROTOTYPE
#include "dlgmisc.cpt"
#endif /* PROTOTYPE */

extern BOOL		vfRecording;
extern HWND             vhwndApp;
extern struct MERR      vmerr;
extern struct SEL       selCur;
extern CHAR             szEmpty[];
extern CHAR             szDoc[];
extern int vfSeeSel;
extern struct DOD       **mpdochdod[];
extern struct PAP       vpapFetch;
extern struct SEP       vsepFetch;
extern struct CA        caSect;
extern struct SPX       mpiwspxSep[];
extern struct UAB       vuab;
extern struct PREF      vpref;
extern struct FLI       vfli;
extern struct CA        caPara;
extern struct MWD       **hmwdCur;
extern struct PRSU      vprsu;
extern struct WWD       **hwwdCur;
extern struct CA        caPage;
extern int				vipgd;
extern int              wwCur;
extern int              vfFileCacheDirty;
extern CHAR             rgchEop[];
extern CHAR             rgchSect[];
extern struct CHP       vchpStc;
extern BOOL             vfRecorderOOM;

CP CpRefFromCpCftnatn ();
CP CpFromIpgd();


#ifdef DEBUG
extern BOOL             vfScanDlg;
#endif /* DEBUG */



/* I N S E R T   B R E A K  C H */
/*  %%Function:  InsertBreakCh  %%Owner:  bobz       */

InsertBreakCh(ch)
int ch;
{
	int     doc = selCur.doc;
	CP      cp;
	struct CA caRM;
	struct CHP chp;

	CheckCollapseBlock();

	if (!FSetUndoBefore(bcmInsBreak, uccPaste))
		return;

	if (CmdAutoDelete(&caRM) != cmdOK)
		return;

	cp = selCur.cpFirst;
	CachePara(doc, cp);
	chp = selCur.chp;
	chp.fRMark = PdodMother(doc)->dop.fRevMarking;
	if (!FInsertRgch(doc, cp, &ch, 1, &chp, NULL))
		return;
	PdodDoc(doc)->fFormatted = fTrue;
	SelectIns( &selCur, cp + 1 );
	PushInsSelCur();
	vfSeeSel = fTrue;
	caRM.cpLim += 1;
	SetUndoAfter(&caRM);
}


/* ****
*
	Function: CmdInsPageBreak
*  Author:
*  Copyright: Microsoft 1986
*  Date: 9/25/87
*
*  Description: Menu level command function for "Insert Page Break" 
** ***/
/*  %%Function:  CmdInsPageBreak  %%Owner:  bobz       */

CMD CmdInsPageBreak(pcmb)
CMB *pcmb;
{
	struct PAP pap;
	struct CHP chp;
	struct CA caT;

	if (FSelWithinTable(&selCur))
		{
		if (!FInTableDocCp(selCur.doc, selCur.cpFirst))
			{
			ModeError();
			return cmdError;
			}

		MapStc(PdodDoc(selCur.doc), stcNormal, 0, &pap);
		chp = vchpStc;
		chp.fRMark = PdodMother(selCur.doc)->dop.fRevMarking;
		if (!FSetUndoB1(imiInsPageBreak, uccInsert, 
				PcaPoint(&caT, selCur.doc, 
				CpFirstTap(selCur.doc, selCur.cpFirst))))
			{
			Beep();
			return(cmdCancelled);
			}

		TurnOffSel(&selCur);
		if (!FInsertRgch(selCur.doc, caT.cpFirst, rgchEop, cchEop, 
				&chp, &pap))
			return cmdNoMemory;
		caT.cpLim += ccpEop;
		if (!FInsertRgch(selCur.doc, caT.cpFirst, rgchSect, cchSect, 
				&chp, 0))
			return cmdNoMemory;
		caT.cpLim += ccpSect;
		SelectIns(&selCur, caT.cpFirst);
		InvalTableProps(selCur.doc, caT.cpFirst, ccpSect + ccpEop,
				fTrue /* fFierce */);
		InvalPageView(selCur.doc);
		SetUndoAfter(&caT);
		}
	else
		InsertBreakCh(chSect);

	return cmdOK;
}



/*  %%Function:  CmdInsColumnBreak  %%Owner:  bobz       */

CMD CmdInsColumnBreak(pcmb)
CMB *pcmb;
{
	struct PAP pap;
	struct CHP chp;
	struct CA caT;


	if (FSelWithinTable(&selCur))
		{
		/* Insert normal paragraph above current row */
		if (!FInTableDocCp(selCur.doc, selCur.cpFirst))
			{
			ModeError();
			return cmdError;
			}
		MapStc(PdodDoc(selCur.doc), stcNormal, 0, &pap);
		chp = vchpStc;
		chp.fRMark = PdodMother(selCur.doc)->dop.fRevMarking;
		if (!FSetUndoB1(bcmSplitTable, uccInsert, 
				PcaPoint(&caT, selCur.doc,
				CpFirstTap(selCur.doc, selCur.cpFirst))))
			{
			Beep();
			return(cmdCancelled);
			}
		TurnOffSel(&selCur);
		if (!FInsertRgch(selCur.doc, caT.cpFirst, rgchEop, cchEop, 
				&chp, &pap))
			return cmdNoMemory;
		InvalTableProps(selCur.doc, caT.cpFirst, ccpEop, 
				fTrue /* fFierce */);
		caT.cpLim += ccpEop;
		SelectIns(&selCur, caT.cpFirst);
		SetUndoAfter(&caT);
		SetAgain(bcmSplitTable);
		DirtyOutline(selCur.doc);
		}
	else
		InsertBreakCh(chColumnBreak);

	return cmdOK;
}







/** G O T O **/

/* forward declaration of functions */

FGotoSz ();
CP CpGotoPgcb();
CP CpFirstIsedGoto ();
CP CpFirstIpgdGoto ();
CP CpLineFromCpCl ();

/* goto structures & constants */

/*  These two structures are use to represent our goto syntax.  A goto
	string is evaluated as ont of (in order until success):
		a bookmark
		a percentage (i.e. 50%, %)
		an exception ("next" is "", "p" next page, "s" next sec etc.
		a general expression.

	a general expression is made up of one or more sub expressions.  Each
	subexpression has an (optional) group identifier ('p', 's', 'l', 'f'),
	an (optional) + or - to indicate relative movement, and an (optional)
	number.  A missing group identifier implies page.  A group identifier
	alone implies "current."  A + or - with no number means +1 or -1. A
	number (without + or -) means absolute.

	Groups take on the following levels:
		section
		page
		line, footnote and annotation
	More than one group from a single level may not be in an expression.
	Only the highest level may be relative.  Other levels modify the
	preceeding levels.  Thus s3f7 results in the seventh footnote
	reference in section three.  If page is NOT the highest level in an
	expression, the page refered to is a pgn (i.e. the page number as
	printed).  In all other cases the sec, page, line or footnote is
	absolute within the next higher object.  If an object is out of range,
	goto the first/last of it in the next higher specified object.
*/

struct GSD
	{  /* Goto Subexpression Descriptor */
	/* CS bug: must be even number of bytes */
	int fUsed : 8;
	int fRelative : 8;
	int wValue;
};

union GCB
	{  /* Goto Control Block */
	struct
		{
		struct GSD gsdSection;
		struct GSD gsdPage;
		struct GSD gsdLine;
		struct GSD gsdFtn;
		struct GSD gsdAtn;
		};
	struct GSD mpgggsd [5];
};


/*  Goto Groups */
#define ggNil       -1
#define ggSection   0
#define ggPage      1
#define ggLine      2
#define ggFtn       3
#define ggAtn       4
#define ggLast      ggAtn

	csconst CHAR mpggch [] = { 
	chGGSection, chGGPage, chGGLine, chGGFtn, chGGAtn 	};




/* C M D  G O T O */
/*  Command level for goto command.  If invoked by keyboard, use prompt
	line. On repeat key or from menu bring up dialog.  If using dialog and
	special button pressed, prepend goto character.  String stored in the
	CAB to make it known to Macro Recorder.
*/
/*  %%Function:  CmdGoto   %%Owner:  bobz       */

CMD CmdGoto (pcmb)
CMB * pcmb;
{
	CHAR sz [cchBkmkMax];

#ifdef DEBUG
	if (vfScanDlg)
		goto LDoDlg;
#endif /* DEBUG */

	Assert (PselActive() == &selCur);

	if (selCur.doc == docNil)
		return cmdError;

	if (selCur.doc != DocMother(selCur.doc))
		{
		CP cpRef;
		CP cpSub = selCur.cpFirst;
		int docSub = selCur.doc;
		int wk = PwwdWw(wwCur)->wk;

		if (wk | wkSDoc)
			/* jump out of ftn/atn/hdr document into mother doc */
			NewCurWw((*hmwdCur)->wwUpper, fTrue);

		if (selCur.doc != DocMother(selCur.doc))
			{
			TurnOffSel(&selCur);
			selCur.sk = skNil;
			selCur.doc = DocMother(selCur.doc);
			}

		if (PdodDoc (docSub)->fFtn || PdodDoc (docSub)->fAtn)
			{
			/*  select the reference mark of where we were in sub doc */
			cpRef = CpRefFromCpSub(docSub,cpSub,
					PdodDoc(docSub)->fFtn ? edcDrpFtn : edcDrpAtn );
			cpRef = CpMin(cpRef, CpMacDocEdit(selCur.doc));
			}
		else  if (PdodDoc(docSub)->fDispHdr)
			{
			cpRef = CpMomFromHdr(docSub);
			}
		else
			{
			ReportSz("unexpected case");
			cpRef = cp0;
			}
		SelectIns (&selCur,cpRef);
		vfSeeSel = fTrue;
		if (pcmb->kc != kcNil)
			/* keyboard use: end of command */
			return cmdOK;
		}

	Assert (selCur.doc != docNil && selCur.doc == DocMother (selCur.doc));

LDoDlg:

	if (FCmdFillCab())
		{
		if (!FSetCabSz(pcmb->hcab, szEmpty, Iag(CABGOTO, hszGoto)))
			return cmdNoMemory;
		}

	if (FCmdDialog())
		{
		if (pcmb->kc != kcNil)
			{
			/*  invoked by keyboard, bring up prompt */
			sz[0] = '\0';
			pcmb->tmc = TmcInputPmtMst(mstGoto, sz, cchBkmkMax,
					bcmGoto, MmoFromCmg(cmgSelect));
			if (!FSetCabSz(pcmb->hcab, sz, Iag(CABGOTO, hszGoto)))
				return cmdNoMemory;

			if (vfRecording && pcmb->tmc == tmcOK)
				{
				FRecordCab(pcmb->hcab, IDDGoto, tmcOK, bcmGoto);
				}
			}

		if (pcmb->kc == kcNil || pcmb->tmc == tmcCmdKc)
			{
			/* invoked by menu OR key repeated, bring up dialog */
			CHAR dlt [sizeof (dltGoto)];

			BltDlt(dltGoto, dlt);
			pcmb->tmc = TmcOurDoDlg(dlt, pcmb);
			}

		switch (pcmb->tmc)
			{
#ifdef DEBUG
		default:
			Assert(fFalse);
			return cmdError;
#endif
		case tmcError:
			ErrorEid(eidNoMemory, "FDoGotoDlg");
			return cmdNoMemory;

		case tmcCancel:
			return cmdCancelled;

		case tmcOK:
			break;
			}
		}

	if (FCmdAction())
		{
		GetCabSz(pcmb->hcab, sz, cchBkmkMax, Iag(CABGOTO, hszGoto));

		/* If sel is off screen, start from first cp on screen */
		EnsureSelInWw(fTrue, &selCur);

		if (!FGotoSz(sz))
			return cmdError;
		}

	return cmdOK;
}



/* C M D  F I N D  N E X T */
/* Repeat previous search or goto */
/* %%Function:  CmdFindNext  %%Owner:  bobz       */

CMD CmdFindNext(pcmb)
CMB *pcmb;
{
	CP		cpFirst, cpLim;
	CHAR		szNext[2];
	extern FNI	fni;

	Assert (PselActive() == &selCur);

	switch (fni.is)
		{
#ifdef DEBUG
	case isGotoCP:
#endif /* DEBUG */
	default:
		Assert(fFalse);
		break;

	case isNever:  /* no previous search or goto */
		ChainCmd(bcmSearch);
		break;

	case isSearch:
		SearchAgain();
		break;

	case isGotoChar:
		if (selCur.doc != DocMother(selCur.doc))
			{
			ChainCmd(bcmGoto);
			break;
			}

		szNext[0] = fni.chGoto;
		szNext[1] = '\0';

		/* If sel is off screen, start from first cp on screen */
		EnsureSelInWw(fTrue, &selCur);

		if (!FGotoSz(szNext))
			return cmdError;
		break;

	case isGotoPercent:
		if (fni.wPercent <= 100)
			cpFirst = CpFirstSty (wwCur, selCur.doc, CpFirstSty (wwCur, selCur.doc,
					(fni.wPercent * (CpMacDoc (selCur.doc)-ccpEop)) / 100,
					styChar, fFalse /* fEnd */), styLine, fFalse);
		else
			cpFirst = CpMacDocEdit (selCur.doc);
		MakeExtendSel(&selCur, cpFirst, cpFirst, (maskFDoAdjust | maskFDoMakeSelCurVisi)  /*grpf*/);
		break;

	case isGotoBkmk:
		if (fni.doc != selCur.doc || selCur.doc != DocMother(selCur.doc))
			{
			ChainCmd(bcmGoto);
			break;
			}

		if (fni.ibkf < ibkmkSpecialMin)
			BkmkCpsFromIbkf(fni.doc, fni.ibkf, &cpFirst, &cpLim);
		else  if (!FBkmkCpsIbkmkSpecial(fni.doc, fni.ibkf, &cpFirst,
				&cpLim))
			return cmdError;
		/*  takes care of extend, SeeSel, etc. */
		MakeExtendSel( &selCur, cpFirst, cpLim, (maskFDoAdjust | maskFDoMakeSelCurVisi)  /*grpf*/);
		break;

	case isNextFld:
		return CmdNextField();

	case isPrevFld:
		return CmdPreviousField();
		}

	return cmdOK;
}




/* F  G O T O  S Z */
/*  Parse sz.  Go to the indicated location.  If sz does not parse
	correctly, return fFalse.
	sz is modified to a proper string to be used for repeated goto's.
*/
/*  %%Function:  FGotoSz   %%Owner:  bobz       */

FGotoSz (sz)
CHAR * sz;

{
	CP cpFirst, cpLim;
	int eid = eidNil;
	int ibkmk;
	BOOL fBookmark = fFalse;
#ifdef DEBUG
	CHAR *pchCP;
#endif /* DEBUG */
	CHAR st [cchBkmkMax];

	extern FNI fni;

	Assert (CchSz (sz) <= cchBkmkMax);
	SzToSt (sz, st);

	/* strip trailing spaces */
	while (*st && st[*st] == ' ')
		st[0]--;

	/*  CASE I: BOOKMARK */
	if (FSearchBookmark (selCur.doc, st, &cpFirst, &cpLim, &ibkmk,
			bmcUser|bmcDoc|bmcSelect))
		{
		fBookmark = fTrue;
		fni.is = isGotoBkmk;
		fni.ibkf = ibkmk;
		fni.doc	= selCur.doc;
		}

#ifdef DEBUG
	else  if (pchCP = index (sz, 'C'))
		{  /* goto cp:  <num>C */
		*pchCP = '\0';
		cpFirst = cpLim = WFromSzNumber (sz);
		fni.is = isGotoCP;
		fni.doc = selCur.doc;
		}
#endif /* DEBUG */

	/*  CASE II: PERCENT */
	else  if (index (sz, chGotoPercent) != NULL)
		{
		CHAR *pch = sz, *pchMax = pch + *st;
		int wPercent = 0;

		while (pch < pchMax && !FDigit (*pch)) pch++;
		while (pch < pchMax && FDigit (*pch))
			wPercent = (wPercent * 10) + (*pch++ - '0');

		if (wPercent > 100)
			cpFirst = cpLim = CpMacDocEdit (selCur.doc);
		else
			{
			wPercent = max(0, wPercent);
			cpFirst = cpLim = CpFirstSty (wwCur, selCur.doc,CpFirstSty (wwCur, selCur.doc,
					(wPercent * (CpMacDoc (selCur.doc)-ccpEop)) / 100,
					styChar, fFalse /* fEnd */), styLine, fFalse);
			}
		fni.is = isGotoPercent;
		fni.wPercent = wPercent;
		fni.doc = selCur.doc;
		}

	/*  CASE III: GENERAL EXPRESSION */
	else
		{
		int gg, ggLow;
		union GCB gcb;

		SetBytes (&gcb, 0, sizeof (union GCB));

		/*  check for exception: next foo */
		if (!FEvalExceptions (st, &gcb))
			/*  not exception: parse each sub-expression */
			{
			CHAR *pch = sz;

			while (*pch)
				if ((eid = EidEvalGotoSE (&pch, &gcb)) != eidNil)
					goto GotoError;
			}

		/*  check validity of resulting Goto Control Block */

		/*  can not specify more than one of 'f', 'l' and 'a' */
		if (gcb.gsdLine.fUsed + gcb.gsdFtn.fUsed + gcb.gsdAtn.fUsed > 1)
			{
			eid = eidGotoLineAndFoot;
			goto GotoError;
			}

		/*  find highest level used */
		for (gg = ggSection; gg <= ggLast && !gcb.mpgggsd [gg].fUsed; gg++);

		/*  find the lowest level used. */
		for (ggLow = ggLast; ggLow > ggNil && !gcb.mpgggsd[ggLow].fUsed;
				ggLow--);
		if (ggLow != ggNil)
			{
			/* Modify the string so that, if used next time around, it
				will proceed to the next occurrence of the target, i.e. page. */
			fni.is = isGotoChar;
			fni.chGoto = mpggch[ggLow];
			fni.doc = selCur.doc;
			}

		/*  expression was empty */
		if (gg > ggLast)
			{
			eid = eidGotoNothing;
			goto GotoError;
			}

		/*  other than highest used may not be relative (ignore if it is) */
		while (++gg <= ggLast)
			if (gcb.mpgggsd [gg].fRelative)
				{
				gcb.mpgggsd [gg].fRelative = fFalse;
				ReportSz("Only highest group may be relative!");
				}

		/*  get the resulting cp */
		cpFirst = cpLim = CpGotoPgcb (selCur.doc, selCur.cpFirst, &gcb);

		}

	if (eid != eidNil)
		{
GotoError:
		ErrorEid(eid, "FGotoSz");
		return fFalse;
		}

	/* save sel before and after the goto */
	PushInsSelCur();

	/*  takes care of extend, SeeSel, etc. */
	MakeExtendSel(&selCur, cpFirst, cpLim, (maskFDoAdjust | maskFDoMakeSelCurVisi)  /*grpf*/);
	PushInsSelCur();

	if (vfSeeSel && hwwdCur != hNil)
		{
		SeeSel();
		vfSeeSel=fFalse;
		}

	return fTrue;

}



/* F  E V A L  E X C E P T I O N S */
/*  An empty string or a string containing only a group char is a quick
	way of saying "next foo."  does not correspond to sub expression
	evaluation, handle as exceptions here.
*/
/*  %%Function:  FEvalExceptions   %%Owner:  bobz       */

FEvalExceptions (st, pgcb)
CHAR *st;
union GCB *pgcb;

{

	int gg = ggPage;

	if (!*st)
		/* goto next page */
		goto GotoNext;

	else  if (*st == 1)
		for (gg = ggLast; gg > ggNil; gg--)
			if (st [1] == mpggch [gg])
				{
GotoNext:
				pgcb->mpgggsd [gg].fUsed = fTrue;
				pgcb->mpgggsd [gg].fRelative = fTrue;
				pgcb->mpgggsd [gg].wValue = 1;
				return fTrue;
				}

	return fFalse;
}


/* E I D  E V A L  G O T O  S E */
/*  Evaluate a single subexpression element in the general goto expression.
	Returns eidNil if successfully evaluated.  Else tries to guess at a
	good error message.
*/
/*  %%Function:  EidEvalGotoSE   %%Owner:  peterj       */

EidEvalGotoSE (ppch, pgcb)
CHAR **ppch;
union GCB *pgcb;

{
	int gg;
	CHAR * pch = *ppch;
	CHAR ch;
	BOOL fRelativeBack = fFalse;
	struct GSD gsd;

	SetBytes (&gsd, 0, sizeof (struct GSD));
	gsd.fUsed = fTrue;

	/*  skip leading white */
	while (FWhite (*pch))
		pch++;

	/*  next should be (optional) group character */
	ch = ChLower (*pch);
	for (gg = ggLast; gg > ggNil; gg--)
		if (ch == mpggch [gg])
			{
			pch++;
			break;
			}

	/*  next may be (optional) sign */
	switch (*pch)
		{
	case chGoBackRel:
		fRelativeBack = fTrue;
		/* fall through */

	case chGoFrwdRel:
		gsd.fRelative = fTrue;
		pch++;
		}

	/*  process numeric argument */
	if (FDigit (*pch))
		{
		do
			{
			if (gsd.wValue != 32767)
				{
				gsd.wValue = (gsd.wValue * 10) + (*pch++ - '0');
				if (gsd.wValue < 0)
					gsd.wValue = 32767;  /* overflow set to max pos val */
				}
			else
				pch++;
			}
		while (FDigit (*pch));
		*ppch = pch;
		}

	/*  no numeric argument */
	else
		{
		*ppch = pch;

		/*  +/- implies +1/-1 */
		if (gsd.fRelative)
			gsd.wValue = 1;

		/*  group character w/o argument ==> current (rel + 0) */
		else  if (gg != ggNil)
			gsd.fRelative = fTrue;

		/*  unknown group character ==> likely bookmark */
		else  if (*pch != '\0')
			return eidNoBkmk;

		/*  end of input */
		else
			return eidNil;
		}

	/*  adjust sign */
	Assert (gsd.wValue >= 0);
	if (fRelativeBack)
		gsd.wValue = -gsd.wValue;

	/*  if no group character, default to page */
	if (gg == ggNil)
		gg = ggPage;

	/*  assure this group not already used */
	if (pgcb->mpgggsd [gg].fUsed)
		return eidGotoConflict;

	pgcb->mpgggsd [gg] = gsd;

	return eidNil;

}


/* C P  G O T O  P G C B */
/*  Given a goto control block, figure out where to go to.
*/
/*  %%Function:  CpGotoPgcb   %%Owner:  bobz       */

CP CpGotoPgcb (doc, cpSel, pgcb)
int doc;
CP cpSel;
union GCB *pgcb;

{
	int dw = -1; /* for absolute */
	CP cp = cp0;
	CP cpFirst = cp0;
	CP cpLim = CpMacDoc(doc);
	struct RPL rpl;

#ifdef SEEGOTO
		{
		int gg;
		CHAR *pch, sz [80];

		CommSz ("CpGotoPgcb:\n\r");

		for (gg = ggSection; gg <= ggLast; gg++)
			{
			pch = sz;
			*pch++ = mpggch [gg];
			pch += CchCopySz (": ", pch);
			if (pgcb->mpgggsd [gg].fUsed)
				{
				if (pgcb->mpgggsd [gg].fRelative &&
						pgcb->mpgggsd [gg].wValue >= 0)
					*pch++ = '+';
				CchIntToPpch (pgcb->mpgggsd [gg].wValue, &pch);
				}
			CchCopySz ("\n\r", pch);
			CommSz (sz);
			}
		}
#endif /* SEEGOTO */


	if ((*hwwdCur)->fPageView && pgcb->gsdPage.fUsed)
		{
		/* repaginate the whole doc to get a updated pgd */
		SetWords(&rpl, pgnMax, cwRPL);
		rpl.cp = cpLim;
		Assert(doc == PmwdWw(wwCur)->doc);
		FRepaginateDoc(doc, &rpl, patSilent);
		}

	if (pgcb->gsdSection.fUsed)
		/*  highest level is section */
		{
		int ised;

		if (pgcb->gsdSection.fRelative)
			/*  relative section to cpSel */
			ised = IsedFromCp (doc, cpSel) + pgcb->gsdSection.wValue;
		else
			ised = pgcb->gsdSection.wValue - 1;

		cp = CpFirstIsedGoto (doc, ised, &cpFirst, &cpLim);

		if (pgcb->gsdPage.fUsed)
			/*  page also specified -- use pgn, not ipgd */
			{
			int ipgd = IpgdPgnCpSect (doc, pgcb->gsdPage.wValue, cp);
			cp = CpFirstIpgdGoto (doc, ipgd, &cpFirst, &cpLim);
			}

		}

	else  if (pgcb->gsdPage.fUsed)
		/*  page is highest level use ipgd */
		{
		int ipgd;

		if (pgcb->gsdPage.fRelative)
			{
			Assert (doc == DocMother (doc));
			CachePage(doc, cpSel);
			ipgd = vipgd + pgcb->gsdPage.wValue;
			}
		else
			ipgd = pgcb->gsdPage.wValue - 1;

		cp = CpFirstIpgdGoto (doc, ipgd, &cpFirst, &cpLim);
		}

	else  if (pgcb->gsdLine.fRelative || pgcb->gsdFtn.fRelative ||
			pgcb->gsdAtn.fRelative)
		/*  actions relative to cpSel, not cp0 */
		{
		cp = cpSel;
		dw = 0;
		}

	if (pgcb->gsdLine.fUsed)
		cp = CpLineFromCpCl (doc, cp, pgcb->gsdLine.wValue + dw, &cpFirst,
				&cpLim);

	else  if (pgcb->gsdFtn.fUsed)
		cp = CpRefFromCpCftnatn (doc, cp, pgcb->gsdFtn.wValue, !dw,
				&cpFirst, &cpLim, edcDrpFtn);

	else  if (pgcb->gsdAtn.fUsed)
		cp = CpRefFromCpCftnatn (doc, cp, pgcb->gsdAtn.wValue, !dw,
				&cpFirst, &cpLim, edcDrpAtn);

	return cp;


}




/* I S E D  F R O M  C P */
/*  Return the section index which contains cp.
*/
/*  %%Function:  IsedFromCp   %%Owner:  bobz       */

IsedFromCp (doc, cp)
int doc;
CP cp;

{
	struct PLC **hplcsed = PdodDoc (doc)->hplcsed;
	int ised;

	Assert (doc == DocMother (doc));
	if (hplcsed == hNil)
		return 0;
	ised = IInPlc (hplcsed, cp);
	Assert (ised >= 0 && ised < (*hplcsed)->iMac);
	return ised;
}



/* C P  F I R S T  I S E D  G O T O */
/*  Return cpFirst of section ised. Bound section by *pcpFirst & *pcpLim.
	Return bounds of section in *pcpFirst/Lim.
*/
/*  %%Function:  CpFirstIsedGoto   %%Owner:  bobz       */

CP CpFirstIsedGoto (doc, ised, pcpFirst, pcpLim)
int doc, ised;
CP *pcpFirst, *pcpLim;

{
	CP cpFirst;
	struct PLC **hplcsed = PdodDoc (doc)->hplcsed;

	Assert (doc == DocMother (doc));
	if (hplcsed == hNil)
		/*  no sections, use min value */
		return *pcpFirst;

	if (ised < 0)
		ised = 0;
	else  if (ised >= IMacPlc( hplcsed ))
		ised = IMacPlc(hplcsed) - 1;

	Assert (ised >= 0 && ised < IMacPlc(hplcsed));
	cpFirst = CpPlc( hplcsed, ised );

	/*  if out of range, find section that is in range */
	if (cpFirst < *pcpFirst)
		{
		ised = IInPlcRef (hplcsed, *pcpFirst);
		Assert (ised >= 0 && ised < IMacPlc(hplcsed));
		}
	else  if (cpFirst >= *pcpLim)
		{
		ised = IInPlc (hplcsed, *pcpLim - 1);
		Assert (ised >= 0 && ised < IMacPlc(hplcsed));
		}

	cpFirst = CpPlc( hplcsed, ised );

	/*  set new bounds */
	Assert (cpFirst >= *pcpFirst && cpFirst < *pcpLim);
	*pcpFirst = cpFirst;
	*pcpLim = CpPlc( hplcsed, ised + 1 );

	return cpFirst;
}



/* I P G D  P G N  C P  S E C T */
/*  Given a pgn (printed page number) and a cp, determine what ipgd in the
	document is in the same section as cp and has that page number.
*/
/*  %%Function:  IpgdPgnCpSect   %%Owner:  bobz       */

IpgdPgnCpSect (doc, pgn, cp)
int doc, pgn;
CP cp;

{
	struct PLC **hplcpgd = PdodDoc (doc)->hplcpgd;
	extern int vipgd;
	struct PGD pgd;

	Assert (doc == DocMother (doc));
	if (hplcpgd == hNil)
		return 0;
	CacheSect (doc, cp);
	CachePage (doc, caSect.cpFirst);
	GetPlc( hplcpgd, vipgd, &pgd );
	return vipgd + pgn - pgd.pgn;
}




/* C P  F I R S T  I P G D  G O T O */
/*  Given an ipgd and a pair of bounding cp's, give the cpFirst of
	that ipgd if in range else the cpFirst of the first or last page in the
	range.  Adjust *pcpFirst and *pcpLim to reflect the returned pages
	bounds.
*/
/*  %%Function:  CpFirstIpgdGoto   %%Owner:  bobz       */

CP CpFirstIpgdGoto (doc, ipgd, pcpFirst, pcpLim)
int doc, ipgd;
CP *pcpFirst, *pcpLim;

{
	CP cpFirst;
	struct PLC **hplcpgd = PdodDoc (doc)->hplcpgd;

	Assert (doc == DocMother (doc));
	Assert(*pcpFirst < *pcpLim);
	if (hplcpgd == hNil)
		return *pcpFirst;

	if (ipgd < 0)
		ipgd = 0;
	else  if (ipgd >= IMacPlc( hplcpgd ))
		ipgd = IMacPlc(hplcpgd) - 1;

	Assert (ipgd >= 0 && ipgd < IMacPlc(hplcpgd));
	CachePage(doc, CpFromIpgd(doc, wwCur, ipgd)); /* HM */

	/*  bound the page */
	if (caPage.cpFirst < *pcpFirst)
		CachePage(doc, *pcpFirst);

	else  if (caPage.cpFirst >= *pcpLim)
		CachePage(doc, *pcpLim - 1);
	cpFirst = CpMax(caPage.cpFirst, *pcpFirst);

	/*  set new bounds */
	Assert (cpFirst >= *pcpFirst && cpFirst < *pcpLim);
	*pcpLim = caPage.cpLim;

	return *pcpFirst = cpFirst;
}




/* C P  L I N E  F R O M  C P  C L */
/*  Give the cpFirst of the displayed line which is cl lines away from
	the line containing cp.  Limit return value *pcpFirst <= cp < *pcpLim.
*/
/*  %%Function:  CpLineFromCpCl   %%Owner:  bobz       */

CP CpLineFromCpCl (doc, cp, cl, pcpFirst, pcpLim)
int doc;
CP cp;
int cl;
CP *pcpFirst, *pcpLim;

{
	int dl;  /* number of lines in para avaliable */
	int il;  /* index of current line in hplc */
	struct PLC **hplc = HplcInit (0, 5, cp0, fTrue /* ext rgFoo */);
	CP cpFirstPara, cpLimPara;
	BOOL fForward = cl > 0;
	BOOL fFirstPass = fTrue;
	struct DOD **hdod = mpdochdod [doc];
	struct WWD *pwwd;

	cl = abs (cl);
	if (hplc == hNil)
		return cp;

	Assert(doc == DocMother(doc));

	SetFlm( flmRepaginate );
	/* if print environment's changed, trash hplcpad and hplcpgd */
	CheckPagEnv(doc, vprsu.fSeeHidden, vprsu.fShowResults);
	LinkDocToWw(doc, wwTemp, wwNil);
	pwwd = PwwdWw(wwTemp);
	pwwd->grpfvisi.w = 0;
	pwwd->grpfvisi.fSeeHidden = vprsu.fSeeHidden;
	pwwd->grpfvisi.flm == flmRepaginate;
	pwwd->grpfvisi.grpfShowResults = vprsu.fShowResults ? ~0 : 0;
	pwwd->grpfvisi.fForceField = fTrue;

	while (cp >= *pcpFirst && cp < *pcpLim)
		{
		Assert (cl >= 0);

		if (fFirstPass)
			fFirstPass = fFalse;
		else if ((dl = ClMacFromPlcpadPhe (wwTemp, doc, cp, &cpFirstPara, &cpLimPara))
				!= 0 && dl <= cl)
			{
#ifdef SEEGOTO
			CommSzNum ("Using Phe: ", dl);
#endif /* SEEGOTO */
			goto LSkipWholePara;
			}

		/* insufficient information in plcpad, must format whole para */
		GetCpFirstCpLimDisplayPara (wwTemp, doc, cp, &cpFirstPara, &cpLimPara);
		if (!FFormatDisplayPara (wwTemp, doc, cpFirstPara, cpLimPara, hplc))
			/*  memory problems */
			break;

		/*  hplc has been filled with starting cp of each line */
		Assert (CpPlc( hplc, IMacPlc(hplc)) == cpLimPara);
		Assert (CpPlc( hplc, 0 ) == cpFirstPara);

		il = IInPlc (hplc, cp);
		dl = fForward ? IMacPlc(hplc) - il : il + 1;
#ifdef SEEGOTO
		CommSzNum ("Formatted by Brute Force: ", dl);
#endif /* SEEGOTO */
		if (dl <= cl)
			{ /* moving more lines than in this paragraph, skip over */
LSkipWholePara:
			cl -= dl;
			cp = fForward ? cpLimPara : cpFirstPara - 1;
			continue;
			}
		/* else: this para contains the line we want */
		cp = CpPlc( hplc, fForward ? il+cl : il-cl);
		break;

		}

	if (cp < *pcpFirst || cp >= *pcpLim)
		/*  result cp is out-of-range, find begin of line in range */
		{
		if (cp < *pcpFirst)
			cp = *pcpFirst;
		else
			cp = *pcpLim -1;

		GetCpFirstCpLimDisplayPara (wwTemp, doc, cp, &cpFirstPara, &cpLimPara);
		if (FFormatDisplayPara (wwTemp, doc, cpFirstPara, cp+1, hplc))
			cp = CpPlc( hplc, IInPlc( hplc, cp ) );

		}

	FreeHplc (hplc);
	return cp;
}



/* C L  M A C  F R O M  P L C P A D  P H E */
/*  If there is a valid entry in the plcpad for cp, return the number
	of lines in that para.  Else return 0
*/
/*  %%Function:  ClMacFromPlcpadPhe   %%Owner:  bobz       */

ClMacFromPlcpadPhe (ww, doc, cp, pcpFirst, pcpLim)
int ww;
int doc;
CP cp;
CP *pcpFirst, *pcpLim;

{
	struct PHE phe;
	if (PwwdWw(ww)->fOutline)
		return 0;	/* not correct for outline! */

/* FUTURE: hplcphe filled with 'visible' paragraphs generated from
	CacheParaL...we really should pass back caParaL.cpFirst/cpLim...*/

	CachePara(doc, cp);
	if (FGetValidPhe(doc, cp, &phe))
		{
		*pcpFirst = caPara.cpFirst;
		*pcpLim = caPara.cpLim;
		return phe.fDiffLines ? 0 : phe.clMac;
		}
	else
		return 0;
}










/* C P  R E F  F R O M  C P  C F T N A T N */
/*  Given a cp, move forward or back cftn/atn footnote/annotation
	references.
*/
/*  %%Function:  CpRefFromCpCftnatn   %%Owner:  bobz       */

CP CpRefFromCpCftnatn (doc, cp, c, fRel, pcpFirst, pcpLim, edcDrp)
int doc;
CP cp;
int c;
BOOL fRel;
CP *pcpFirst, *pcpLim;
int edcDrp;

{
	int i;
	CP cpRef;
	struct PLC **hplcRef;
	struct DRP *pdrp;

	pdrp = ((int *)PdodDoc(doc)) + edcDrp;
	hplcRef = pdrp->hplcRef;

	Assert (doc == DocMother (doc));
	if (hplcRef == hNil)
		return *pcpFirst;

	if (c == 0)
		c = 1;

	if (cp == cp0 && !fRel)
		i = -1;
	else
		i = IInPlcCheck (hplcRef, fRel ? cp : cp-1);
	i += c;
	if (i >= IMacPlc(hplcRef))
		i = IMacPlc(hplcRef) - 1;

	Assert (i < IMacPlc(hplcRef));
	if (i < 0)
		return *pcpFirst;
	cpRef = CpPlc( hplcRef, i );

	if (cpRef < *pcpFirst)
		return *pcpFirst;
	else  if (cpRef >= *pcpLim)
		{
		i = IInPlcCheck (hplcRef, *pcpLim - 1);
		if (i < 0)
			return *pcpFirst;
		cpRef = CpPlc( hplcRef, i );
		}

	if (cpRef < *pcpFirst)
		return *pcpFirst;
	if (cpRef >= *pcpLim)
		return *pcpLim-1;
	*pcpFirst = cpRef;
	*pcpLim = cpRef + 1;

	return cpRef;
}



#define ipbPage    0
#define ipbColumn  1
#define ipbNext    2
#define ipbCont    3
#define ipbEven    4
#define ipbOdd     5


/*  %%Function:  CmdInsBreak  %%Owner:  bobz       */

CMD CmdInsBreak(pcmb)
CMB * pcmb;
{
	BOOL fTable;

	if (pcmb->fDefaults)
		{
		((CABINSBREAK *) *pcmb->hcab)->iType = 0;
		}

	if (pcmb->fDialog)
		{
		CHAR dlt [sizeof (dltInsBreak)];

		BltDlt(dltInsBreak, dlt);
		switch (TmcOurDoDlg(dlt, pcmb))
			{
#ifdef DEBUG
		default:
			Assert(fFalse);
			return cmdError;
#endif
		case tmcError:
			ErrorEid(eidNoMemory, "CmdInsBreak");
			return cmdNoMemory;

		case tmcCancel:
			return cmdCancelled;

		case tmcOK:
			break;
			}
		}

	if (pcmb->fAction)
		{
		int bkc;
		int ch;
		struct CA ca, caRM;
		struct SEP sep;
		int dcp = 0;
		CP cpFirst;

		TurnOffSel(&selCur); /* so we don't try to turn it on when the
		EDL's are not up to date */

		/* I know this switch is ugly but we can't play games with
			the bpc's because they aren't in a nice order. */
		switch (((CABINSBREAK *) *pcmb->hcab)->iType)
			{
		case ipbPage:
			return CmdInsPageBreak(pcmb);

		case ipbColumn:
			return CmdInsColumnBreak(pcmb);

		case ipbNext:
			bkc = bkcNewPage;
			break;

		case ipbOdd:
			bkc = bkcOddPage;
			break;

		case ipbEven:
			bkc = bkcEvenPage;
			break;

		case ipbCont:
			bkc = bkcNoBreak;
			break;

#ifdef DEBUG
		default:
			Assert(fFalse);
			break;
#endif  /* DEBUG */
			}

		if (PdodDoc(selCur.doc)->fShort)
			{
			ErrorEid(eidNoSectAllowed, "CmdInsBreak");
			return cmdError;
			}

		CheckCollapseBlock();

/* include the section mark for undo to work correctly */
		cpFirst = selCur.cpFirst;
		if (fTable = FInTableDocCp(selCur.doc, cpFirst))
			{
			cpFirst = CpFirstTap(selCur.doc, cpFirst);
			InvalTableProps(selCur.doc, cpFirst, ccpSect, fTrue/*fFierce*/);
			InvalPageView(selCur.doc);
			}
		CacheSectSedMac(selCur.doc, cpFirst);
		ca = caSect;
		ca.cpFirst = cpFirst;
		caSect.doc = docNil;
		AssureLegalSel(&ca);
		if (!FSetUndoB1(ucmInsertSect, uccPaste, &ca))
			{
			Beep();
			return cmdCancelled;
			}
#ifdef FUTURE
/* ignore autodelete for insert break because CmdAutoDelete messes up docUndo again */
		if (!FSelWithinTable(&selCur))
			if (CmdAutoDelete(&caRM) != cmdOK)
				return cmdCancelled;
#endif
		if (fTable) /* do after the SetUndo so selCur is undone properly */
			SelectIns(&selCur, cpFirst);

/* Code merge bug fix: passing &vsepFetch to CmdInsertSect1 when the 
	content is not even valid is a NO NO! */
		CacheSect(selCur.doc, cpFirst+dcp);
		bltb(&vsepFetch, &sep, cbSEP);
		sep.bkc = bkc;
		if (CmdInsertSect1(&selCur.ca, &sep, 0, 0 /* pchp/pap */,
				fTrue /* fSepNext */, fTrue /* fRM */ ) != cmdOK)
			return cmdCancelled;

		SelectIns(&selCur, selCur.cpFirst + 1);
		CacheSectSedMac(selCur.doc, cpFirst + 1);
		AssureLegalSel(PcaSet(&ca, selCur.doc, cpFirst, caSect.cpLim));
		SetUndoAfter(&ca);
		caSect.doc = docNil;
		vfSeeSel = fTrue;
		}

	return cmdOK;
}


/*  CmdInsBookmark

	This function is called by the Insert Bookmark menu item or by a
	macro.  If called from the menu, a dialog is presented to
	allow the user to give the current selection a new name, modify an
	old bookmark to refer to the current selection or to delete an
	existing bookmark.  The dialog may be used to delete multiple
	bookmarks (the dialog does not exit on deletion).

	The CAB for this dialog has an extra field appended to the end (tmcRet).
	This field is used to allow a macro to indicate that it wishes to delete
	a bookmark instead of inserting/modifying one.  The dialog calling
	routine will set this field to either tmcOk or tmcCancel which are
	the only valid exit states from the dialog.  A macro should set up
	the CAB and then set the field to either tmcOk or tmcDeleteBkmk.
	The CmdInsBookmark code will perform a insert/modify in tmcOk, a
	deletion on tmcDeleteBkmk and no action on tmcCancel.
*/
/*  %%Function:  CmdInsBookmark  %%Owner:  bobz       */

CMD CmdInsBookmark(pcmb)
CMB * pcmb;
{
	int iBkmk;
	int doc;
	BOOL fUndoCancel;

	doc = selCur.doc;

	/*  bookmarks can be inserted into a "mother" document only. */
	if (doc == docNil || doc != DocMother (selCur.doc))
		{
		Assert (fFalse);
		ModeError();
		return cmdError;
		}


	if (FCmdFillCab())
		{
		if (!FSetCabSz(pcmb->hcab, szEmpty, Iag(CABINSBOOKMARK, hszBkmkName)))
			{
			return cmdNoMemory;
			}
		}

	if (FCmdDialog())
		{
		extern BOOL vfScanDlg;

#ifdef DEBUG
		if (vfScanDlg)
			goto LDoDlg;
#endif /* DEBUG */

		pcmb->tmc = tmcOK;

		if (pcmb->kc != kcNil)
			{
			CHAR sz [cchBkmkMax];

			/* invoked by keyboard, bring up prompt */
			sz[0] = '\0';
			pcmb->tmc = TmcInputPmtMst(mstInsBookmark, sz, 
					cchBkmkMax-1, bcmInsBookmark,
					MmoFromCmg(cmgApplyProp));
			if (!FSetCabSz(pcmb->hcab, sz, Iag(CABINSBOOKMARK, hszBkmkName)))
				return cmdNoMemory;

			if (vfRecording && pcmb->tmc == tmcOK)
				{
				FRecordCab(pcmb->hcab, IDDInsBookmark, tmcOK, bcmInsBookmark);
				}
			}

		if (pcmb->kc == kcNil || pcmb->tmc == tmcCmdKc)
			{
			CHAR dlt [sizeof (dltInsBookmark)];
LDoDlg:
			/* invoked by menu OR key repeated */
			BltDlt(dltInsBookmark, dlt);
			pcmb->tmc = TmcOurDoDlg(dlt, pcmb);
			}

		switch (pcmb->tmc)
			{
#ifdef DEBUG
		default:
			Assert(fFalse);
			return cmdError;
#endif
		case tmcError:
			ErrorEid (eidNoMemory, "CmdInsBookmark");
			return cmdNoMemory;

		case tmcCancel:
			return cmdCancelled;

		case tmcOK:
			break;
			}
		}

	if (FCmdAction())
		{
		BOOL fCancelled;
		CHAR st [cchMaxSz + 1];

		switch (pcmb->tmc)
			{
		case tmcOK:
			GetCabSz(pcmb->hcab, st + 1, cchMaxSz,
					Iag(CABINSBOOKMARK, hszBkmkName));
			st[0] = CchStripString(st + 1, CchSz(st + 1) - 1);

			if (!FLegalBkmkName(st))
				{  /* must start with an alpha char */
				ErrorEid(eidInvalBkmkNam, "CmdInsBookmark");
				return cmdError;
				}

			if (!FInsertStBkmk(&selCur.ca, st, &fCancelled))
				return fCancelled ? cmdCancelled : cmdError;
			break;

		case tmcDeleteBkmk:
			if (!FDeleteBkmkHcab(pcmb->hcab))
				return cmdError;
			break;
			}
		}

	return cmdOK;
}


/*  %%Function:  FDeleteStBkmk  %%Owner:  bobz       */

FDeleteStBkmk(stBkmkName)
CHAR * stBkmkName;
{
	int doc, iBkmk;
	BOOL fUndoCancel;

	doc = selCur.doc;
	if (!FSearchBookmark(doc, stBkmkName, NULL, NULL, &iBkmk, bmcUser))
		return fFalse;

	DeleteIBkmk(doc, iBkmk, &fUndoCancel);
	if (fUndoCancel)
		return fFalse;

	return fTrue;
}


/* D E L E T E  I B K M K */
/* This function deletes bookmark iBkmk from document doc.  If
pfSetUndo is non-NULL, the undo document is set up to allow un-doing
of the deletion.  If the bookmark deleted is the last bookmark
in the document, the bookmark handles are freed.  If pfUndoCancel
is non-null then the undo document will be set up, and the flag
returned in pfUndoCancel will be true iff the user refused to
continue without undo. 
*/
/*  %%Function:DeleteIBkmk %%Owner:peterj %%reviewed: 6/28/89 */
DeleteIBkmk (doc, ibkf, pfUndoCancel)
int doc;
int ibkf;
int *pfUndoCancel;
{
	struct DOD **hdod = mpdochdod [doc];
	struct STTB **hsttb = (*hdod)->hsttbBkmk;
	struct PLC **hplcbkf = (*hdod)->hplcbkf;
	struct PLC **hplcbkl = (*hdod)->hplcbkl;

	if (pfUndoCancel)
		*pfUndoCancel = fFalse;

	if (hdod == hNil || hsttb == hNil || hplcbkf == hNil || hplcbkf == hNil)
		{
		Assert (fFalse);
		return;
		}

	Assert (doc == DocMother (doc));
	Assert (!(*hdod)->fShort);

	DeleteIbkf (doc, ibkf, hsttb, hplcbkf, hplcbkl, pfUndoCancel);
	if (pfUndoCancel && *pfUndoCancel)
		return;

	if (IMacPlc(hplcbkf) == 0)
		/*  no more bookmarks, get rid of structures */
		{
		struct DOD *pdod = PdodDoc(doc);
		FreezeHp ();
		FreePhsttb (&pdod->hsttbBkmk);
		FreePhplc (&pdod->hplcbkf);
		FreePhplc (&pdod->hplcbkl);
		MeltHp ();
		}
}





/*  FDlgInsBookmark
	This function is called by the insert Bookmark dialog.
	on initialization, it fills the combo box with the names of
	any existing bookmarks.  On a delete it ensures that the
	requested bookmark exists and if it does deletes it & refills
	the list box.
*/

/*  %%Function:  FDlgInsBookmark  %%Owner:  bobz       */

BOOL FDlgInsBookmark(dlm, tmc, wNew, wOld, wParam)
DLM dlm;
TMC tmc;
WORD wNew, wOld, wParam;
{
	int ibkmk;
	HCAB hcab;
	CP cpFirst, cpLim;
	CHAR st [ichMaxBufDlg + 1];

	switch (dlm)
		{
	case dlmInit:
		EnableTmc(tmcOK, fFalse);
		EnableTmc(tmcDeleteBkmk, fFalse);
		break;

	case dlmClick:
		if (tmc == tmcDeleteBkmk)
			{
			/* set the focus back on name */
			SetFocusTmc(tmcBkmkName & ~ftmcGrouped);

			if ((hcab = HcabFromDlg(fFalse)) == hcabNotFilled)
				return fFalse;
			if (hcab == hcabNull)
				/* sdm filter will take down dialog */
				return (fTrue);

			if (!FDeleteBkmkHcab(hcab))
				{
				SetTmcTxs(tmcBkmkName & ~ftmcGrouped, 
						TxsOfFirstLim(0, ichLimLast));
				return fFalse;
				}

			SetTmcText(tmcBkmkName, szEmpty);

			/* update list box to reflect deleted item */
			RedisplayTmc(tmcBkmkName);
			EnableTmc(tmcOK, fFalse);
			EnableTmc(tmcDeleteBkmk, fFalse);

			ChangeCancelToClose();
			SetDefaultTmc(tmcCancel);
			}
		break;

	case dlmChange:
		if (tmc == (tmcBkmkName & ~ftmcGrouped))
			{
			GrayButtonOnBlank(tmcBkmkName & ~ftmcGrouped, 
					tmcOK);
			GrayButtonOnBlank(tmcBkmkName & ~ftmcGrouped, 
					tmcDeleteBkmk);
			}
		break;

	case dlmTerm:
		if (tmc != tmcOK)
			break;

		/* verify valid bookmark name */
		GetTmcText(tmcBkmkName & ~ftmcGrouped, 
				st + 1, ichMaxBufDlg);
		st[0] = CchStripString(st + 1, CchSz(st + 1) - 1);

		if (!FLegalBkmkName(st))
			{
			/* must start with a letter and not be too long */
			ErrorEid(eidInvalBkmkNam, "DlgfInsBookmark");
			SetTmcTxs(tmcBkmkName & ~ftmcGrouped, 
					TxsOfFirstLim(0, ichLimLast));

			return fFalse;
			}
		break;
		}

	return fTrue;
}


/*  %%Function:  FDeleteBkmkHcab  %%Owner:  bobz       */

FDeleteBkmkHcab(hcab)
HCABINSBOOKMARK hcab;
{
	int ibkmk;
	CP cpFirst, cpLim;
	CHAR st [cchBkmkMax + 2];

	GetCabSz(hcab, st + 1, cchBkmkMax + 1, 
			Iag(CABINSBOOKMARK, hszBkmkName));
	st[0] = CchStripString(st + 1, CchSz(st + 1) - 1);

	if (!FSearchBookmark(selCur.doc, st, &cpFirst, &cpLim, &ibkmk, bmcUser))
		{
		ErrorEid(eidNoBkmk, "FDeleteBkmkHcab");

		return fFalse;
		}

	/* else:  ok to delete iBkmk */
	return FDeleteStBkmk(st);
}



/* W  L I S T  B K M K */
/* %%Function:  WListBkmk  %%Owner:  bobz       */

EXPORT WORD WListBkmk(tmm, sz, isz, filler, tmc, wParam)
TMM tmm;
CHAR * sz;
int isz;
WORD filler;
TMC tmc;
WORD wParam;
{
	struct STTB **hsttb;

	Assert(selCur.doc != docNil && !PdodDoc(selCur.doc)->fShort);

	switch (tmm)
		{
	case tmmCount:
		return -1;

	case tmmText:
		if ((hsttb = PdodMother(selCur.doc)->hsttbBkmk) == hNil
				|| isz >= (*hsttb)->ibstMac)
			return fFalse;
		GetSzFromSttb(hsttb, isz, sz);
		return fTrue;
		}
	return 0;
}



/*  %%Function:  CmdInsFile  %%Owner:  rosiep  */

CMD CmdInsFile(pcmb)
CMB * pcmb;
{
	int 	cmd = cmdOK;
	int         tmc;
	CHAR       *pch;
	CABINSFILE *pcabinsfile;
	CHAR        szAll[6];
	CHAR        st[ichMaxFile + ichMaxBufDlg];
	DLT * pdlt;
	CHAR dlt [sizeof (dltInsFile)];
	BOOL        fAddPara = fFalse;

	extern CHAR stDOSPath[];

	tmc = tmcOK;
	BuildSzAll(szDoc, szAll);
	if (pcmb->fDefaults)
		{
		FSetCabSz(pcmb->hcab, szAll, Iag(CABINSFILE, hszFile));
		FSetCabSz(pcmb->hcab, szEmpty, Iag(CABINSFILE, hszRange));
		pcabinsfile = *pcmb->hcab;
		pcabinsfile->iDirectory = uNinchList;
		pcabinsfile->fLink = fFalse;
		/* fMemFail will be true if FSetCabSz fails */
		if (vmerr.fMemFail)
			return cmdNoMemory;
		}

	if (pcmb->fDialog)
		{
		/* To check if the path was changed by the save as dialog. */
		Assert(ichMaxBufDlg >= ichMaxFile);
		CopySt(stDOSPath, st);

		BltDlt(dltInsFile, dlt);

		tmc = TmcOurDoDlg(dlt, pcmb);

		if (FNeNcSt(stDOSPath, st))
			{
			if (vfRecording)
				RecordChdir(stDOSPath);
			vfFileCacheDirty = fTrue;
			UpdateTitles(); /* changes window menu, vhsttbWnd and captions */
			}

		if (tmc == tmcOK && vfRecording)
			FRecordCab(pcmb->hcab, IDDInsFile, pcmb->tmc, pcmb->bcm);

		if (tmc == tmcError)
			{
			ErrorEid(eidNoMemory, "CmdInsFile");
			return cmdError;
			}
		
		if (tmc == tmcCancel)
			return cmdCancelled;

		}

	if (pcmb->fCheck)
		{
		if (!FTermFile(pcmb, Iag(CABINSFILE, hszFile), tmcNull, 
				((CABINSFILE *) *pcmb->hcab)->fLink, fTrue, nfoNormal))
			{
			return cmdError;
			}
		}

	if (tmc == tmcOK && pcmb->fAction)
		{
		CHAR    stT[ichMaxFile + 1];

		StartLongOp();
		GetCabSz(pcmb->hcab, st, sizeof(st), Iag(CABINSFILE, hszFile));
		SzToStInPlace(st);
/* following can fail if allocations fail */
		if (!FNormalizeStFile(st, stT, nfoNormal))
			{
			SetErrorMat( matMem );
			cmd = cmdError;
			goto LRet;
			}

		pcabinsfile = PcabFromHcab(pcmb->hcab);
		if (pcabinsfile->fLink)
			{
			DoubleBSSt(stT, st);
			pch = st + st[0] + 1;
			GetCabSz(pcmb->hcab, stT, sizeof(stT), Iag(CABINSFILE, hszRange));
			SzToStInPlace(stT);
			if (stT[0] != 0 && *st + *stT + 1 < sizeof(st))
				{
				*pch++ = chSpace;
				st[0] += 1;
				bltbyte(&stT[1], pch, (int) (stT[0]));
				st[0] += stT[0];
				}
			Assert (*st < sizeof(st));
			StToSzInPlace(st);
			cmd = CmdInsFltSzAtSelCur(fltInclude, st /*szArgs*/, imiInsFile,
					fTrue /*fCalc*/, fTrue /*fShowDefault*/, fTrue /*fShowResult*/);
			}
		else
			{
			struct CA caSel, caSrc, caRM;
			int docSrc;
			int fn;
			CP  cpFirst, cpLim;
			int fNewDocSrc = fTrue;

			CopySt(stT, st);
			GetCabSz(pcmb->hcab, stT, sizeof(stT), Iag(CABINSFILE, hszRange));
			SzToStInPlace(stT);

			/* see if file is already opened and we can use it */
			if ((docSrc = DocFromSt(st)) != docNil &&
					((fn = PdodDoc(docSrc)->fn) == fnNil ||
					!PfcbFn(fn)->fForeignFormat))
				{
				fNewDocSrc = fFalse;
				if (!*stT || !FSearchBookmark (caSrc.doc = docSrc, stT, &caSrc.cpFirst,
						&caSrc.cpLim, NULL, bmcUser))
					{
					goto LWholeDoc;
					}
				caSrc.cpLim = CpMin (caSrc.cpLim, CpMacDocEdit(docSrc));
				}

			/*  try to open the file using converter & subset */
			else  if ((fn = FnOpenSt(st, 0, ofcDoc, NULL)) == fnNil ||
					(docSrc = DocCreateFn (fn, fTrue, NULL, stT, NULL)) == docNil)
				/*  open failed */
				{
				ErrorEid (eidCantOpen, " CmdInsFile");
				cmd = cmdError;
				goto LRet;
				}
			else  if (docSrc == docCancel)
				{
				cmd = cmdCancelled;
				goto LRet;
				}

			else
				/*  open succeeded */
				{
LWholeDoc:
				PcaSet( &caSrc, docSrc, cp0, CpMacDocEdit(docSrc) );
				}

			if (!FMoveOkForTable(selCur.doc, selCur.cpFirst, &caSrc))
				{
				ErrorEid(eidIllegalTextInTable, "CmdInsFile");
				cmd = cmdError;
				goto LRet;
				}

			CheckCollapseBlock();

			/* we now have the doc and the limits of interest */
			if (!FSetUndoBefore(imiInsFile, uccPaste))
				{
				cmd = cmdCancelled;
				goto LRet;
				}
			if ((cmd = CmdAutoDelete(&caRM)) != cmdOK)
				goto LRet;

			if (FInTableDocCp(docSrc, cp0))
				{
				struct CA caT;

				if (!FEopBeforePca(PcaPoint(&caT, selCur.doc, cpFirst = selCur.cpFirst)))
					{
					cmd = cmdNoMemory;
					goto LRet;
					}

				if (caT.cpFirst != cpFirst)
					{
					fAddPara = fTrue;
					selCur.cpFirst += ccpEop;
					if (selCur.fIns) /* cpLim gets adjusted by AdjustSels if skSel */
						selCur.cpLim += ccpEop;
					}
				}

			caSel = selCur.ca;
			AssureLegalSel(&caSel);
			if (!FReplaceCpsRM(&caSel, &caSrc))
				{
				cmd = cmdNoMemory;
				goto LRet;
				}

			CopyStylesFonts(docSrc, caSel.doc, caSel.cpFirst, DcpCa(&caSrc));
			caSel.cpLim = caSel.cpFirst + DcpCa(&caSrc);

			/* do properties before select, since select does UpdateWw */
			AssureNestedPropsCorrect(&caSel, fTrue);

			/* save insertion pts at beginning and end of inserted file */
			SelectIns( &selCur, caSel.cpFirst );
			PushInsSelCur();
			SelectIns (&selCur, caSel.cpLim);
			PushInsSelCur();

			caRM.cpLim += DcpCa(&caSel) + (fAddPara ? ccpEop : 0);
			SetUndoAfter(&caRM);
			vfSeeSel = fTrue;
			if (fNewDocSrc)
				{
				DisposeDoc (docSrc);
				KillExtraFns();
				}
			}
LRet:
		EndLongOp(fFalse /* fAll */);
		}

	return cmd;
}


/*  %%Function:  DoubleBSSt  %%Owner:  bobz       */

DoubleBSSt(stOrg, stDest)
CHAR    stOrg[];
CHAR    stDest[];
{
	int     ich, ichMac, cch;
	CHAR    *pchEye, *pchPen;

	pchEye = &stOrg[1];
	pchPen = &stDest[1];

	for (cch = ich = 0, ichMac = stOrg[0]; ich < ichMac; ich++, cch++)
		{
		if (*pchEye == chFieldEscape)
			{
			cch++;
			*pchPen++ = chFieldEscape;
			}
		*pchPen++ = *pchEye++;
		}
	stDest[0] = cch;
}


/* P R O M P T  U S E R  N A M E */
/* %%Function:  PromptUserName  %%Owner:  bobz       */

PromptUserName()
{
	CHAR sz [ichMaxBufDlg];
	CMB cmb;
	CHAR dlt [sizeof (dltUserName)];

	if (!vmerr.fSdmInit && !FAssureSdmInit())
		{
		SetErrorMat(matMem);
		return;
		}

	BltDlt(dltUserName, dlt);

	if ((cmb.hcab = HcabAlloc(cabiCABUSERNAME)) == hNil)
		return;

	if (!FSetCabSt(cmb.hcab, vpref.stUsrName, 
			Iag(CABUSERNAME, hszName)))
		goto LRet;
	if (!FSetCabSt(cmb.hcab, vpref.stUsrInitl,
			Iag(CABUSERNAME, hszInitials)))
		goto LRet;

	cmb.pv = NULL;
	cmb.cmm = cmmNormal;
	cmb.bcm = bcmNil;

	if (TmcOurDoDlg(dlt, &cmb) != tmcOK)
		goto LRet;

	UpdateUserName(&cmb, fFalse /* fFromCustomize */ );

	/* force the keyboard speed while we are at it */
	ForceKeyboardSpeed();

LRet:
	FreeCab(cmb.hcab);
}


/*  %%Function:  FDlgUsername  %%Owner:  bobz       */

BOOL FDlgUsername(dlm, tmc, wNew, wOld, wParam)
DLM dlm;
TMC tmc;
WORD wNew, wOld, wParam;
{
	CHAR stzName [ichMaxBufDlg + 1];
	CHAR stzInitl [ichUsrInitlMax + 1];
	CHAR stzInitlOld [ichUsrInitlMax + 1];

	switch (dlm)
		{
	case dlmChange:
		if (tmc == tmcUserName)
			{
			GetTmcText(tmcUserInitial, stzInitlOld + 1, 
					ichUsrInitlMax);
			stzInitlOld[0] = CchStripString(stzInitlOld + 1, 
					CchSz(stzInitlOld + 1) - 1);

			GetTmcText(tmcUserName, stzName + 1, 
					ichMaxBufDlg);
			stzName[0] = CchStripString(stzName + 1,
					CchSz(stzName + 1) - 1);

			GetInitials(stzName, stzInitl);

			if (FNeNcSt(stzInitl, stzInitlOld))
				{
				SetTmcText(tmcUserInitial, 
						stzInitl + 1);
				}
			}
         /* this makes sure there are <= 255 leading spaces */
		GrayButtonOnBlank(tmcUserName, tmcOK);
		break;

	case dlmInit:
		Assert(HcabDlgCur());
		GetCabSz(HcabDlgCur(), stzName, ichUsrNameMax,
				Iag(CABUSERNAME, hszName));
		GrayBtnOnSzBlank(stzName, tmcOK);
		break;
		}

	return (fTrue);
}



/*  %%Function:  UpdateUserName  %%Owner:  bobz       */

UpdateUserName(pcmb, fFromCustomize)
CMB * pcmb;
int fFromCustomize;
{
	int iag;
	CHAR stzName [ichMaxBufDlg + 1];
	CHAR stzInitl [ichUsrInitlMax + 1];


/* note: don't use f?CABX:CABY because Iag is a macro */
	if (fFromCustomize)
		iag = Iag(CABCUSTOMIZE, hszName);
	else
		iag = Iag(CABUSERNAME, hszName);

       /* dialog functions must guarantee validity of user name */
       /* there can be no > than 255 leading spaces. We
          also truncate to max valid name size
       */
	GetCabStz(pcmb->hcab, stzName, ichMaxBufDlg + 1, iag);
	stzName[0] = CchStripString(&stzName[1], (int)(stzName[0]));
	stzName[0] = min (stzName[0], ichUsrNameMax);

	Assert(stzName [0]);

	CopySt( stzName, vpref.stUsrName );

/* initials */
	if (fFromCustomize)
		iag = Iag(CABCUSTOMIZE, hszInitials);
	else
		iag = Iag(CABUSERNAME, hszInitials);

	GetCabStz(pcmb->hcab, stzInitl, ichUsrInitlMax + 1, iag);
	stzInitl[0] = CchStripString(&stzInitl[1], (int)(stzInitl[0]));
	if (stzInitl[0] == 0)
		{
/* extract initial from username */
		FreezeHp();
		GetInitials(stzName, stzInitl);
		MeltHp();
		}
	CopySt( stzInitl, vpref.stUsrInitl );
}


/*  %%Function:  GetInitials  %%Owner:  bobz       */

GetInitials(stzName, stzInitl)
CHAR * stzName;
CHAR * stzInitl;
{
	CHAR * pch = &stzName[1];
	int ich = 1;

	stzName[0] = CchStripString(stzName + 1, stzName[0]);
	pch = &stzName[1];
	while (*pch != '\0' && ich < ichUsrInitlMax)
		{
		stzInitl[ich++] = *pch++;
		while (*pch != '\0' && *pch++ != ' ');
		while (*pch != '\0' && *pch == ' ')
			pch++; /* skip multiple spaces between words */
		}
	stzInitl[ich] = '\0'; /* make it an sz */
	stzInitl[0] = ich - 1; /* make it an st too */
}


/*  %%Function:  ForceKeyboardSpeed  %%Owner:  bobz       */

ForceKeyboardSpeed()
{
	HANDLE hKeyboardModule;
	FARPROC lpfnSetSpeed;
	extern int vwWinVersion;

	if (vwWinVersion < 0x0210)
		/* don't even try in old versions of windows */
		return;

	/* first change the WIN.INI so windows will do this for us from
		now on (won't effect this windows session) */
	WriteProfileString(SzFrameKey("windows",WindowsWININI),
			SzFrameKey("KeyboardSpeed",KBSpeedWININI), SzFrame("31"));

	/* now change it for this session */
	/* NOTE: code stolen from USERs wininit1.c */
	if ((hKeyboardModule = GetModuleHandle(SzShared("KEYBOARD"))) != NULL &&
			(lpfnSetSpeed = GetProcAddress(hKeyboardModule,
					MAKEINTRESOURCE(idoSetSpeed)))
			!= NULL)
		(*lpfnSetSpeed)((int)31);
}


/* F  S E L  W I T H I N  T A B L E */
/* Description: Returns true if deleting the range of characters described
/* by psel will leave an insertion point within a table. In other words,
/* if replacing psel by a character will put that character into a table.
/* This is used to avoid putting illegal characters (like chSect) into a table.
/**/
/* %%Function:FSelWithinTable %%Owner:davidmck %%Reviewed:6/27/89 */
FSelWithinTable(psel)
struct SEL	*psel;
{
	/* does the selection begin within a table? */
#ifdef WIN
	if ((selCur.fIns || !vpref.fAutoDelete) &&
			FInTableDocCp(psel->doc, psel->cpFirst))
#else
		if (FInTableDocCp(psel->doc, psel->cpFirst))
#endif
			return fTrue;

	if (psel->fBlock && !psel->fColumn)
		return fFalse;

	/* does the selection end within a table? */
	if (psel->sk != skIns)
		{
		Assert(psel->cpLim > psel->cpFirst);
		return FInTableDocCp(psel->doc, psel->cpLim-1);
		}

	/* it's OK to have a table strictly contained within the selection */
	return fFalse;

} /* FSelWithinTable */


