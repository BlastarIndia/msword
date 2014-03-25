/* ****************************************************************************
**
**      COPYRIGHT (C) 1987 MICROSOFT
**
** ****************************************************************************
*
*  Module: Functions for Insert Field
*
**
** REVISIONS
**
** Date         Who Rel Ver     Remarks
**
** 10/15/87     yxy		new file
**
** ************************************************************************* */
#define NOMINMAX
#define NOSCROLL
#define NOSHOWWINDOW
#define NOREGION
#define NOVIRTUALKEYCODES
#define NOWH
#define NORASTEROPS
#define NOGDI
#define NOMETAFILE
#define NOBITMAP
#define NOWNDCLASS
#define NOBRUSH
#define NOWINOFFSETS
#define NONCMESSAGES
#define NOKEYSTATE
#define NOCLIPBOARD
#define NOHDC
#define NOCREATESTRUCT
#define NOTEXTMETRIC
#define NOGDICAPMASKS
#define NODRAWTEXT
#define NOSYSMETRICS
#define NOMENUS
#define NOMB
#define NOCOLOR
#define NOPEN
#define NOFONT
#define NOOPENFILE
#define NOMEMMGR
#define NORESOURCE
#define NOSYSCOMMANDS
#define NOICON
#define NOKANJI
#define NOSOUND
#define NOCOMM

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "doc.h"
#include "props.h"
#include "inter.h"
#include "debug.h"
#include "ch.h"
#include "sel.h"
#include "dlbenum.h"
#include "field.h"
#include "style.h"
#include "cmdtbl.h"
#include "opuscmd.h"

#include "ifld.h"
#include "insfield.h"


#include "idd.h"
#include "sdmdefs.h"
#include "sdmver.h"
#include "sdm.h"
#include "sdmtmpl.h"
#include "sdmparse.h"

#include "insfield.hs"
#include "insfield.sdm"


#ifdef PROTOTYPE
#include "insfield.cpt"
#endif /* PROTOTYPE */

extern struct SEL	selCur;

extern CHAR     szEmpty[];

extern CHAR		(**vhmpiststcp)[];
extern int		vfShowAllStd;
extern int		vstcBackup;
extern int		vdocStsh;
extern int		vistLbMac;

CHAR *PchKeywordFromSyntax();

int FAbstEnumDateInst();
int FAbstEnumTimeInst();
int FAbstEnumGlossInst();
int FAbstEnumInfoInst();
int FAbstEnumNumInst();
int FAbstEnumPNumInst();
int FAbstEnumBkmkInst();
int FAbstEnumStyleInst();
int FAbstEnumTOCInst();
int FAbstEnumIndexInst();
int FAbstEnumEqInst();
int FAbstEnumMacroInst();
int FAbstEnumPrintInst();


/* Define the order in which they are listed (in insfield.h) */
csconst FD	rgfd[] = rgfdDef;

csconst CHAR	stzApFldSwFormat[] = stzApFldSwFormatDef;

/*  %%Function:  CmdInsField  %%Owner:  bobz       */

CMD CmdInsField(pcmb)
CMB * pcmb;
{
	int tmc;
	int cmd = cmdOK;
	CABINSFIELD *pcabifld;

	if (pcmb->fDefaults)
		{
		pcabifld = (CABINSFIELD *) *pcmb->hcab;
		pcabifld->uFlt = 0; /* Select the first entry. */
		pcabifld->uInst = uNinch;
		if (!FSetCabSz(pcmb->hcab, szEmpty, Iag(CABINSFIELD, hszFld)))
			return cmdNoMemory;
		}

	if (pcmb->fDialog)
		{
		CHAR dlt [sizeof (dltInsField)];

		BltDlt(dltInsField, dlt);

		switch (TmcOurDoDlg(dlt, pcmb))
			{
#ifdef DEBUG
		default:
			Assert(fFalse);
			return cmdError;
#endif
		case tmcError:
			return cmdError;

		case tmcCancel:
			return cmdCancelled;

		case tmcOK:
			break;
			}
		}

	if (pcmb->fAction)
		{
		CHAR szUser [ichMaxBufDlg];

		GetCabSz(pcmb->hcab, szUser, ichMaxBufDlg,
				Iag(CABINSFIELD, hszFld));
		cmd = CmdInsFltSzAtSelCur(fltNil, szUser, imiInsField, 
				fTrue, fTrue, fTrue);
		PushInsSelCur();
		}

	return cmd;
}


