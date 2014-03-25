/* ****************************************************************************
**
**      COPYRIGHT (C) 1986 MICROSOFT
**
** ****************************************************************************
*
*  Module: tabs.c module for Opus tab setting manipulation stuff
*
*  Functions included:
*
**
** REVISIONS
**
** Date         Who Rel Ver     Remarks
**
** ************************************************************************* */


#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOSYSMETRICS
#define NOICON
#define NOKEYSTATE
#define NORASTEROPS
#define NOSHOWWINDOW
#define NOBITMAP
#define NOBRUSH
#define NOCLIPBOARD
#define NOCOLOR
#define NOCREATESTRUCT
#define NODRAWTEXT
#define NOFONT
#define NOGDI
#define NOHDC
#define NOMB
#define NOMEMMGR
#define NOMETAFILE
#define NOMINMAX
#define NOOPENFILE
#define NOPEN
#define NOREGION
#define NOSCROLL
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOWNDCLASS
#define NOCOMM
#define NOKANJI

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "props.h"
#include "sel.h"
#include "debug.h"
#include "error.h"
#include "doc.h"
#include "disp.h"
#include "inter.h"
#include "wininfo.h"
#include "prm.h"
#include "cmd.h"
#include "ruler.h"

#include "idd.h"

#include "sdmdefs.h"
#include "sdmver.h"
#include "sdm.h"
#include "sdmtmpl.h"

#include "tabs.hs"
#include "tabs.sdm"


/* #define DBG */


#define cchGrpprlApplyTabs 4 + (3*itbdMax*sizeof(int)) + (itbdMax*sizeof(CHAR))
/* # of char that fits in the to-be-cleared static text list. */
/* Say the width of tmcTabsClList is x.  Then the following
	constant is: floor((x - 12) / 4)  where 4 is a width of the
	system font used by Windows in dialog layout.  12 is of course
	3 times 4 for an ellipsis. Note the static text is double high, so
	double x.  Now, Yoshi does not want to hear
	again that the tabs dialog does not put in a proper ellipsis
	in the "to-be-cleared" list. */
#define cchClListMax        41

typedef struct
{
	/* tdsd.itbdCur == itbdNil means the position info in the edit
		control could not be parsed to dxa.  tdsd.itbdCur == itbdNew
		means that it was parsed to dxa however, there is no corresponding
		tab entry in the list. */
	int         itbdCur;
	int         dxaTabCur;
	struct TBD  tbdCur;

	/* Local copy of the corresponding elements in pap. */
	int         itbdMac;
	int         rgdxaTab[itbdMax];
	CHAR        rgtbd[itbdMax];
	CHAR        rgfSet[itbdMax];

	/* List of tabs stops to be deleted. */
	BOOL        fClearAll;
	int         itbdClListMac;
	int         rgdxaClearTab[itbdMax];
	CHAR        rgfClear[itbdMax];

	/*  fTrue, if the above two lists have changed. */
	BOOL        fDirty;

	/* fTrue, if the tab setting has already been recorded */
	BOOL	fTabSetBefore;

	BOOL	fDlgChanged;
	/* The grpprl that represents all this (only valid near the end) */
	char stFGrpprl [cchGrpprlApplyTabs + 2]; /* +2 for cch & flag */
}       TDSD;   /* Tabs Dialog State Descriptor */

#define vptdsd	((TDSD *) pcmb->pv)

STATIC char ** hstFGrpprl = hNil; /* Kept around for Repeat command */

csconst char szClearAll[] = SzSharedKey("All",All);

#define CchMaxPos               15
#define DxaMinMove              (czaInch / 8)

/* ----- Local procedures ----- */
BOOL FDeleteTab();
BOOL FInsertTab();
int  ITbdFindApproxDxa();

/* ----- External Vars ----- */


extern BOOL         fElActive;
extern BOOL         vfRecording;
extern struct PREF  vpref;
extern struct MWD **hmwdCur;
extern int		wwCur;
extern int          vstcStyle;
extern int          vfDefineStyle;
extern struct SEL   selCur;
extern struct CHP	vchpGraySelCur;
extern struct PAP	vpapSelCur;
extern struct PAP	vpapGraySelCur;
extern struct MERR  vmerr;
extern CHAR         szEmpty[];
extern CHAR         stEmpty[];
extern CP	    cpRecord;


/* ----- Some Macroes ----- */
#define IFromJc(jc)       (jc)
#define IFromTlc(tlc)     (tlc)
#define JcFromI(i)        (i)
#define TlcFromI(i)       (i)

/* ----- The following macros are used to manage an insertion/removal  ----- */
/* ----- of an entry from a list maintained in tdsd.                   ----- */
#define ShiftIPlus1(rg, i, iMac, type) \
		bltbyte(&((rg)[(i)]), &((rg)[(i) + 1]), sizeof(type) * ((iMac) - (i)))
#define ShiftIMinus1(rg, i, iMac, type) \
		bltbyte(&((rg)[(i) + 1]), &((rg)[(i)]), sizeof(type) * ((iMac) - (i) - 1))

#define itbdNil          (-1)
#define itbdNew           (-2)
#define itbdBlank         (-3)
#define itbdTooLarge      (-4)

/*  %%Function:  TmcGosubTabs  %%Owner:  bobz       */

TmcGosubTabs()
{
	int tmc;
	TDSD tdsd;
	CMB cmb;

	if ((cmb.hcab = HcabAlloc(cabiCABTABS)) == hNil)
		return tmcError;

	cmb.cmm = cmmNormal;
	cmb.pv = &tdsd;
	cmb.bcm = bcmTabs;
	tmc = TmcTabs(&cmb);

	FreeCab(cmb.hcab);

	return tmc;
}


