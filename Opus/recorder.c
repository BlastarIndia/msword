#include "word.h"
#include "error.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "debug.h"
#include "doc.h"
#include "disp.h"
#include "ch.h"
#include "status.h"
#include "cmdtbl.h"
#include "props.h"
#include "sel.h"
#include "recorder.h"
#include "menu2.h"
#define USEBCM
#include "opuscmd.h"
#include "format.h"
#include "el.h"
#include "macrocmd.h"
#include "insert.h"
#include "file.h"
#include "wininfo.h"
#include "message.h"
#include "help.h"
#include "resource.h"
#include "keys.h"
#include "prm.h"
/*#include "fontwin.h"*/

#include "idd.h"
/* #include "sdmdefs.h"
#include "sdmver.h"
#include "sdm.h" included by el.h*/
#include "sdmtmpl.h"

#include "tmc.h"

#include "asgn2key.hs"
#include "asgn2mnu.hs"
#include "tabs.hs"
#include "char.hs"
#include "para.hs"
#include "abspos.hs"
#include "edstyle.hs"
#include "cust.hs"
#include "style.hs"
#include "spell.hs"

#include "recorder.hs"
#include "recorder.sdm"

#include "prm.h"
#include "fkp.h"

#include "screen.h"
#include "rareflag.h"


/* G L O B A L S */
	struct RAC vrac = { 
	0 	};


/* E X T E R N A L S */
extern HWND vhwndCBT;
extern struct SCI vsci;
extern struct STTB ** vhsttbFont;
extern BOOL vfDefineStyle;
extern struct PREF vpref;
extern struct MERR vmerr;
extern CHAR szEmpty [];
extern CHAR stEmpty [];
extern struct FLI vfli;
extern HWND vhwndApp;
extern HWND vhwndDeskTop;
extern BOOL vfRecording;
extern int docRecord;
extern CP cpRecord;
extern char rgchEop [];
extern struct MWD ** hmwdCur;
extern char rgchInsert[];
extern int ichInsert;
extern struct RAC vrac;
extern struct SEL selCur;
extern int docGlobalDot;
extern BOOL	fRecordMove;
extern BOOL	fRecordSize;
extern int	cdlScroll;
extern BOOL	fCdlScrollSet;
extern HWND vhwndStatLine;
extern HCURSOR  vhcRecorder;
extern BOOL vfRecordNext;
extern BOOL vfRecorderOOM;
extern int vcchRecord;
extern int vdocDotRecord;

#define cchMacLineRec	80

#define cchRecInsQuanta 32
char ** hrgchRecIns = hNil;
uns ichMacRecIns = 0, ichMaxRecIns = 0;



#define ValGetIag(hcab, iag) \
	(*(((int *) *(hcab)) + cwCabMin + (iag)))
#define LValGetIag(hcab, iag) \
	(* ((long *) (((int *) *(hcab)) + cwCabMin + (iag))))