/*  %%Function:  FDlgInsField  %%Owner:  bobz       */

BOOL FDlgInsField(dlm, tmc, wNew, wOld, wParam)
DLM dlm;
TMC tmc;
WORD wOld, wNew, wParam;
{
	CABINSFIELD *pcabifld;
	BOOL	(*pfnFAbstEnum)();
	int	ifd;
	HCAB	hcab;
	CHAR    sz[ichMaxBufDlg];

	switch (dlm)
		{
	case dlmInit:
		/* Set the initial text */
		hcab = HcabDlgCur();
		Assert (hcab);
		pcabifld = (CABINSFIELD *) *hcab;
		FillLBoxInst(pcabifld->uFlt, sz);
		break;

	case dlmTerm:
		if (tmc != tmcOK)
			break;

		if ((hcab = HcabFromDlg(fFalse)) == hcabNotFilled)
			return fFalse;
		if (hcab == hcabNull)
			{
LRetErr:

			/* sdm will take down the dialog */
			return (fTrue);
			}

		/* Can't do this after exitting the dialog, because
			(*pfnFAbstEnum)() requires the dialog to exist
			to function properly. */

		pcabifld = (CABINSFIELD *) *hcab;
		if (pcabifld->uInst != uNinchList)
			{
			ifd = pcabifld->uFlt;
			Assert(ifd != uNinchList && ifd < ifdMax);
			pfnFAbstEnum = rgfd[ifd].pfnFAbstEnum;
			Assert(pfnFAbstEnum != NULL);
			GetCabSz(hcab, sz, ichMaxBufDlg,
					Iag(CABINSFIELD, hszFld));
			pcabifld = *hcab;
			(*pfnFAbstEnum)(abmAppend, pcabifld->uInst,
					sz, ichMaxBufDlg);
			if (!FSetCabSz(hcab, sz, Iag(CABINSFIELD, hszFld)))
				goto LRetErr;
			}
		break;

	case dlmClick:
		switch (tmc)
			{
		case tmcIFldAdd:
				{
				int	i;

			/* Append instructions here. */
				i = ValGetTmc(tmcIFldInst);
				if (i != LB_ERR)
					{
					ifd = ValGetTmc(tmcIFldFlt);
					Assert(ifd < ifdMax);
					pfnFAbstEnum = rgfd[ifd].pfnFAbstEnum;
					Assert(pfnFAbstEnum != NULL);

					GetTmcText(tmcIFldTxt, sz, ichMaxBufDlg);
					(*pfnFAbstEnum)(abmAppend, i, sz, ichMaxBufDlg);
					SetTmcText(tmcIFldTxt, sz);
					SetTmcVal(tmcIFldInst, uNinchList);
					/* have to do this if was disabled */
					GrayButtonOnBlank(tmcIFldTxt, tmcOK);
					/* should not be blank or add would
					not be enabled
					*/
					Assert(FEnabledTmc(tmcOK));
					SetFocusTmc(tmcOK);
					EnableTmc(tmcIFldAdd, fFalse);
					}
				}
			return fFalse; /* so this will not get recorded yet */

		case tmcIFldInst:
			EnableTmc(tmcIFldAdd,
					ValGetTmc(tmcIFldInst) != LB_ERR);
			break;

		case tmcIFldFlt:
			ifd = ValGetTmc(tmcIFldFlt);
			if (ifd != uNinchList)
				{
				FillLBoxInst(ifd, sz);
				}
			else
				{
				EnableTmc(tmcIFldInst, fFalse);
				SetTmcText(tmcIFldSyntax, szEmpty);
				SetTmcText(tmcIFldHelp, szEmpty);
				}
			break;
			}
		break;

	case dlmChange:
		if (tmc == tmcIFldTxt)
			{
			GrayButtonOnBlank(tmcIFldTxt, tmcOK);
			break;
			}
		break;
		}
	return fTrue;
}



/* F i l l L B o x  I n s t */
/*  %%Function:  FillLBoxInst  %%Owner:  bobz       */