/*  %%Function:  CmdTabs  %%Owner:  bobz       */

CMD CmdTabs(pcmb)
CMB * pcmb;
{
	TDSD tdsd;
	int  tmc;

	vptdsd = &tdsd;
	tmc = TmcTabs(pcmb);

	return tmc == tmcOK ? cmdOK : tmc == tmcCancel ? cmdCancelled : cmdError;
}



/* ****
*
	Function:  TmcTabs
*  Author:    yxy
*  Copyright: Microsoft 1986
*  Date:      9/24/86
*
*  Description: Tabs menu command
*
*  Input:
*
*  Output:
*
*  Side Effects:
*       Modifies the content of tdsd, changes tab settings in the
*       current selection.
*
** ***/


/*  %%Function:  TmcTabs  %%Owner:  bobz       */

int TmcTabs(pcmb)
CMB * pcmb;
{
	char * stFGrpprl;
	BOOL fNotEmpty;

	Assert( vptdsd != NULL );
	stFGrpprl = vptdsd->stFGrpprl;

	stFGrpprl[0] = 0;

	if (pcmb->fRepeated)
		{
		if (hstFGrpprl == hNil || **hstFGrpprl == '\0')
			return tmcError;

		CopySt(*hstFGrpprl, stFGrpprl); /* For Repeat command */
		goto LSetTabs;
		}

	fNotEmpty = FInitTdsd(pcmb);

	/* set jc code from current ruler value if any.
	if not empty, tbdcur.jc will the the value from init or the
	ruler value, if ruler is up. If empty, we use the ruler value,
	if up or else jcLeft.
	*/

	if (hmwdCur != hNil && (*hmwdCur)->hwndRuler != hNil)
		{
		struct RSD **hrsd;

		hrsd = (struct RSD **)WRefFromHwndIb((*hmwdCur)->hwndRuler);
		vptdsd->tbdCur.jc = 
				(*hrsd)->jcTabCur;
		}
	else if (!fNotEmpty)
		vptdsd->tbdCur.jc = jcLeft;

	if (pcmb->fDefaults)
		{
		/* Pick up the first tab. */
		if (fNotEmpty)
			{
			/* Guaranteed not to have any gray value. */
			CABTABS * pcabtabs;
			char * pchT;
			char sz [ichMaxNum + 1];

			pchT = sz;

			pcabtabs = (CABTABS *) *pcmb->hcab;
			pcabtabs->sab = 0;
			pcabtabs->iAlignment = IFromJc(vptdsd->tbdCur.jc);
			pcabtabs->iLeader = IFromTlc(vptdsd->tbdCur.tlc);
			CchExpZa(&pchT, vptdsd->dxaTabCur, 
					vpref.ut, ichMaxNum, fTrue);
			FSetCabSz(pcmb->hcab, sz, Iag(CABTABS, hszTabsPos));
			}
		else
			{
			CABTABS * pcabtabs = (CABTABS *) *pcmb->hcab;
			pcabtabs->iAlignment = IFromJc(vptdsd->tbdCur.jc);
			pcabtabs->iLeader = IFromTlc(tlcNone);
			FSetCabSz(pcmb->hcab, szEmpty, 
					Iag(CABTABS, hszTabsPos));
			}
		/* fMemFail will be true if FSetCabSz fails */
		if (vmerr.fMemFail)
			return cmdNoMemory;
		}

	if (pcmb->fDialog)
		{
		char dlt [sizeof (dltTabs)];
		CP cpSave;

		BltDlt(dltTabs, dlt);
		cpSave=cpRecord;
		if (TmcOurDoDlg(dlt, pcmb) != tmcOK)
			{
			if (pcmb->tmc == tmcCancel) cpRecord=cpSave;
			return pcmb->tmc;
			}
		}

	if (pcmb->fAction)
		{
		struct CA ca;

		if (fElActive)
			{
			switch (pcmb->tmc)
				{
#ifdef DEBUG
			default:
				Assert(fFalse);
				return pcmb->tmc;
#endif

			case tmcOK:
			case tmcTabsSet:	/* Set tab */
				if (!FSetTab(pcmb, fTrue))
					return tmcError;
				break;

			case tmcTabsClear:	/* Clear (one) tab */
				KillTab(pcmb);
				break;

			case tmcTabsReset:	/* ClearAll tabs */
				KillAllTabs(pcmb);
				break;
				}
			}

		if (hmwdCur != hNil && (*hmwdCur)->hwndRuler != hNil)
			{
			int jc;
			CABTABS *pcabtabs = (CABTABS *) *pcmb->hcab;

			if ((jc = JcFromI(pcabtabs->iAlignment)) != uNinchRadio)
				UpdateRulerTab(jc, (*hmwdCur)->hwndRuler);
			}

		if (!vptdsd->fDirty)
			return tmcOK;

		stFGrpprl[0] = (CHAR) (1 + CchGenGrpprlTabs(pcmb, &stFGrpprl[2]));
		stFGrpprl[1] = (CHAR) (vptdsd->fClearAll);

LSetTabs:
		if (!vfDefineStyle)
			{
			/* Set up undo before you do anything. */
			PcaSet(&ca, selCur.doc,
					CpFirstSty(wwCur, selCur.doc, selCur.cpFirst, 
					styPara, fTrue),
					CpLimSty(wwCur, selCur.doc,selCur.cpLim, 
					styPara, fTrue));
			AssureLegalSel(&ca);
			if (!FSetUndoB1(bcmFormatting, uccFormat,&ca))
				return tmcCancel;
			}

		if (stFGrpprl[1])
			ClearAllTabs();

		if (stFGrpprl[0] > 1) /* Only if there's a usable grpprl. */
			{
			if (vfDefineStyle)
				{
				ApplyGrpprlToStshPrope(&stFGrpprl[2], 
						stFGrpprl[0] - 1,
						fFalse /* fChp */, 
						fTrue /* fUpdBanter */);
				}
			else
				{
				ApplyGrpprlSelCur(&stFGrpprl[2], 
						stFGrpprl[0] - 1, fFalse);
				}
			}

		if (!vfDefineStyle)
			SetUndoAfter(&ca);

		/* hstFGrpprl remembers the last tabs dialog setup for
			the repeat command */
		if (hstFGrpprl != hNil)
			FreeH(hstFGrpprl);
		hstFGrpprl = HstCreate(vptdsd->stFGrpprl);
		}

#ifdef DBG
	ShowTabs(pcmb);
#endif
	return tmcOK;
}