/* F  R E C O R D  C A B */
/* Callback provided to SDM while we're recording things... */
/* NOTE: fDismiss is really a bcm when not called from a dialog box! */
/* %%Function:FRecordCab %%Owner:bradch */
BOOL FRecordCab(hcab, hid, tmc, fDismiss)
HCAB hcab;
WORD hid;
TMC tmc;
BOOL fDismiss; /* NOTE: doubles as bcm if no dialog is up */
{
	int ieldi, cb, ielfd, ielfdMac, iagFirstSimple, ielpidTarget;
	ELDI ** heldi;
	BOOL fComma = fFalse;
	BOOL fCheckRecordable = fTrue;
	BCM bcm;
	CP cpBreak;
	BOOL fInDlg;
	WORD hidT;
	CMB * pcmb;

	if (!vfRecording || vfRecorderOOM || fElActive)
		return fTrue;

	Assert (hcab != NULL);

	fInDlg = !(HdlgGetCur() == hdlgNull || 
			(hidT = HidOfDlg(HdlgGetCur())) == cxtRulerIconBar 
			|| hidT == cxtRibbonIconBar);

	switch (hid)
		{
	case IDDOpen:
	case IDDInsFile:
	case IDDCmpFile:
	case IDDInsPic:
		/* These are recorded after the dialog goes away! */
		if (fInDlg && hidT != IDDCatalog)
			return fTrue;
		bcm = (BCM) fDismiss;
		break;

	case IDDCustomize:
			{
			extern int vcBtnFldClicks;
		/* Must set this here because SDM won't let us keep the real
			value in this field for the dialog. */
			((CABCUSTOMIZE *) *hcab)->cBtnFldClicks = vcBtnFldClicks;
			bcm = bcmCustomize;
			}
		break;

	case IDDApplyStyle:
		if (fInDlg)
			return fTrue;
		bcm = bcmApplyStyleDlg;
		break;

	case IDDPrint:
		if (fInDlg)
			return fTrue;
		bcm = bcmPrint;
		break;

	case IDDTabs:
		if (vfDefineStyle || tmc == tmcOK)
			return fTrue;
		bcm = bcmTabs;
		break;

	case IDDStyleTabs:
		bcm = bcmStyTabs;
		hid = IDDTabs;
		break;

	case IDDDefineStyle:
		bcm = bcmStyles;
		if (tmc != tmcDSRename && tmc != tmcDSMerge && 
				tmc != tmcDSDefine && tmc != tmcDSDelete)
			return fTrue;
		break;

	case IDDStyleChar:
		bcm = bcmStyChar;
		hid = IDDCharacter;
		break;

	case IDDStylePara:
		bcm = bcmStyPara;
		hid = IDDParaLooks;
		break;

	case IDDStyleAbsPos:
		bcm = bcmStyPos;
		hid = IDDAbsPos;
		break;

	case IDDSearchChar:
		bcm = bcmSearchChar;
		hid = IDDCharacter;
		break;

	case IDDSearchPara:
		bcm = bcmSearchPara;
		hid = IDDParaLooks;
		break;

	case IDDReplaceChar:
		bcm = bcmReplaceChar;
		hid = IDDCharacter;
		break;

	case IDDReplacePara:
		bcm = bcmReplacePara;
		hid = IDDParaLooks;
		break;

	case IDDSaveAs:
		/* Special cased because this is recorded after the dialog
			goes away so that SummaryInfo can get recorded first! */
		bcm = imiSaveAs;
		break;

	case IDDCatalog:
		if (fInDlg)
			return fTrue;
		bcm = bcmCatalog;
		break;

	case IDDCharacter:
		if (fInDlg)
			return fTrue;
		bcm = bcmCharacter;
		break;

	case IDDIndex:
		if (fInDlg)
			return fTrue;
		bcm = bcmIndex;
		break;

	case IDDInsToc:
		if (fInDlg)
			return fTrue;
		bcm = imiTOC;
		break;

	case IDDSpeller:
/*		Convert the full name of the main dictionary to the two-letter
 *		form.
 */
		{
		CHAR szFileT[ichMaxFile];

		GetCabSz(hcab, szFileT, ichMaxFile, Iag(CABSPELLER, hszSplMDictBox));
		if (FSzLngFromSz(szFileT, ichMaxFile))
			AssertDo(FSetCabSz(hcab, szFileT, Iag(CABSPELLER,
						       hszSplMDictBox)));
		bcm = imiSpelling;
		}
		break;

	default:
		if (!fInDlg)
			{
			bcm = fDismiss;
			}
		else
			{
			pcmb = PcmbDlgCur();
			Assert(pcmb != NULL);
			bcm = pcmb->bcm;
			}
		}

	if (bcm == bcmChangeKeys && (/*!fDismiss ||*/ (tmc != tmcReset && 
			tmc != tmcAssignKey && tmc != tmcUnassignKey)))
		return fTrue;

	if (bcm == bcmAssignToMenu && tmc != tmcResetMenu && 
			tmc != tmcAssignMenu && tmc != tmcUnassignMenu)
		return fTrue;

	if (fCheckRecordable && !FRecordableBcm(bcm))
		return fTrue;

	if (tmc == tmcCancel)
		return fTrue;

	FlushVrac();

	if ((ieldi = IeldiFromHid(hid)) == iNil)
		return fTrue;

	cb = CbGetInfoIeldi(ieldi, hpNil);
	if ((heldi = HAllocateCb(cb)) == hNil)
		{
		ErrorEid(eidNoMemRecord, "FRecordCab");
		return fFalse;
		}

	CbGetInfoIeldi(ieldi, HpOfSbIb(sbDds, *HpOfSbIb(sbDds, heldi)));

	Assert(hid == (*heldi)->hid);

	ielfdMac = (*heldi)->celfd;

	iagFirstSimple = ((CABH *) (*hcab))->cwHandle;

	vcchRecord = 0;
	/* Fetch and record the name of this command. */
	FetchCm(bcm);
	RecordSt(vpsyFetch->stName);

	/* We don't want to record any parameters for the about box... */
	if (bcm == bcmAbout)
		ielfdMac = 0;

	ielpidTarget = 0;
	for (ielfd = 0; ielfd < ielfdMac; ielfd += 1)
		{
		ELK elk;
		ELV elv;
		int iag;
		char stBuf [256];

			/* BLOCK */
			{
			ELFD * pelfd;

			pelfd = &(*heldi)->rgelfd[ielfd];
			iag = pelfd->iag;
			Assert(iag == pelfd->tmc); /* union with tmc */
			elv = pelfd->elv;
			elk = pelfd->elk;
			}

		/* SDM SUCKS: really need way to tell if a value is ninch! */

		/* Skip push buttons that do not match tmc */
		if ((elv == elvNil && tmc != (TMC) iag) || 
				(elv != elvNil && elv != elvInt && elv != elvSd))
			{
			continue;
			}

		if (fComma)
			{
			RecordSt(StSharedKey(",",Comma));
			}
		else
			fComma = fTrue;

		RecordSt(StSharedKey(" ",MacroSpace));

		cpBreak = cpRecord;
		GetNameElk(elk, stBuf);
		RecordSt(StSharedKey(".",Period));
		RecordSt(stBuf);

		if (elv != elvNil)
			{
			RecordSt(StSharedKey("=",Equals));

			if (elv == elvInt || elv == elvListBoxInt)
				{
				RecordInt(ValGetIag(hcab, iag));
				}
			else  if (iag >= iagFirstSimple)
				{
				/* parsed item */
				RecordParsedItem(hid, hcab, iag, 
						ielpidTarget++);
				}
			else
				{
				GetCabSt(hcab, stBuf, sizeof (stBuf), iag);
				RecordQuotedSt(stBuf);
				}
			}

		if (vcchRecord >= cchMacLineRec)
			{
			CP cpSav;
			char ch;

			vcchRecord = (int) (cpRecord - cpBreak);
			cpSav = cpRecord;
			cpRecord = cpBreak;
			ch = '\\';
			InsertRgchRec(&ch, 1);
			RecordEop();
			cpRecord = cpSav + 1 + ccpEop;
			}
		}

	RecordEop();

	return fTrue;
}