FillLBoxInst(ifd, sz)
int ifd;
CHAR  sz[];
{
	BOOL (*pfnFAbstEnum)();

	Assert(ifd < ifdMax);
	SetFldDescrTxt(ifd, tmcIFldFlt, tmcIFldSyntax,
			tmcIFldTxt, tmcIFldHelp);
	pfnFAbstEnum = rgfd[ifd].pfnFAbstEnum;
	if (pfnFAbstEnum == NULL ||
			(*pfnFAbstEnum)(abmEnumInit, 0, NULL, 0) == 0)
		{
		SetTmcVal(tmcIFldInst, uNinchList);
		StartListBoxUpdate(tmcIFldInst); /* empties box  */
		EndListBoxUpdate(tmcIFldInst);
		EnableTmc(tmcIFldInst, fFalse);
		EnableTmc(tmcIFldAdd, fFalse);
		}
	else
		{
		int	i;
		BOOL	fEnable;

		/* Fill the inst. list box here. */
		EnableTmc(tmcIFldInst, fTrue);	/* so scroll bars will be activated */
		if (pfnFAbstEnum != FAbstEnumMacroInst)
			StartListBoxUpdate(tmcIFldInst);
		for (i = 0;
				((*pfnFAbstEnum)(abmEnum, i, sz, ichMaxBufDlg));
				i++)
			{
			AddListBoxEntry(tmcIFldInst, sz);
			}
		if (pfnFAbstEnum != FAbstEnumMacroInst)
			EndListBoxUpdate(tmcIFldInst);
		i = CEntryListBoxTmc(tmcIFldInst);
		fEnable = (i != 0 && i != LB_ERR);
		EnableTmc(tmcIFldInst, fEnable);
		SetTmcVal(tmcIFldInst, uNinchList);
		EnableTmc(tmcIFldAdd, fFalse);
		}
}


/* P A R S E  F L D  L I S T */
/*  %%Function:  WListFld  %%Owner:  bobz       */

WORD WListFld(tmm, sz, isz, filler, tmc, wParam)
TMM tmm;
CHAR * sz;
int isz;
WORD filler;
TMC tmc;
WORD wParam;
{
	switch (tmm)
		{
	case tmmCount:
		return -1;

	case tmmText:
		if (isz < ifdMax)
			{
			bltbx((CHAR FAR *) &((rgfd[isz].stzName)[1]),
					(CHAR FAR *) sz,
					(int) ((rgfd[isz].stzName)[0]));

			return fTrue;
			}
		}

	return 0;
}


/* S E T  F L D  D E S C R  T X T */
/* Assumes the focus is in tmcFldFlt */
/*  %%Function:  SetFldDescrTxt  %%Owner:  bobz       */

SetFldDescrTxt(ifd, tmcFldFlt, tmcFldSyntax, tmcFldTxt, tmcFldHelp)
int	ifd;
int	tmcFldFlt;
int	tmcFldSyntax;
int	tmcFldTxt;
int	tmcFldHelp;
{
	CHAR	*pchPen;
	int	cch;
	CHAR	sz[cchSzFdMax];

	Assert(ifd >= 0);

	/* Syntax Text */
	bltbx((CHAR FAR *) (&(rgfd[ifd].stzSyntax)[1]),
			(CHAR FAR *) sz,
			(int) ((rgfd[ifd].stzSyntax)[0]));
	SetTmcText(tmcFldSyntax, sz);

	pchPen = PchKeywordFromSyntax(sz);
	*pchPen++ = chSpace;
	*pchPen++ = '\0';
	AnsiLower(sz);
	SetTmcText(tmcFldTxt, sz);
#ifdef HORRIBLEFLUSHING
	cch = CchSz(sz) - 2;
	TmcSetSel(tmcFldTxt, cch, cch);
	TmcSetFocus(tmcFldFlt);
	EnableTmc(tmcFldTxt, fTrue);
#endif /* HORRIBLEFLUSHING */	

	/* Set help text. */
	bltbx((CHAR FAR *) (&(rgfd[ifd].stzHelp)[1]),
			(CHAR FAR *) sz,
			(int) ((rgfd[ifd].stzHelp)[0]));
	SetTmcText(tmcFldHelp, sz);
}



/* ------------- Set of FAbstEnumX functions ---------------- */

/* string table handle shared by FAbstEnumX functions. */
struct STTB	**hsttbAbstEnum;
#ifdef DEBUG
struct STTB	**hsttbAbstEnumUsed;
#endif

csconst AEPIC	rgaepicDate[] = rgaepicDateDef;
csconst CHAR	stzDTPicSw[] = stzDTPicSwDef;

/*  %%Function:  FAbstEnumDateInst  %%Owner:  bobz       */