/*  %%Function:  FDlgTabs  %%Owner:  bobz       */

BOOL FDlgTabs(dlm, tmc, wNew, wOld, wParam)
DLM dlm;
TMC tmc;
WORD wNew, wOld, wParam;
{
	CMB * pcmb = PcmbDlgCur();
	HCAB hcab;
	char sz [256];

	switch (dlm)
		{
	case dlmInit:
		ChangeTabDlgState(pcmb);
		SetTmcText(tmcTabsClList, szEmpty);
		break;

	case dlmIdle:
		if (wNew /* cIdle */ == 0)
			return fTrue;  /* call FSdmDoIdle and keep idling */
		if (vptdsd->fDlgChanged)
			{
			ChangeTabDlgState(pcmb);
			vptdsd->fDlgChanged = fFalse;
			}
		return fFalse;  /* so we keep idling */

	case dlmChange:
		if (tmc == (tmcTabsPos & ~ftmcGrouped))
			{
			GrayButtonOnBlank(tmc, tmcTabsSet);
			GrayButtonOnBlank(tmc, tmcTabsClear);
			vptdsd->fDlgChanged = fTrue;
			}
		break;

	case dlmSetItmFocus:
	case tmcTabsLeft:
	case tmcTabsCenter:
	case tmcTabsRight:
	case tmcTabsDecimal:
	case tmcTabsNoFill:
	case tmcTabsDot:
	case tmcTabsMinus:
	case tmcTabsUScore:
		if (vptdsd->fDlgChanged)
			{
			ChangeTabDlgState(pcmb);
			vptdsd->fDlgChanged = fFalse;
			}
		break;

	case dlmClick:
		if (vptdsd->fDlgChanged)
			{
			ChangeTabDlgState(pcmb);
			vptdsd->fDlgChanged = fFalse;
			}
		if ((hcab = HcabFromDlg(fFalse)) == hcabNotFilled)
			return fFalse;
		if (hcab == hcabNull)
			{
			/* FFilterSdmMsg will take dialog down */
			return (fTrue);
			}

		switch (tmc)
			{
		case (tmcTabsPos & ~ftmcGrouped) + 1: /* list box part */
			vptdsd->fDlgChanged = wNew != uNinchList;
			break;

		case tmcTabsSet:
				{
				BOOL	fDlgChanged;

				if (fDlgChanged = FSetTab(pcmb, fFalse))
					{
					SetTmcVal((tmcTabsPos & ~ftmcGrouped) + 1, 
							uNinchList);
					if (vfRecording)
						{
						/* update cab to have units if recording */
						if ((hcab = HcabFromDlg(fFalse)) == hcabNull)
							{
							/* FFilterSdmMsg will take dialog down */
							return (fTrue);
							}
						Assert (hcab != hcabNotFilled);

						FRecordCab(pcmb->hcab, IDDTabs, 
								tmcTabsSet, fFalse);
						}
					}
				SetFocusTmc(tmcTabsPos);
				vptdsd->fDlgChanged = fDlgChanged;
				return fFalse;
				}

		case tmcTabsClear:
			KillTab(pcmb);
			SetFocusTmc(tmcTabsPos);
			vptdsd->fDlgChanged = fTrue;
			return fFalse;

		case tmcTabsReset:
			KillAllTabs(pcmb);
			SetFocusTmc(tmcTabsPos);
			vptdsd->fDlgChanged = fTrue;
			/* break; */
			}
		break;

	case dlmTerm:
		switch (tmc)
			{
		case tmcOK:
			if (vptdsd->fDlgChanged)
				{
				ChangeTabDlgState(pcmb);
				vptdsd->fDlgChanged = fFalse;
				}
			if ((hcab = HcabFromDlg(fFalse)) == hcabNotFilled)
				return fFalse;
			if (hcab == hcabNull)
				{
				/* dialog will be taken down without EndDlg */
				return (fTrue);
				}
			vptdsd->fTabSetBefore=fFalse;
			if (!FSetTab(pcmb, fTrue))
				return fFalse;

			if (!vptdsd->fTabSetBefore && vfRecording && vptdsd->itbdCur >= 0)
				{
				if ((hcab = HcabFromDlg(fFalse)) == hcabNull)
					{
					/* dialog will be taken down without EndDlg */
					return (fTrue);
					}
				Assert (hcab != hcabNotFilled);
				FRecordCab(pcmb->hcab, IDDTabs, 
						tmcTabsSet, fFalse);
				}
			/* break; */
			}
		/* break; */
		}

	return fTrue;
}