/* G E N  S T A T E M E N T */
/* Given a command index, and assuming the global CAB is set up, generate
a macro statement to execute the command */
/* %%Function:GenStatement %%Owner:bradch */
GenStatement(bcm)
int bcm;
{
	SY  * psy;
	char rgchSy[cbMaxSy];
	if (vrac.racop != racopNil)
		FlushVrac();

#ifdef DRECORDER
	CommSzNum(SzShared("Genstatement: bcm="), bcm);
#endif
	/* If bcm is not recordable, or is recorded elsewhere, just return */
	if (!FRecordableBcm(bcm) || bcm == bcmColor || bcm == bcmFont ||
		 bcm == bcmFontSize || bcm == bcmSizeWnd || bcm == bcmMoveWnd)
		return;

	psy = PsyGetSyOfBsy(rgchSy, bcm);

	if (psy->mct == mctCmd)
		{
		RecordSt(psy->stName);
		if (FToggleBcm(bcm))
			{
			RecordSt(StSharedKey(" ",MacroSpace));
			RecordInt(FStatToggleBcm(bcm));
			}
		else  if (vfShiftKey)
			{
			switch (bcm)
				{
			case bcmBeginRow:
			case bcmEndRow:
			case bcmTopColumn:
			case bcmBottomColumn:
				RecordSt(StSharedKey(" 1", MacroSpaceOne));
				break;
				}
			}

		RecordEop();
		}
}