int FAbstEnumDateInst(abm, iae, sz, ichMax)
int abm, iae, ichMax;
CHAR sz[];
{
	CHAR	*pch;
	CHAR	szT[cchMaxPic + cchDTPicSwMax + 2];
	CHAR	szT1[cchMaxPic];

	Assert(abmMin <= abm && abm < abmMax);
	switch (abm)
		{
	case abmEnumInit:
		return iaepicDateMax + 1;
	case abmAppend:
		bltbx((CHAR FAR *) &stzDTPicSw[1], (CHAR FAR *) szT,
				(int) stzDTPicSw[0]);
		pch = szT + stzDTPicSw[0] - 1;
		GetTmcText(tmcIFldInst, pch, sizeof (szT) - stzDTPicSw[0]);
		if (*pch != '\0')
			{
			FieldLegalSz(pch, szT1, cchMaxPic + 2, fFalse);
			AppendSzWordSz(sz, szT, ichMax);
			}
		return (fTrue);
	case abmEnum:
		if (iae < iaepicDateMax)
			{
			if (iae != 0)
				{
				bltbx((CHAR FAR *) &(rgaepicDate[iae - 1].stzPic[1]),
						(CHAR FAR *) sz,
						(int) (rgaepicDate[iae - 1].stzPic[0]));
				}
			else
				{
				DefSzPicDTTmplt(fTrue, szT1, cchMaxPic);
				LocalSzPicDTTmplt(szT1, sz);
				}
			return (fTrue);
			}
		else  if (iae == iaepicDateMax)
			{
			Assert(ichMax >= cchMaxPic * 2 + 1);
			GetDefSzPicDT(sz);
			return fTrue;
			}
		return (fFalse);
		}
	Assert (fFalse);
}


csconst AEPIC	rgaepicTime[] = rgaepicTimeDef;

/*  %%Function:  FAbstEnumTimeInst  %%Owner:  bobz       */

int FAbstEnumTimeInst(abm, iae, sz, ichMax)
int abm, iae, ichMax;
CHAR sz[];
{
	CHAR	*pch;
	CHAR	szT[cchMaxPic + cchDTPicSwMax + 2];
	CHAR	szT1[cchMaxPic];

	Assert(abmMin <= abm && abm < abmMax);
	switch (abm)
		{
	case abmEnumInit:
		return iaepicTimeMax;
	case abmAppend:
		bltbx((CHAR FAR *) &stzDTPicSw[1], (CHAR FAR *) szT,
				(int) stzDTPicSw[0]);
		pch = szT + stzDTPicSw[0] - 1;
		GetListBoxEntry(tmcIFldInst, iae, pch, 
				sizeof (szT) - stzDTPicSw[0]);
		if (*pch != '\0')
			{
			FieldLegalSz(pch, szT1, cchMaxPic + 2, fFalse);
			AppendSzWordSz(sz, szT, ichMax);
			}
		return (fTrue);
	case abmEnum:
		if (iae < iaepicTimeMax)
			{
			bltbx((CHAR FAR *) &(rgaepicTime[iae].stzPic[1]),
					(CHAR FAR *) szT1,
					(int) (rgaepicTime[iae].stzPic[0]));
			LocalSzPicDTTmplt(szT1, sz);
			return (fTrue);
			}
		return (fFalse);
		}
	Assert (fFalse);
}


/*  %%Function:  FAbstEnumGlossInst  %%Owner:  bobz       */

int FAbstEnumGlossInst(abm, iae, sz, ichMax)
int abm, iae, ichMax;
CHAR sz[];
{
	CHAR	szT[cchGlsyMax], szT1[cchGlsyMax];

	Assert(abmMin <= abm && abm < abmMax);
	switch (abm)
		{
	case abmEnumInit:
		return 1;
	case abmEnum:
		FEnsureGlsyInited();
		FillTmcGlossary(tmcIFldInst, fFalse);
		return (fFalse);
	case abmAppend:
		GetListBoxEntry(tmcIFldInst, iae, szT, sizeof (szT));
		if (szT[0] != '\0')
			{
			FieldLegalSz(szT, szT1, cchGlsyMax, fFalse);
			AppendSzWordSz(sz, szT, ichMax);
			}
		return (fTrue);
		}
	Assert (fFalse);
}


csconst CHAR rgifdInfo[] = rgifdInfoDef;

/*  %%Function:  FAbstEnumInfoInst  %%Owner:  bobz       */