/*  %%Function:  WListTabs  %%Owner:  bobz       */

WORD WListTabs(tmm, str, isz, filler, tmc, wParam)
TMM	tmm;
char *	str;
int	isz;
WORD	filler;
TMC	tmc;
WORD	wParam;
{
	CMB * pcmb = PcmbDlgCur();
	char * pchT = str;

	switch (tmm)
		{
	case tmmCount:
		return -1;

	case tmmText:
		if (isz < vptdsd->itbdMac)
			{
			CchExpZa(&pchT, vptdsd->rgdxaTab[isz], 
					vpref.ut, ichMaxBufDlg, fTrue);
			return fTrue;
			}
		}

	return 0;
}


/* F I N I T T D S D */

/* It grabs tab list from either the currently selected
	paragraphes or the appropriate stsh entry,
	picks up the strictly matching tabs and puts them in TDSD.
	It will return fFalse if there's no tabs set in TDSD.
	Otherwise, fTrue. */

/*  %%Function:  FInitTdsd  %%Owner:  bobz       */

BOOL FInitTdsd(pcmb)
CMB * pcmb;
{
	struct CHP chp;
	struct PAP *ppap, pap;
	struct STSH stsh;

	SetBytes(vptdsd->rgfSet, fFalse, itbdMax);
	SetBytes(vptdsd->rgfClear, fFalse, itbdMax);

	/* From a given selection, bring in tab info from
		the current paragraph or the current style. */
	if (vfDefineStyle)
		{
		int docStsh = DocMother(selCur.doc);

		ppap = &pap;
		MapStc(PdodDoc(docStsh), vstcStyle, &chp, ppap);
		}
	else
		{
		if (selCur.fUpdatePap)
			{
			/* non-interruptible */
			FGetParaState(fFalse, /* fAll */ fFalse /* fAbortOk */ );
			}
		ppap = &vpapSelCur;
		}

	vptdsd->fDirty = fFalse;
	vptdsd->fDlgChanged = fFalse;
	vptdsd->fTabSetBefore=fFalse;

	vptdsd->fClearAll = fFalse;
	vptdsd->itbdClListMac = 0;

	if (!vfDefineStyle && vpapGraySelCur.itbdMac != 0)
		{
		/* The number of tabs differ.  List no tabs. */
		vptdsd->itbdMac = 0;
		}
	else
		{
		int          itbd, itbdMac;
		int         *rgdxaTabGray;
		CHAR        *rgtbdTabGray;
		int         *pdxaTab, *pdxaTabCopy;
		CHAR        *ptbd, *ptbdCopy;
		int          cTabs;

		rgdxaTabGray = vfDefineStyle ? NULL : &vpapGraySelCur.rgdxaTab[0];
		rgtbdTabGray = vfDefineStyle ? NULL : &vpapGraySelCur.rgtbd[0];
		pdxaTab      = &ppap->rgdxaTab[0];
		pdxaTabCopy  = &vptdsd->rgdxaTab[0];
		ptbd         = &ppap->rgtbd[0];
		ptbdCopy     = &vptdsd->rgtbd[0];

		itbdMac = vfDefineStyle ? ppap->itbdMac : vpapSelCur.itbdMac;
		for (itbd = 0, cTabs = 0; itbd < itbdMac; itbd++)
			{
			if (vfDefineStyle || rgdxaTabGray[itbd] == 0 && rgtbdTabGray[itbd] == 0)
				{
				/* It matches! */
				cTabs++;
				*(pdxaTabCopy++) = *pdxaTab;
				*(ptbdCopy++)    = *ptbd;
				}
			pdxaTab++;
			ptbd++;
			}

		vptdsd->itbdMac = cTabs;
		}

	if (vptdsd->itbdMac == 0) 
		{
		vptdsd->itbdCur = itbdNil;
		return fFalse;
		}
	else
		{
		vptdsd->itbdCur = 0;
		vptdsd->dxaTabCur = vptdsd->rgdxaTab[0];
		bltbyte(&vptdsd->rgtbd[0], (CHAR) &vptdsd->tbdCur, sizeof(struct TBD));
		return fTrue;
		}
}


/* S E T T D S D */

/* From the current position typed in the edit control, it
	sets up the current fields of vptdsd-> */
/*  %%Function:  SetTdsd  %%Owner:  bobz       */

SetTdsd(pcmb, fAllowBlank)
CMB * pcmb;
BOOL fAllowBlank;
{
	int     dxaTabPos;
	CHAR    sz[ichMaxBufDlg];
	int     cch;
	int itbd;
	BOOL    fOverflow;
	CHAR   *PchSkipSpacesPch();

	/* Check for a blank line in the edit control.  If blank
		line is allowed (only true when called upon tmcOK,)
		assign the special value itbdBlank to itbdCur to not
		this fact so that a proper action can be taken by
		the caller. */
	GetCabSz(pcmb->hcab, sz, ichMaxBufDlg - 1, Iag(CABTABS, hszTabsPos));
#ifdef DBG
	CommSz("TabCabSz: \""); 
	CommSz(sz); 
	CommSz("\"\r\n");
#endif /* DBG */
	cch = CchSz(sz) - 1;
	if (fAllowBlank &&
			(cch == 0 || (*PchSkipSpacesPch(sz) == '\0')))
		{
		vptdsd->itbdCur = itbdBlank;
		return;
		}

	if (!FZaFromSs(&dxaTabPos, sz, cch, vpref.ut, &fOverflow))
		{
		vptdsd->itbdCur = fOverflow ? itbdTooLarge : itbdNil;
		return;
		}

	for (itbd = 0; itbd < vptdsd->itbdMac &&
			!FCloseXa(dxaTabPos, vptdsd->rgdxaTab[itbd]); itbd++)
		;

	vptdsd->itbdCur    = (itbd == vptdsd->itbdMac) ? itbdNew : itbd;
	vptdsd->dxaTabCur  = dxaTabPos;
	vptdsd->tbdCur.jc  = JcFromI(((CABTABS *) *pcmb->hcab)->iAlignment);
	vptdsd->tbdCur.tlc = TlcFromI(((CABTABS *) *pcmb->hcab)->iLeader);
}