/* %%Function:FRecordableBcm %%Owner:bradch */
FRecordableBcm(bcm)
BCM bcm;
{
	/* These are either not recordable, or are recorded elsewhere */
	if (bcm == bcmNil || 
			bcm == imiRecorder || 
			bcm == bcmSplit || 
			bcm == bcmHelp ||
			bcm == bcmTraceMacro ||
			bcm == bcmStepMacro ||
			bcm == bcmContinueMacro ||
			bcm == bcmAnimateMacro ||
			bcm == bcmPauseRecorder ||
			bcm == bcmRepeat ||
			bcm == bcmIconBarMode ||
			bcm == bcmMenuMode ||
			bcm == bcmEditRuler ||
#ifdef DEBUG
			bcm == imiDebugFailures ||
#endif
			bcm == bcmShowVars)
		{
#ifdef DRECORDER
		CommSzNum(SzShared("bcm not recordable, bcm="), bcm);
#endif
		return fFalse;
		}

	return fTrue;
}


/* R E C O R D  I N T */
/* %%Function:RecordInt %%Owner:bradch */
RecordInt(i)
int i;
{
	char rgch[ichMaxNum];
	char * pch = &rgch[1];

	rgch[0] = CchIntToPpch(i, &pch);
	RecordSt(rgch);
}


/* R E C O R D  Q U O T E D  S T */
/* %%Function:RecordQuotedSt %%Owner:bradch */
RecordQuotedSt(st)
char * st;
{
	RecordQuotedRgch(st + 1, *st);
}


/* R E C O R D  Q U O T E D  R G C H */
/* %%Function:RecordQuotedRgch %%Owner:bradch */
RecordQuotedRgch(rgch, cch)
char rgch [];
int cch;
{
	int ich, ichStart;
	char * pch;
	char stQuote[3];

	stQuote[0] = 1;
	stQuote[1] = '"';
	stQuote[2] = '"';

	if (cch == 0)
		{
		InsertRgchRec(stQuote + 1, 2);
		return;
		}

	ichStart = 0;
	for (pch = rgch, ich = 0; ich < cch; ++ich, ++pch)
		{
		if (ich == cch - 1 || *pch == '"')
			{
			int cchInsert;

			if (ichStart > 0)
				RecordSt(StSharedKey("+",MacroPlus));

			Assert((1==1)==1);/* Assert fTrue == 1 */
			cchInsert = ich - ichStart + (*pch != '"');

			if (cchInsert > 0)
				{
				RecordSt(stQuote);
				InsertRgchRec(rgch + ichStart, cchInsert);
				RecordSt(stQuote);
				}

			if (*pch == '"')
				{
				if (ich > 0)
					RecordSt(StSharedKey("+",MacroPlus));

				RecordSt(StSharedKey("CHR$(34)",CHR34));

				ichStart = ich + 1;
				}
			}
		}
}


/* R E C O R D  S T */
/* %%Function:RecordSt %%Owner:bradch */
RecordSt(st)
char * st;
{
	if (vrac.racop != racopNil)
		FlushVrac();
	InsertRgchRec(&st[1], st[0]);
}


/* %%Function:RecordSz %%Owner:bradch */
RecordSz(sz)
char * sz;
{
	int cch;

	cch = CchSz(sz) - 1;
	if (vrac.racop != racopNil)
		FlushVrac();
	InsertRgchRec(sz, cch);
}



/* R E C O R D  E O P */
/* %%Function:RecordEop %%Owner:bradch */
RecordEop()
{
	InsertRgchRec(rgchEop, (int)ccpEop);
}


/* R E C O R D  C O M M A */
/* %%Function:RecordComma %%Owner:bradch */
RecordComma()
{
	char stComma[3];

	stComma[0] = 2;
	stComma[1] = ',';
	stComma[2] = ' ';
	RecordSt(stComma);
}



/* R E C O R D  I N S E R T */
/* %%Function:RecordInsert %%Owner:bradch */
RecordInsert(ichInsert, ptlv)
int ichInsert;
TLV * ptlv;
{
	int cchInsert;

	/* We don't handle para marks here, so if there is one, strip it. */
	cchInsert = ichInsert - ptlv->ichFill;

#ifdef DEBUG
		{
		int ich;

#ifdef CRLF
		Assert((int) cchEop == 2);
		Assert(rgchInsert[ichInsert - 2] != rgchEop[0] ||
				rgchInsert[ichInsert - 1] != rgchEop[1] ||
				rgchInsert[ichInsert - 2] == rgchEop[0] &&
				rgchInsert[ichInsert - 1] == rgchEop[1]);
#endif /* CRLF */

		for (ich = 0; ich < ichInsert - (int) ccpEop; ++ich)
			{
			Assert(rgchInsert[ich] != rgchEop[0]);
#ifdef CRLF
			Assert(rgchInsert[ich] != rgchEop[1]);
#endif /* CRLF */
			}
		}
#endif /* DEBUG */

	if (rgchInsert[ichInsert - (int) ccpEop] == rgchEop[0])
		cchInsert -= (int) ccpEop;

	if (cchInsert <= 0)
		return;

	FRecInsRgch(rgchInsert + ptlv->ichFill, cchInsert);
}