int FAbstEnumInfoInst(abm, iae, sz, ichMax)
int abm, iae, ichMax;
CHAR sz[];
{
	CHAR	szT[cchMaxFieldKeyword + 1];
	int	ifd;

	Assert(abmMin <= abm && abm < abmMax);

	ifd = rgifdInfo[iae];

	switch (abm)
		{
	case abmEnumInit:
		return iifdInfoMax;
	case abmEnum:
		if (iae < iifdInfoMax)
			{
			bltbx((CHAR FAR *) &(rgfd[ifd].stzName[1]),
					(CHAR FAR *) sz,
					(int) (rgfd[ifd].stzName[0]));
			return (fTrue);
			}
		return (fFalse);
	case abmAppend:
		if (iae >= iifdInfoMax)
			{
			/* Wimp out. */
			return fFalse;
			}

		bltbx((CHAR FAR *) &(rgfd[ifd].stzSyntax[1]),
				(CHAR FAR *) &szT[0], cchMaxFieldKeyword + 1);
		szT[cchMaxFieldKeyword + 1] = '\0';
		PchKeywordFromSyntax(&szT[0]);
		AnsiLower(szT);

		AppendSzWordSz(sz, szT, ichMax);
		return (fTrue);
		}
	Assert (fFalse);
}


csconst AEPIC	rgaepicNum[] = rgaepicNumDef;
csconst CHAR	stzNumPicSw[] = stzNumPicSwDef;

/*  %%Function:  FAbstEnumNumInst  %%Owner:  bobz       */

int FAbstEnumNumInst(abm, iae, sz, ichMax)
int abm, iae, ichMax;
CHAR sz[];
{
	CHAR	*pch;
	CHAR	szT[cchMaxPic + cchNumPicSwMax + 2];
	CHAR	szT1[cchMaxPic];

	Assert(abmMin <= abm && abm < abmMax);
	switch (abm)
		{
	case abmEnumInit:
		return iaepicNumMax;
	case abmAppend:
		bltbx((CHAR FAR *) &stzNumPicSw[1], (CHAR FAR *) szT,
				(int) stzNumPicSw[0]);
		pch = szT + stzNumPicSw[0] - 1;
		GetListBoxEntry(tmcIFldInst, iae, pch, 
				sizeof (szT) - stzNumPicSw[0]);
		if (*pch != '\0')
			{
			FieldLegalSz(pch, szT1, cchMaxPic + 2, fTrue);
			AppendSzWordSz(sz, szT, ichMax);
			}
		return (fTrue);
	case abmEnum:
		if (iae < iaepicNumMax)
			{
			bltbx((CHAR FAR *) &(rgaepicNum[iae].stzPic[1]),
					(CHAR FAR *) szT1,
					(int) (rgaepicNum[iae].stzPic[0]));
			LocalSzPicNumTmplt(szT1, sz);
			return (fTrue);
			}
		return (fFalse);
		}
	Assert (fFalse);
}


csconst AELP	rgaelpPNum[] = rgaelpPNumDef;

/*  %%Function:  FAbstEnumPNumInst  %%Owner:  bobz       */

int FAbstEnumPNumInst(abm, iae, sz, ichMax)
int abm, iae, ichMax;
CHAR sz[];
{
	CHAR *pch;
	int   cch, cchCopy;
	CHAR  szT[cchMaxPNumActual + cchApFldSwFormat];

	Assert(abmMin <= abm && abm < abmMax);
	switch (abm)
		{
	case abmEnumInit:
		return iaelpPNumMax;
	case abmEnum:
		if (iae < iaelpPNumMax)
			{
			bltbx((CHAR FAR *) &(rgaelpPNum[iae].stzInstName[1]),
					(CHAR FAR *) sz,
					(int) (rgaelpPNum[iae].stzInstName[0]));
			return fTrue;
			}
		return fFalse;
	case abmAppend:
		if (iae >= iaelpPNumMax)
			{
			/* Do nothing */
			return fTrue;
			}

		bltbx((CHAR FAR *) &(stzApFldSwFormat[1]),
				(CHAR FAR *) szT,
				(int) (stzApFldSwFormat[0]));
		pch = szT + stzApFldSwFormat[0] - 1;
		bltbx((CHAR FAR *) &(rgaelpPNum[iae].stzInstActual[1]),
				(CHAR FAR *) pch,
				(int) (rgaelpPNum[iae].stzInstActual[0]));

		AppendSzWordSz(sz, szT, ichMax);
		return (fTrue);
		}
	Assert (fFalse);
}


/*  %%Function:  FAbstEnumBkmkInst  %%Owner:  bobz       */