/* F  S E T  T A B */

/* Given the current setting in TDSD, FSetTab() will try to insert
	a tab with the current attributes in the Cur fields of TDSD in
	the list maintained, if possible.  If fAllowBlank is fTrue
	and the edit control is empty, then it unconditionally returns
	fTrue.  fAllowBlank is supposed to be fTrue only when this
	function is called on tmcOK.  */

/*  %%Function:  FSetTab  %%Owner:  bobz       */

FSetTab(pcmb, fAllowBlank)
CMB * pcmb;
BOOL fAllowBlank;
{
	int itbd;
	CHAR * pchT;
	BOOL fInsertString;
	CHAR sz [ichMaxNum + 1];

	fInsertString = fFalse;

	SetTdsd(pcmb, fAllowBlank);

	if (fAllowBlank && vptdsd->itbdCur == itbdBlank)
		return fTrue;

	/* vptdsd->itbdCur == itbdNil means the position info in the edit
		control could not be parsed to dxa.  vptdsd->itbdCur == itbdNew
		means that it was parsed to dxa however, there is no corresponding
		tab entry in the list. vptdsd->itbdCur == itbdTooLarge means
		the value typed in the edit control was too large for tab dxa
		value.  */
#ifdef DBG
	CommSzNum(SzShared("vptdsd->itbdCur = "), vptdsd->itbdCur);
#endif /* DBG */
	switch (vptdsd->itbdCur)
		{
	case itbdTooLarge:
		RangeError(-czaMax, czaMax, fTrue, vpref.ut);
		goto LErr;

	case itbdNil:
		ErrorEid(eidInvTabPos, " FSetTab");
LErr:
		if (pcmb->fDialog)
			{
			SetFocusTmc(tmcTabsPos);
			SetTmcTxs(tmcTabsPos, TxsOfFirstLim(0, ichLimLast));
			}

		return fFalse;

	case itbdNew:
		if (vptdsd->itbdMac == itbdMax)
			{
			ErrorEid(eidTooManyTabs, " FSetTab");

			if (pcmb->fDialog)
				SetTmcText(tmcTabsPos, szEmpty);
			goto LErr;
			}

		/* Insert a tab at the appropriate place here. */

		/* Search for the place to insert. */
		for (itbd = 0; itbd < vptdsd->itbdMac &&
#ifdef DEBUG
				!FCloseXa(vptdsd->dxaTabCur, vptdsd->rgdxaTab[itbd]) &&
#endif
				vptdsd->dxaTabCur > vptdsd->rgdxaTab[itbd]; itbd++)
			;

		/* This tab can not exist in the list, because if it 
		were FCloseXa of SetTdsd should have caught it. */
		Assert((itbd >= vptdsd->itbdMac) || 
				(vptdsd->dxaTabCur <= vptdsd->rgdxaTab[itbd]));

		ShiftIPlus1(vptdsd->rgdxaTab, itbd, vptdsd->itbdMac, int);
		ShiftIPlus1(vptdsd->rgtbd, itbd, vptdsd->itbdMac, CHAR);
		ShiftIPlus1(vptdsd->rgfSet, itbd, vptdsd->itbdMac, CHAR);

		vptdsd->itbdMac++;
		vptdsd->itbdCur = itbd;
		vptdsd->rgdxaTab[itbd] = vptdsd->dxaTabCur;
		vptdsd->rgfSet[itbd] = fTrue;
		fInsertString = fTrue;
		break;

	default:
		if (fAllowBlank && 
				((struct TBD)(vptdsd->rgtbd[vptdsd->itbdCur])).jc==
				vptdsd->tbdCur.jc &&
				((struct TBD)(vptdsd->rgtbd[vptdsd->itbdCur])).tlc==
				vptdsd->tbdCur.tlc)
			vptdsd->fTabSetBefore=fTrue;
		}

	bltbyte(&(vptdsd->tbdCur), &(vptdsd->rgtbd[vptdsd->itbdCur]),
			sizeof (struct TBD));

	pchT = &sz;
	CchExpZa(&pchT, vptdsd->dxaTabCur, vpref.ut, ichMaxNum, fTrue);

	if (pcmb->fDialog)
		{
		if (fInsertString)
			{
			InsertListBoxEntry(tmcTabsPos, sz,
					(vptdsd->itbdCur == vptdsd->itbdMac) ?
					uNinchList : vptdsd->itbdCur);
			}
		SetTmcText(tmcTabsPos, sz);
		SetTmcVal((tmcTabsPos & ~ftmcGrouped) + 1, vptdsd->itbdCur);
		}

	vptdsd->fDirty = fTrue;

	return fTrue;
}


/* C H A N G E T A B D L G S T A T E */