/* %%Function:FRecInsRgch %%Owner:bradch */
FRecInsRgch(pch, cch)
char * pch;
int cch;
{
	if (vrac.racop != racopNil && vrac.racop != racopInsert)
		FlushVrac();

	vrac.racop = racopInsert;

	if (hrgchRecIns == hNil && 
			(hrgchRecIns = HAllocateCb(cchRecInsQuanta + cch)) == hNil)
		{
		goto LNoMem;
		}

	if (ichMacRecIns + cch > ichMaxRecIns)
		{
		ichMaxRecIns = ichMacRecIns + cch + cchRecInsQuanta;
		if (!FChngSizeHCb(hrgchRecIns, ichMaxRecIns, fFalse))
			{
LNoMem:
			ErrorNoMemory(eidNoMemForRecord, "FRecInsRgch");
			return fFalse;
			}
		}

	bltb(pch, *hrgchRecIns + ichMacRecIns, cch);

	ichMacRecIns += cch;

	return fTrue;
}


/* %%Function:RecordBksp %%Owner:bradch */
RecordBksp(ptlv)
TLV *ptlv;
{
	/* Backspace is a special case */
	if (ichMacRecIns > 0)
		{
		if ((ichMacRecIns -= 1) == 0)
			{
			Assert(vrac.racop == racopInsert);
			vrac.racop = racopDelete;
			vrac.sty = styChar;
			vrac.dSty = 0;
			}
		}
	else
		{
		AddToVrac(racopDelete, styChar, -1);
		}

	ptlv->fRecordBksp = fTrue;
}



#define ichMaxInsStmt 60

/* %%Function:FlushRecIns %%Owner:bradch */
FlushRecIns()
{
	BOOL fEop;
	int ich, cch, cchBack, ichBreak;
	char * pch;
	char rgchBuf [ichMaxInsStmt + 1];

	if (ichMacRecIns == 0)
		return;

	AssertH(hrgchRecIns);

	for (ich = 0; ich < ichMacRecIns; ich += cch)
		{
		bltb(*hrgchRecIns + ich, rgchBuf, 
				cch = min(ichMaxInsStmt, ichMacRecIns - ich));
		rgchBuf[cch] = '\0';

		fEop = fFalse;
		ichBreak = iNil;
		for (pch = &rgchBuf[0]; *pch != '\0'; ++pch)
			{
			if (*pch == chEop)
				{
				fEop = fTrue;
				cch = pch - rgchBuf;
				break;
				}

			if (*pch <= ' ')
				ichBreak = ich;
			}

#define cchMaxInsStmtMrgn 10 /* TUNE */

		if (ichBreak != iNil && cch == ichMaxInsStmt && 
				++ichBreak < cch && ichBreak > cch - cchMaxInsStmtMrgn)
			{
			cch = ichBreak;
			}

		if (cch > 0)
			{
			RecordSt(StSharedKey("Insert ",CmdInsert));
			RecordQuotedRgch(rgchBuf, cch);
			RecordEop();
			}

		if (fEop)
			{
			RecordSt(StSharedKey("InsertPara",CmdInsertPara));
			RecordEop();
			ich += 1;
			}
		}

	ichMacRecIns = 0;
}



/* %%Function:RecordInsPara %%Owner:bradch */
RecordInsPara(ichInsert, ptlv)
int ichInsert;
TLV * ptlv;
{
	char rgch[1];
	extern BOOL vfInsertMode;
	Assert(!vfInsertMode);

	rgch[0] = chEop;
	FRecInsRgch(rgch, 1);

	if (vfRecordNext)
		EndRecordNextCmd();
}