int FAbstEnumBkmkInst(abm, iae, sz, ichMax)
int abm, iae, ichMax;
CHAR sz[];
{
	int	ichMac;
	CHAR	HUGE *hpst, *pch;
	CHAR	szT[cchBkmkMax + 2], szT1[cchBkmkMax + 2];

	Assert(abmMin <= abm && abm < abmMax);

	switch (abm)
		{
	case abmEnumInit:
		hsttbAbstEnum = PdodMother(selCur.doc)->hsttbBkmk;
		Debug(hsttbAbstEnumUsed = hsttbAbstEnum);
		return ((hsttbAbstEnum == hNil) ? 0 : (*hsttbAbstEnum)->ibstMac);
	case abmAppend:
		GetListBoxEntry(tmcIFldInst, iae, szT, sizeof (szT));
		if (szT[0] != '\0')
			{
			FieldLegalSz(szT, szT1, cchBkmkMax + 2, fFalse);
			AppendSzWordSz(sz, szT, ichMax);
			}
		return (fTrue);
	case abmEnum:
		pch = &szT[0];
		Assert(hsttbAbstEnum != hNil &&
				hsttbAbstEnumUsed == hsttbAbstEnum);
		if (iae < (*hsttbAbstEnum)->ibstMac)
			{
			hpst = HpstFromSttb(hsttbAbstEnum, iae);
			ichMac = *hpst++;
			Assert(ichMac < cchBkmkMax);
			ichMac = min(ichMac, cchBkmkMax);
			bltbh(hpst, pch, ichMac);
			*(pch + ichMac) = '\0';

			bltbyte(szT, sz, ichMac + 1);
			return (fTrue);
			}
		hsttbAbstEnum = hNil;
		return (fFalse);
		}
	Assert (fFalse);
}


/*  %%Function:  FAbstEnumStyleInst  %%Owner:  bobz       */

int FAbstEnumStyleInst(abm, iae, sz, ichMax)
int abm, iae, ichMax;
CHAR sz[];
{
	int		ichMac;
	CHAR		*pst, *pch;
	int		stcp;
	int		stc;
	struct STSH	stsh;
	CHAR		szT[cchMaxStyle + 2], szT1[cchMaxStyle + 2];

	Assert(abmMin <= abm && abm < abmMax);

	switch (abm)
		{
	case abmEnumInit:
		/* Ensure vhmpiststcp is not in use. */
		Assert(vhmpiststcp == hNil);

		vhmpiststcp = HAllocateCw(cwMpiststcp);
		if (vhmpiststcp == hNil)
			{
			return 0;
			}
		else
			{
			vistLbMac = 0;
			SetVdocStsh(selCur.doc);
			vstcBackup = stcNil;
			vfShowAllStd = fFalse;
			GenLbStyleMapping();
			return vistLbMac;
			}
	case abmAppend:
		GetListBoxEntry(tmcIFldInst, iae, szT, sizeof (szT));
		if (szT[0] != '\0')
			{
			FieldLegalSz(szT, szT1, cchMaxStyle + 2, fFalse);
			AppendSzWordSz(sz, szT, ichMax);
			}
		return (fTrue);
	case abmEnum:
		if (vhmpiststcp != hNil && iae < vistLbMac)
			{
			pch = &szT[0];
			stcp = (**vhmpiststcp)[iae];
			RecordStshForDocNoExcp(vdocStsh, &stsh);
			stc = StcFromStcp(stcp, stsh.cstcStd);
			GenStyleNameForStcp(pch, stsh.hsttbName,
					stsh.cstcStd, stcp);
			Assert(*pch < cchMaxStyle);
			ichMac = *pch;
			StToSzInPlace(pch);

			bltbyte(szT, sz, ichMac + 1);
			return (fTrue);
			}

		/* Clean up the mess that we have created. */
		if (vhmpiststcp != hNil)
			{
			vfShowAllStd = fFalse;
			FreePh(&vhmpiststcp);
			}
		return (fFalse);
		}
	Assert (fFalse);
}


csconst AELP	rgaelpTOC[] = rgaelpTOCDef;

/*  %%Function:  FAbstEnumTOCInst  %%Owner:  bobz       */