/* It will gray Clear, Set and OK pushbuttons, if the edit control
	becomes empty.  Also, it will update the radio button setting
	as the string in the edit control changes. */

/*  %%Function:  ChangeTabDlgState  %%Owner:  bobz       */

ChangeTabDlgState(pcmb)
CMB * pcmb;
{
	HCAB hcab;

	/* If Position is set, enable Clear, Set, OK button. */
	GrayButtonOnBlank(tmcTabsPos, tmcTabsClear);
	GrayButtonOnBlank(tmcTabsPos, tmcTabsSet);

	/* Depending on the state change, set up the radio buttons */
	if ((hcab = HcabFromDlg(fFalse)) == hcabNotFilled || hcab == hcabNull)
		return;

	SetTdsd(pcmb, fFalse);
	if (vptdsd->itbdCur == itbdNil || vptdsd->itbdCur == itbdNew ||
			vptdsd->itbdCur == itbdTooLarge)
		{

#ifdef NOTUSED
		/* if the ruler is up, we want that value to be used as the
		default jc value, so keep the value in the cab. bz 1/25/89
		*/
		if (JcFromI(ValGetTmc(tmcAlignment)) != jcLeft)
			{
			SetTmcVal(tmcAlignment, IFromJc(jcLeft));
			}

#endif /* NOTUSED */

		if (TlcFromI(ValGetTmc(tmcLeader)) != tlcNone)
			{
			SetTmcVal(tmcLeader, IFromTlc(tlcNone));
			}
		}
	else
		{
		if (((struct TBD)(vptdsd->rgtbd[vptdsd->itbdCur])).jc !=
				JcFromI(ValGetTmc(tmcAlignment)))
			{
			SetTmcVal(tmcAlignment,
					IFromJc(((struct TBD)(vptdsd->rgtbd[vptdsd->itbdCur])).jc));
			}

		if (((struct TBD)(vptdsd->rgtbd[vptdsd->itbdCur])).tlc !=
				TlcFromI(ValGetTmc(tmcLeader)))
			{
			SetTmcVal(tmcLeader,
					IFromTlc(((struct TBD)(vptdsd->rgtbd[vptdsd->itbdCur])).tlc));
			}
		}
}


/* K I L L T A B */

/* It adds the tab stop specified in the edit control to
	the "to be cleared" list. */
/*  %%Function:  KillTab  %%Owner:  bobz       */

KillTab(pcmb)
CMB * pcmb;
{
	int itbd;
	HCAB hcab;

	SetTdsd(pcmb, fFalse);

	/* vptdsd->itbdCur == itbdNil means the position info in the edit
		control could not be parsed to dxa.  vptdsd->itbdCur == itbdNew
		means that it was parsed to dxa however, there is no corresponding
		tab entry in the list. vptdsd->itbdCur == itbdTooLarge means
		the value typed in the edit control was too large for tab dxa
		value.  */
	if ((itbd = vptdsd->itbdCur) == itbdNil || itbd == itbdTooLarge)
		{
		ErrorEid((itbd == itbdNil) ? eidInvTabPos : eidTabPosTooLarge,
				" FSetTab");
		if (pcmb->fDialog)
			{
			SetFocusTmc(tmcTabsPos);
			SetTmcTxs(tmcTabsPos, TxsOfFirstLim(0, ichLimLast));
			}
		return;
		}

	if (vfRecording) 
		{
		char *pchT;
		char sz[ichMaxNum + 1];

		pchT = &sz;
		CchExpZa(&pchT, vptdsd->dxaTabCur, vpref.ut, ichMaxNum, fTrue);
		SetTmcText(tmcTabsPos, sz);
		SetTmcVal((tmcTabsPos & ~ftmcGrouped) + 1,vptdsd->itbdCur);
		if ((hcab = HcabFromDlg(fFalse)) == hcabNotFilled || hcab == hcabNull)
			return;

		FRecordCab(pcmb->hcab, IDDTabs, tmcTabsClear, fFalse);
		}

	if (itbd != itbdNew)
		{
		if (!FInsertClListDxa(pcmb, vptdsd->rgdxaTab[itbd]))
			{
			/* Could not add it to the "to be cleared" list,
				bag it. */
			return;
			}

		if (itbd != vptdsd->itbdMac - 1)
			{
			int itbdMac;

			itbdMac = vptdsd->itbdMac;
			ShiftIMinus1(vptdsd->rgdxaTab, itbd, itbdMac, int);
			ShiftIMinus1(vptdsd->rgtbd, itbd, itbdMac, CHAR);
			ShiftIMinus1(vptdsd->rgfSet, itbd,itbdMac, CHAR);
			}

		if (pcmb->fDialog)
			DeleteListBoxEntry(tmcTabsPos, itbd);

		vptdsd->itbdMac--;
		Assert(vptdsd->itbdMac >= 0);

		/* Update the next selection in the list box. */
		if (vptdsd->itbdMac == 0)
			{
			vptdsd->itbdCur = itbdNil;
			if (pcmb->fDialog)
				SetTmcText(tmcTabsPos, szEmpty);
			}
		else
			{
			CHAR    sz[ichMaxNum + 1];

			/* If there is no (itbd + 1)-st entry, set it to the
				last entry in the list. */
			if (vptdsd->itbdCur == vptdsd->itbdMac)
				{
				vptdsd->itbdCur = vptdsd->itbdMac - 1;
				}
			vptdsd->dxaTabCur  = vptdsd->rgdxaTab[vptdsd->itbdCur];
			vptdsd->tbdCur     = vptdsd->rgtbd[vptdsd->itbdCur];

			if (pcmb->fDialog)
				{
				GetListBoxEntry(tmcTabsPos, vptdsd->itbdCur, 
						sz, sizeof (sz));
				SetTmcText(tmcTabsPos, sz);
				SetTmcVal((tmcTabsPos & ~ftmcGrouped) + 1, 
						vptdsd->itbdCur);
				}
			}
		}
	else
		{
		FInsertClListDxa(pcmb, vptdsd->dxaTabCur);
		if (pcmb->fDialog)
			{
			SetTmcText(tmcTabsPos, szEmpty);
			SetTmcVal(tmcAlignment, IFromJc(jcLeft));
			SetTmcVal(tmcLeader, IFromTlc(tlcNone));
			}
		}
}