csconst char csmpstyM1st [] [] =
{
	StKey("CharLeft",CmdCharLeft),
			StKey("WordLeft",CmdWordLeft),
			StKey("SentLeft",CmdSentLeft),
			StKey("ParaUp",CmdParaUp),
			StKey("LineUp",CmdLineUp),
			StKey("StartOfDocument",CmdStartOfDoc),
			StKey("StartOfWindow",CmdStartOfWindow),
			StKey("StartOfLine",CmdStartOfLine),
			StKey("PageUp",CmdPageUp),
			StKey("SectionUp",CmdSectionUp),
#define styM1stMid	10
			StKey("CharRight",CmdCharRight),
			StKey("WordRight",CmdWordRight),
			StKey("SentRight",CmdSentRight),
			StKey("ParaDown",CmdParaDown),
			StKey("LineDown",CmdLineDown),
			StKey("EndOfDocument",CmdEndOfDoc),
			StKey("EndOfWindow",CmdEndOfWindow),
			StKey("EndOfLine",CmdEndOfLine),
			StKey("PageDown",CmdPageDown),
			StKey("SectionDown",CmdSectionDown),
};




/* F L U S H  V R A C */
/* %%Function:FlushVrac %%Owner:bradch */
FlushVrac()
{
	int racop;

	racop = vrac.racop;
	vrac.racop = racopNil;

	switch (racop)
		{
	case racopNil:
		return;

	case racopInsert:
		FlushRecIns();
		return;

	case racopDelete:
		if (vrac.dSty != 0)
			{
			RecordSt(StSharedKey("EditClear ",CmdEditClear));
			RecordInt(vrac.dSty);
			RecordEop();
#ifdef DRECORDER
			CommSzNum(SzShared("$ Delete "), vrac.dSty);
#endif
			}
		return;

	case racopMove:
	case racopSelect:
			{
			int styM1;
			char stStmt [40];

			if (vrac.dSty == 0)
				return;

		/* HACK: to keep our array small, convert styScreenEnd
			to the unused (thus far) styWordNoSpace */
			if (vrac.sty == styScreenEnd)
				vrac.sty = styWordNoSpace;

			Assert(vrac.sty > 0 && vrac.sty < styM1stMid + 1);

			styM1 = vrac.sty - 1;
			if (vrac.dSty > 0)
				{
				styM1 += styM1stMid;
				}
			else
				{
				vrac.dSty = -vrac.dSty;
				}

			bltbx(csmpstyM1st[styM1], (char far *) stStmt, 
					sizeof (stStmt));
			stStmt[++stStmt[0]] = ' ';
			RecordSt(stStmt);

			if (vrac.sty == styDoc || vrac.sty == styLineEnd || 
					vrac.sty == styWordNoSpace)
				{
				if (racop == racopSelect)
					RecordSt(StSharedKey("1",NumeralOne));
				}
			else
				{
				RecordInt(vrac.dSty);

				if (racop == racopSelect)
					RecordSt(StSharedKey(", 1",CommaOne));
				}

			RecordEop();
			return;
			}

	case racopHScroll:
	case racopVScroll:
		DoRecScroll(racop, vrac.sty, vrac.dSty);
		return;

#ifdef DEBUG
	default:
		Assert(fFalse);
		return;
#endif /* DEBUG */
		}

	RecordInt(vrac.sty);

	if (vrac.dSty != 1)
		{
		RecordComma();
		RecordInt(vrac.dSty);
		}
	RecordEop();
}


/* A D D  T O  V R A C */
/* %%Function:AddToVrac %%Owner:bradch */
AddToVrac(racop, sty, dSty)
int racop, sty, dSty;
{
	if (racop != vrac.racop || sty != vrac.sty || 
		 (dSty < 0 && vrac.dSty > 0) || (dSty > 0 && vrac.dSty < 0))
		{
		FlushVrac();
		vrac.racop = racop;
		vrac.sty = sty;
		vrac.dSty = dSty;
		}
	else
		{
		/* styEndLine's do not accumulate! */
		if (racop != racopHScroll && racop != racopVScroll &&
				(vrac.sty == styLineEnd || vrac.sty == styScreenEnd ||
				vrac.sty == styDoc))
			{
			vrac.dSty = dSty;
			}
		else
			{
			vrac.dSty += dSty;
			}
		}
}