int FAbstEnumTOCInst(abm, iae, sz, ichMax)
int abm, iae, ichMax;
CHAR sz[];
{
	CHAR	szT[cchMaxTOCActual];

	Assert(abmMin <= abm && abm < abmMax);
	switch (abm)
		{
	case abmEnumInit:
		return iaelpTOCMax;
	case abmEnum:
		if (iae < iaelpTOCMax)
			{
			bltbx((CHAR FAR *) &(rgaelpTOC[iae].stzInstName[1]),
					(CHAR FAR *) sz,
					(int) (rgaelpTOC[iae].stzInstName[0]));
			return fTrue;
			}
		return fFalse;
	case abmAppend:
		if (iae >= iaelpTOCMax)
			{
			/* Wimp out. */
			return fFalse;
			}

		bltbx((CHAR FAR *) &(rgaelpTOC[iae].stzInstActual[1]),
				(CHAR FAR *) szT,
				(int) (rgaelpTOC[iae].stzInstActual[0]));
		AppendSzWordSz(sz, szT, ichMax);
		return (fTrue);
		}
	Assert (fFalse);
}


csconst AELP	rgaelpIndex[] = rgaelpIndexDef;

/*  %%Function:  FAbstEnumIndexInst  %%Owner:  bobz       */

int FAbstEnumIndexInst(abm, iae, sz, ichMax)
int abm, iae, ichMax;
CHAR sz[];
{
	CHAR	szT[cchMaxIndexActual];

	Assert(abmMin <= abm && abm < abmMax);
	switch (abm)
		{
	case abmEnumInit:
		return iaelpIndexMax;
	case abmEnum:
		if (iae < iaelpIndexMax)
			{
			bltbx((CHAR FAR *) &(rgaelpIndex[iae].stzInstName[1]),
					(CHAR FAR *) sz,
					(int) (rgaelpIndex[iae].stzInstName[0]));
			return fTrue;
			}
		return fFalse;
	case abmAppend:
		if (iae >= iaelpIndexMax)
			{
			/* Wimp out. */
			return fFalse;
			}

		bltbx((CHAR FAR *) &(rgaelpIndex[iae].stzInstActual[1]),
				(CHAR FAR *) szT,
				(int) (rgaelpIndex[iae].stzInstActual[0]));
		AppendSzWordSz(sz, szT, ichMax);
		return (fTrue);
		}
	Assert (fFalse);
}


csconst AELP	rgaelpPrint[] = rgaelpPrintDef;

/*  %%Function:  FAbstEnumPrintInst  %%Owner:  bobz       */

int FAbstEnumPrintInst(abm, iae, sz, ichMax)
int abm, iae, ichMax;
CHAR sz[];
{
	CHAR	szT[cchMaxPrintActual];

	Assert(abmMin <= abm && abm < abmMax);
	switch (abm)
		{
	case abmEnumInit:
		return iaelpPrintMax;
	case abmEnum:
		if (iae < iaelpPrintMax)
			{
			bltbx((CHAR FAR *) &(rgaelpPrint[iae].stzInstName[1]),
					(CHAR FAR *) sz,
					(int) (rgaelpPrint[iae].stzInstName[0]));
			return fTrue;
			}
		return fFalse;
	case abmAppend:
		if (iae >= iaelpPrintMax)
			{
			/* Wimp out. */
			return fFalse;
			}

		bltbx((CHAR FAR *) &(rgaelpPrint[iae].stzInstActual[1]),
				(CHAR FAR *) szT,
				(int) (rgaelpPrint[iae].stzInstActual[0]));
		AppendSzWordSz(sz, szT, ichMax);
		return (fTrue);
		}
	Assert (fFalse);
}


csconst AELP	rgaelpEq[] = rgaelpEqDef;

/*  %%Function:  FAbstEnumEqInst  %%Owner:  bradch       */

int FAbstEnumEqInst(abm, iae, sz, ichMax)
int abm, iae, ichMax;
CHAR sz[];
{
	CHAR	szT[cchMaxEqActual];

	Assert(abmMin <= abm && abm < abmMax);
	switch (abm)
		{
	case abmEnumInit:
		return iaelpEqMax;
	case abmEnum:
		if (iae < iaelpEqMax)
			{
			bltbx((CHAR FAR *) &(rgaelpEq[iae].stzInstName[1]),
					(CHAR FAR *) sz,
					(int) (rgaelpEq[iae].stzInstName[0]));
			return fTrue;
			}
		return fFalse;
	case abmAppend:
		if (iae >= iaelpEqMax)
			{
			/* Wimp out. */
			return fFalse;
			}

		bltbx((CHAR FAR *) &(rgaelpEq[iae].stzInstActual[1]),
				(CHAR FAR *) szT,
				(int) (rgaelpEq[iae].stzInstActual[0]));
		AppendSzWordSz(sz, szT, ichMax);
		return (fTrue);
		}
	Assert (fFalse);
}