/* K I L L A L L T A B S */

/* Sets fClearAll flag in tdsd to avoid any more addition to the
	"Clear List."  It also clears out the tab list.   And clears
	all the dialog items. */

/*  %%Function:  KillAllTabs  %%Owner:  bobz       */

KillAllTabs(pcmb)
CMB * pcmb;
{
	if (!vptdsd->fClearAll)
		{
		vptdsd->fDirty = fTrue;
		}

	vptdsd->fClearAll = fTrue;
	vptdsd->itbdClListMac = vptdsd->itbdMac = 0;
	vptdsd->itbdCur = itbdNil;

	if (pcmb->fDialog)
		{
		RedisplayTmc(tmcTabsPos); /* Clear the list box, since mac is 0 */
		SetTmcText(tmcTabsPos, szEmpty);
		SetTmcVal(tmcAlignment, IFromJc(jcLeft));
		SetTmcVal(tmcLeader, IFromTlc(tlcNone));
		ShowNewClList(vptdsd->fClearAll, vptdsd->rgdxaClearTab,
				vptdsd->itbdClListMac);
		}
}


/* F I N S E R T C L L I S T D X A */

/* Adds a tab at dxa to the "Clear List," if it has not already
	been set to fClearAll. Updates the dialog box display accordingly.  */

/*  %%Function:  FInsertClListDxa  %%Owner:  bobz       */

BOOL FInsertClListDxa(pcmb, dxa)
CMB * pcmb;
int dxa;
{
	int     dxaClear;

	if (!vptdsd->fClearAll && vptdsd->itbdClListMac == itbdMax)
		{
		ErrorEid(eidTooManyClTabs, " FInsertClListDxa");
		if (pcmb->fDialog)
			{
			SetFocusTmc(tmcTabsPos);
			SetTmcTxs(tmcTabsPos, TxsOfFirstLim(0, ichLimLast));
			}
		return (fFalse);
		}
	else
		{
		int     itbd;

		if (vptdsd->fClearAll)
			{
			/* There's no need to adjust for this addition. */
			return (fTrue);
			}

		for (itbd = 0, dxaClear = vptdsd->rgdxaClearTab[0];
				itbd < vptdsd->itbdClListMac &&
				!FCloseXa(dxa, dxaClear)     &&
				dxa > dxaClear;
				dxaClear = vptdsd->rgdxaClearTab[++itbd]);

		/* If it is already in there, we just don't bother to add
			to it. */
		if (FCloseXa(dxa, vptdsd->rgdxaClearTab[itbd]))
			{
			return (fTrue);
			}
		else
			{
			ShiftIPlus1(vptdsd->rgdxaClearTab, itbd, vptdsd->itbdClListMac, int);
			ShiftIPlus1(vptdsd->rgfClear, itbd, vptdsd->itbdClListMac, CHAR);
			vptdsd->itbdClListMac++;
			vptdsd->rgdxaClearTab[itbd] = dxa;
			vptdsd->rgfClear[itbd] = fTrue;
			vptdsd->fDirty = fTrue;

			if (pcmb->fDialog)
				{
				ShowNewClList(vptdsd->fClearAll, vptdsd->rgdxaClearTab,
						vptdsd->itbdClListMac);
				}

			return (fTrue);
			}
		}
}


/* S H O W N E W C L L I S T */

/* Composes a sz from a given tab position list, to be displayed
	in the "Clear List" of the dialog box. */

/*  %%Function:  ShowNewClList  %%Owner:  bobz       */

ShowNewClList(fClearAll, rgdxa, itbdMac)
BOOL    fClearAll;
int     rgdxa[];
int     itbdMac;
{
	CHAR    sz[cchClListMax], szNum[ichMaxNum];
	CHAR    *pchCur, *pchMax, *pchT;
	int     itbd;
	int     cch;
	CHAR    chList;

	extern struct ITR vitr;

	chList = vitr.chList;
	if (fClearAll)
		{
		CopyCsSz(szClearAll, sz);
/*        PchFillPchId(sz, IDSTRClearAll, cchClListMax); */
		}
	else
		{
		pchMax = &sz[cchClListMax];
		for (itbd = 0, pchCur = &sz[0];
				itbd < itbdMac && pchCur < pchMax - 1; itbd++)
			{
			/* We use szNum here to avoid CchExpZa() wimping out
				on us, when pchMax - pchCur < ichMaxNum */
			pchT = &szNum[0];
			cch = CchExpZa(&pchT, rgdxa[itbd], vpref.ut, ichMaxNum, fTrue);
			if (pchCur + cch < pchMax)
				{
				bltbyte(szNum, pchCur, cch);
				pchCur += cch;
				}
			else
				{
				/* There was not enough space for this tabs stop. */
				goto LFillEllipse;
				}

			if (itbd < itbdMac - 1)
				{
				if (pchCur + 4 < pchMax)
					{
					*pchCur++ = chList;
					*pchCur++ = ' ';
					}
				else
					{
LFillEllipse:
					while (pchCur + 4 >= pchMax)
						{
						for (--pchCur; *pchCur != chList && pchCur != &sz[0];
								--pchCur);
						}
					*pchCur++ = '.';
					*pchCur++ = '.';
					*pchCur++ = '.';
					break;
					}
				}
			}
		*pchCur = '\0';
		}
	SetTmcText(tmcTabsClList, sz);
}