/* R E C O R D  D E L E T E */
/* %%Function:RecordDelete %%Owner:bradch */
RecordDelete(sty, dSty)
int sty;
int dSty;
{
	if (dSty == 0)
		return;

	AddToVrac(racopDelete, sty, dSty);

	if (vfRecordNext)
		EndRecordNextCmd();
}


/* R E C O R D  C U R S O R */
/* %%Function:RecordCursor %%Owner:bradch */
RecordCursor(sty, dSty, fSelect)
int sty;
int dSty;
{
	if (dSty == 0)
		return;

	AddToVrac(fSelect ? racopSelect : racopMove, sty, dSty);

	if (vfRecordNext)
		EndRecordNextCmd();
}




/* %%Function:RecordScroll %%Owner:bradch */
RecordScroll(wm, sb, pos)
int wm, sb, pos;
{
	BOOL fHoriz;

	switch (sb)
		{
	case SB_THUMBPOSITION:
		if (vrac.racop != racopNil)
			FlushVrac();
		RecordSt((fHoriz = wm == WM_HSCROLL) ?
				StSharedKey("HScroll ",CmdHScroll) :
				StSharedKey("VScroll ",CmdVScroll));
		RecordInt(NMultDiv(pos, 100, 
				fHoriz ? vsci.xpRightMax : dqMax));
		RecordEop();
		break;

	case SB_LINEUP:
	case SB_LINEDOWN:
		AddToVrac(wm == WM_HSCROLL ? racopHScroll : racopVScroll,
				sb, (wm==WM_HSCROLL || !fCdlScrollSet) ? 1 : cdlScroll);
		break;

	case SB_PAGEUP:
	case SB_PAGEDOWN:
		AddToVrac(wm == WM_HSCROLL ? racopHScroll : racopVScroll,
				sb, 1);
		/* break; */
		}

	if (vfRecordNext)
		EndRecordNextCmd();
}


#ifdef INTL

csconst char csrgstScroll [] [] =
	{
	StKey("HLine", CmdHLine),
	StKey("HPage", CmdHPage),
#define icsstVert 2
	StKey("VLine", CmdVLine),
	StKey("VPage", CmdVPage)
	};

#endif



/* %%Function:DoRecScroll %%Owner:bradch */
DoRecScroll(racop, sb, c)
int racop, sb, c;
{
	BOOL fUp;
	char * pch, * st;
	char stBuf [256];

#ifdef INTL

	{
	int icsst;

	if (sb != SB_LINEUP && sb != SB_LINEDOWN &&
			sb != SB_PAGEUP && sb != SB_PAGEDOWN)
		{
		return;
		}

	icsst = (racop == racopHScroll ? 0 : icsstVert) +
		(sb == SB_PAGEUP || sb == SB_PAGEDOWN);
	CopyCsSt(csrgstScroll[icsst], stBuf);
	pch = stBuf + 1 + stBuf[0];
	fUp = sb == SB_LINEUP || sb == SB_PAGEUP;
	}

#else

	pch = &stBuf[1];
	fUp = fFalse;

	switch (racop)
		{
#ifdef DEBUG
	default:
		Assert(fFalse);
		break;
#endif

	case racopHScroll:
		*pch++ = 'H';
		break;

	case racopVScroll:
		*pch++ = 'V';
		break;
		}

	switch (sb)
		{
	default:
		return;

	case SB_LINEUP:
		fUp = fTrue;
		/* FALL THROUGH */

	case SB_LINEDOWN:
		st = StSharedKey("Line",CmdLine);
		goto LBlt;

	case SB_PAGEUP:
		fUp = fTrue;
		/* FALL THROUGH */

	case SB_PAGEDOWN:
		st = StSharedKey("Page",CmdPage);
LBlt: /* Note: content of st is only valid within this block */
		bltb(st + 1, pch, st[0]);
		pch += st[0];
		break;
		}
#endif

	if (fUp)
		c = -c;

	if (c != 1)
		{
		*pch++ = ' ';
		CchIntToPpch(c, &pch);
		}

	stBuf[0] = pch - stBuf - 1;
	RecordSt(stBuf);
	RecordEop();
}