/*  %%Function:  FAbstEnumMacroInst  %%Owner:  bradch       */

int FAbstEnumMacroInst(abm, iae, sz, ichMax)
int abm, iae, ichMax;
CHAR sz[];
{
	int	cch;
	CHAR	szT[cchMaxSyName + 1];

	Assert(abmMin <= abm && abm < abmMax);
	switch (abm)
		{
	case abmEnumInit:
		return 1;
	case abmEnum:
		/* We only need to this once */
		StartLongOp();
		FillMacroListTmc(tmcIFldInst, mctAll);
		EndLongOp(fFalse);
		return (fFalse);
	case abmAppend:
		GetListBoxEntry(tmcIFldInst, iae, szT, sizeof (szT));
		if (szT[0] != '\0')
			{
			cch = CchSz(szT) - 1;
			szT[cch] = chSpace;
			szT[cch + 1] = '\0';
			AppendSzWordSz(sz, szT, ichMax);
			}
		return (fTrue);
		}
	Assert (fFalse);
}



/* ------------- Miscellaneous Functions ---------------- */

/*  %%Function:  PchKeywordFromSyntax  %%Owner:  bobz       */

CHAR *PchKeywordFromSyntax(sz)
CHAR *sz;
{
	CHAR	*pchLim, *pchEye, *pchPen;

	/* Derive the field keyword from syntax string. */
	pchLim = index(sz, chSpace);
	if (pchLim != NULL)
		{
		for (pchEye = pchPen = sz; pchEye < pchLim; pchEye++)
			{
			if (*pchEye != chOptBeg && *pchEye != chOptEnd)
				{
				*pchPen++ = *pchEye;
				}
			}
		}
	else
		{
		pchPen = sz + CchSz(sz) - 1;
		}
	*pchPen = '\0';
	return pchPen;
}


/*  %%Function:  AppendSzWordSz  %%Owner:  bobz       */

AppendSzWordSz(szWord, sz, ichMax)
CHAR	*szWord;
CHAR	*sz;
int	ichMax;
{
	int	cch, cchCopy;
	CHAR	*pch;

	cch = CchSz(szWord) - 1;
	if (cch >= ichMax)
		{
		/* It's full already.  no can do... */
		return;
		}
	pch = &szWord[cch - 1];
	if (!FWhite(*pch))
		{
		*(++pch) = chSpace;
		cch++;
		}
	pch++;
	cchCopy = CchSz(sz);
	cchCopy = min(cchCopy, ichMax - cch);
	bltbyte(sz, pch, cchCopy);
	szWord[ichMax - 1] = '\0';
}


/*  %%Function:  FieldLegalSz  %%Owner:  bobz       */

FieldLegalSz(szSrc, szT, ichMax, fQuoteIt)
CHAR	*szSrc;
CHAR	*szT;
int	ichMax;
BOOL	fQuoteIt;
{
	CHAR	*pchEye, *pchPen, *pchLim;
	int	cch;

	/* Reserve space for possible quotes. */
	pchLim = szT + ichMax - 2;

	for (pchEye = szSrc, pchPen = szT; pchPen < pchLim && *pchEye != '\0';
			pchEye++, pchPen++)
		{
		if (FWhite(*pchEye))
			{
			fQuoteIt = fTrue;
#ifdef CRLF
			if (*pchEye == chReturn || *pchEye == chEol)
#else
				if (*pchEye == chEol)
#endif /* CRLF */
					{
					*pchPen = chSpace;
					}
				else
					{
					*pchPen = *pchEye;
					}
			}
		else  if (*pchEye == chGroupExternal)
			{
			*pchPen++ = chFieldEscape;
			if (pchPen >= pchLim)
				{
				pchPen--;
				break;
				}
			*pchPen = chGroupExternal;
			}
		else  if (*pchEye == chFieldEscape)
			{
			*pchPen++ = chFieldEscape;
			if (pchPen >= pchLim)
				{
				pchPen--;
				break;
				}
			*pchPen = chFieldEscape;
			}
		else
			{
			*pchPen = *pchEye;
			}
		}
	*pchPen = '\0';
	cch = pchPen - szT;

	pchPen = szSrc;
	pchEye = szT;
	if (fQuoteIt)
		{
		*pchPen++ = chGroupExternal;
		}
	bltbyte(pchEye, pchPen, cch);
	pchPen += cch;
	if (fQuoteIt)
		{
		*pchPen++ = chGroupExternal;
		}
	*pchPen = '\0';
}