/* C L E A R A L L T A B S */

#define cchGrpprlClearTabs 4 + (2 * sizeof(int))

/*  %%Function:  ClearAllTabs  %%Owner:  bobz       */

ClearAllTabs()
{
	CHAR    grpprl[cchGrpprlClearTabs];
	int	    xa;

	grpprl[0] = sprmPChgTabs;
	grpprl[2] = 1;
	xa = 0;
	bltbyte(&xa, &grpprl[3], sizeof(int)); /* dxaDel == 0 */
	xa = czaMax;
	bltbyte(&xa, &grpprl[5], sizeof(int)); /* dxaClose == czaMax */
	grpprl[7] = 0;

	grpprl[1] = 2 + (2 * sizeof(int));

	if (vfDefineStyle)
		ApplyGrpprlToStshPrope(grpprl, cchGrpprlClearTabs, FALSE /* fChp */, TRUE /* fUpdBanter */);
	else
		ApplyGrpprlSelCur(grpprl, cchGrpprlClearTabs, fFalse);
}


/* A P P L Y T A B S */

/*
*  Builds a group prl reflecting the tab setting in the vptdsd->
*  Then it applies it to either the paragraphes in the current
*  selection or the appropriate style in the stsh.
*/
/*  %%Function:  CchGenGrpprlTabs  %%Owner:  bobz       */

CchGenGrpprlTabs(pcmb, grpprl)
CMB * pcmb;
CHAR    grpprl[];
{
	CHAR    *grpprlT;
	CHAR    *pchT;
	int      dgrpprl;

	grpprl[0] = sprmPChgTabs;

	grpprlT = &grpprl[2];

	if (!vptdsd->fClearAll)
		{
		/* Tabs to be cleared. */
		*grpprlT++ = (CHAR) vptdsd->itbdClListMac;
		bltbyte(vptdsd->rgdxaClearTab, grpprlT,
				(dgrpprl = vptdsd->itbdClListMac * sizeof(int)));
		grpprlT += dgrpprl;
		SetWords (grpprlT, dxaCloseMin, dgrpprl / sizeof(int));  /* rgdxaClose [i] == dxaCloseMin */
		grpprlT += dgrpprl;
		}
	else
		{
		*grpprlT++ = (CHAR) 0;
		}

	/* Tabs to be added. */
	*grpprlT++ = (CHAR) vptdsd->itbdMac;
	bltbyte(vptdsd->rgdxaTab, grpprlT,
			(dgrpprl = vptdsd->itbdMac * sizeof(int)));
	grpprlT += dgrpprl;
	bltbyte(vptdsd->rgtbd, grpprlT, (dgrpprl = vptdsd->itbdMac * sizeof(CHAR)));
	grpprlT += dgrpprl;

	grpprl[1] = grpprlT - &grpprl[2];

	return grpprlT - grpprl;
}



#ifdef DEBUG
#ifdef DBG
STATIC CHAR    *mpjcszJc[4] =
{
	" left   ", " center ", " right  ", " decimal"};


STATIC CHAR    *mptlcszTlc[4] =
{
	" None", " ....", " ----", " ____"};


/* S H O W T A B S */

/* Shows the tab list on COM1 */

/*  %%Function:  ShowTabs  %%Owner:  bobz       */

ShowTabs(pcmb)
CMB * pcmb;
{
	int     itbd;
	CHAR    sz[80];
	CHAR    *pchCur;

	if (!vptdsd->fDirty)
		{
		CommSz("No action was taken.\r\n");
		return;
		}

	CommSz("=====================================\r\n");
	CommSzNum("itbdMac: ", vptdsd->itbdMac);

	for (itbd = 0; itbd < vptdsd->itbdMac; itbd++)
		{
		struct TBD  tbd;

		pchCur = sz;
		bltbyte(&(vptdsd->rgtbd[itbd]), &tbd, sizeof(struct TBD));
		CchExpZa(&pchCur, vptdsd->rgdxaTab[itbd], vpref.ut, 80, fTrue);
		bltbyte(mpjcszJc[tbd.jc], pchCur, 8);
		pchCur += 8;
		bltbyte(mptlcszTlc[tbd.tlc], pchCur, 6);
		CommSz(sz);
		CommSz("\r\n");
		}
	CommSz("-------------------------------------\r\n");
	if (vptdsd->fClearAll)
		{
		CommSz("Kill Them ALL!!!\n\r");
		}
	else  
		{
		CommSzNum("itbdClListMac: ", vptdsd->itbdClListMac);
		for (itbd = 0; itbd < vptdsd->itbdClListMac; itbd++)
			{
			pchCur = sz;
			CchExpZa(&pchCur, vptdsd->rgdxaClearTab[itbd], vpref.ut, 80, fTrue);
			CommSzSz("On: ", sz);
			}
		}
	CommSz("=====================================\r\n");
}


#endif
#endif