/* %%Function:InsertRgchRec %%Owner:bradch */
InsertRgchRec(pch, cch)
char * pch;
int cch;
{
	extern BOOL vfInsertMode;
	int doc;
	FC fc;
	BOOL fPap;
	struct CA caT;
	struct CHP chp;
	struct PAP pap;
	struct PAP papStd;
	int cchPapx, dcp;
	CP cpM1, cp;
	char papx [cchPapxMax+1];
	extern struct CA caPara;

	if (vfRecorderOOM)
		return;

	Assert(!vfInsertMode);

	vcchRecord += cch;

	if (vfRecordNext)
		{
		extern void ** vhmes; /* NOTE: really a MES ** */

		if (vhmes == hNil)
			{
LFail:
			Beep();
			return fFalse;
			}

		doc = DocEditOfMeiCur();
		Assert(doc != docNil);

		if (doc == selCur.doc) 
			{
			cp = selCur.cpFirst;
			Assert(selCur.cpLim >= cp);
			dcp = (int)(selCur.cpLim - cp);
			TurnOffSel(&selCur);
			}
		else
			 cp = PwwdWw(WwDisp(doc, wwNil, fFalse))->sels.cpFirst;

		/* If we're in a table for some reason, don't bother inserting.
		 * This is easier than trying to move to a valid place and restoring
		 * the selection correctly, etc. */
		if (FInTableDocCp(doc, cp))
			goto LFail;
		}
	else
		{
		doc = docRecord;
		cp = cpRecord;
		cpRecord += cch;
		}

	Assert(doc != docNil);
	if (fPap = (cch == (int) ccpEop && *pch == rgchEop[0]))
		StandardPap(&pap);

	StandardChp(&chp);

	if (FInsertRgch(doc, cp, pch, cch, &chp, fPap ? &pap : 0))
		cp += cch;
	else
		vfRecorderOOM = fTrue;

	if (vfRecordNext)
		{
		PdodDoc(doc)->fDirty = fTrue;

		if (doc == selCur.doc)  /* Macro doc has selCur. Restore it for benefit of other
										   commands relying on selCur. */
			{
			CP cpSelFirst = cp;
			CP cpSelLim = cpSelFirst + dcp;

#ifdef CRLF
			/* make sure we don't split a CR/LF pair */
			CachePara(doc, cpSelFirst);
			if (cpSelFirst == caPara.cpLim - 1)
				cpSelFirst -= 1;
			if (cpSelLim == caPara.cpLim - 1)
				cpSelLim -= 1;
#endif /* CRLF */
			
			Select(&selCur, cpSelFirst, cpSelLim);
			TurnOnSelCur();
			}
		else 
		   {
		  	struct SELS * psels;

		  	psels = &PwwdWw(WwDisp(doc, wwNil, fFalse))->sels;
	 		psels->cpFirst = psels->cpLim = cp;
		   }
		}
}


/* %%Function:RecordDocClose %%Owner:bradch */
RecordDocClose()
{
	if (vrac.racop != racopNil)
		FlushVrac();

	RecordSt(StSharedKey("DocClose",CmdDocClose));
	RecordEop();

	if (vfRecordNext)
		EndRecordNextCmd();
}


/* %%Function:RecordRulerTab %%Owner:bradch */
RecordRulerTab(dxaIns, dxaDel, tbd)
int dxaIns, dxaDel;
struct TBD tbd;
{
	CABTABS ** hcab, * pcab;
	char * pchT;
	char szPos [ichMaxNum + 1];

	if ((hcab = HcabAlloc(cabiCABTABS)) == hNil)
		return;

	pcab = *hcab;
	pcab->iAlignment = tbd.jc;
	pcab->iLeader = tbd.tlc;

	if (dxaDel != xaNil)
		{
		pchT = szPos;
		CchExpZa(&pchT, dxaDel, vpref.ut, ichMaxNum, fTrue);
		if (FSetCabSz(hcab, szPos, Iag(CABTABS, hszTabsPos)))
			FRecordCab(hcab, IDDTabs, tmcTabsClear, bcmTabs);
		else
			vfRecorderOOM = fTrue;
		}

	if (dxaIns != xaNil)
		{
		pchT = szPos;
		CchExpZa(&pchT, dxaIns, vpref.ut, ichMaxNum, fTrue);
		if (FSetCabSz(hcab, szPos, Iag(CABTABS, hszTabsPos)))
			FRecordCab(hcab, IDDTabs, tmcTabsSet, bcmTabs);
		else
			vfRecorderOOM = fTrue;
		}

	FreeCab(hcab);

	if (vfRecordNext)
		EndRecordNextCmd();
}
