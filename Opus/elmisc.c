/* elmisc.c */


#include "word.h"
#include "inter.h"
#include "error.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "debug.h"
#include "doc.h"
#include "props.h"
#include "border.h"
#include "version.h"
/* #include "ch.h" */
#define ccpEop          2L	/* defines provided to substitute for */
#define chCRJ           11	/* ch.h include due to out of compile mem */
#define chReturn        13
#define chEop           10

extern long LWholeFromNum();

#include "fkp.h"
#include "file.h"
#include "cmdlook.h"
#include "prm.h"
#include "sel.h"
#include "cmdtbl.h"
#include "menu2.h"
#include "el.h"
#include "rerr.h"
#include "disp.h"
#include "wininfo.h"
#include "ibdefs.h"
#include "prompt.h"
#define RULER
#include "ruler.h"
#include "field.h"
#include "cmd.h"
#include "insert.h"
#include "error.h"
#include "idle.h"
#include "doslib.h"
#include "strtbl.h"
#include "macrocmd.h"
#include "help.h"

#include "sdmtmpl.h"
#include "idd.h"
#include "prompt.hs"
#include "prompt.sdm"

extern struct UAB vuab;
extern struct MERR vmerr;
extern char * PchSkipSpaces();
extern SB sbStrings;
extern struct ITR vitr;
extern struct PAP vpapFetch;
extern struct MWD ** hmwdCur;
extern HWND vhwndApp;
extern struct SEL selCur;
extern struct WWD ** hwwdCur;
extern int		wwCur;
extern BOOL vfBlockSel;
extern int docGlsy;
extern MUD ** vhmudUser;
extern KMP ** vhkmpUser;
extern char szApp [];
extern char szEmpty [];
extern char stEmpty [];
extern struct PREF vpref;
extern int vfElFunc;
extern int vcElParams;
extern int vfSysMenu;
extern int vssc;
extern RERR vrerr;
extern MES ** vhmes;
extern struct SEL selMacro;
extern int vcxtHelp;
extern BOOL          vfAwfulNoise;
extern int vfSeeSel;


BOOL vfDisableAutoMacros = fFalse;

/* Simple Cursor Movement / Selection Commands */

#define StyOfCty(cty)	((cty) & 0xff)
#define FFwdCty(cty)	(((cty) & (ctyRight | ctyDown | ctyEnd)) != 0)
#define FUpDownCty(cty)	(((cty) & (ctyUp | ctyDown)) != 0)
#define ctyLeft			0x8000
#define ctyRight		0x4000
#define ctyUp			0x2000
#define ctyDown			0x1000
#define ctyStart		0x0800
#define ctyEnd			0x0400

#define ctyLeftChar 		(styChar | ctyLeft)
#define ctyRightChar 		(styChar | ctyRight)
#define ctyLeftWord 		(styWord | ctyLeft)
#define ctyRightWord 		(styWord | ctyRight)
#define ctyLeftSent		(stySent | ctyLeft)
#define ctyRightSent		(stySent | ctyRight)
#define ctyUpPara 		(styPara | ctyLeft)
#define ctyDownPara 		(styPara | ctyRight)
#define ctyUpSect		(stySect | ctyUp)
#define ctyDownSect		(stySect | ctyDown)
#define ctyStartOfDoc 		(styDoc | ctyUp)
#define ctyEndOfDoc 		(styDoc | ctyDown)
#define ctyUpLine 		(styLine | ctyUp)
#define ctyDownLine 		(styLine | ctyDown)
#define ctyStartOfLine 		(styLineEnd | ctyLeft)
#define ctyEndOfLine 		(styLineEnd | ctyRight)
#define ctyUpWindow 		(styScreen | ctyUp)
#define ctyDownWindow 		(styScreen | ctyDown)
#define ctyStartOfWindow 	(styScreenEnd | ctyUp)
#define ctyEndOfWindow 		(styScreenEnd | ctyDown)



/*

SETBOOKMARK "name"

DELBOOKMARK "name"

COUNTBK()

BKNAME$(n)

CMPBK("name1", "name2")

NILBK("name")

EMPTYBK("name")

ENDOFBK "name1" [,"name2"]

STARTOFBK "name1" [,"name2"]

COPYBK "name1", "name2"

*/



/* FUTURE: add word and sentence sty? */
/* %%Function:GetCaOfBk %%Owner:bradch */
GetCaOfBk(pca, st)
struct CA * pca;
char * st;
{
	extern struct CA caPage;
	extern struct TCC vtcc;
	extern struct SELS rgselsInsert [];
	int ibkmk, sty;

	if (!PdodDoc(selCur.doc)->fMother ||
			!FSearchBookmark(selCur.doc, st, &pca->cpFirst, &pca->cpLim,
			&ibkmk, bmcDoc|bmcUser|bmcSelect))
		pca->doc = docNil;
	else
		pca->doc = selCur.doc;
}


/* %%Function:SetBk %%Owner:bradch */
SetBk(pca, st)
struct CA * pca;
char * st;
{
	char stBuf [cchBkmkMax];

	Assert(pca->doc == selCur.doc);
	Assert(pca->doc != docNil);

	CopySt(st, stBuf);
	if (!FLegalBkmkName(stBuf))
		{
		ErrorEid(eidInvalBkmkNam, "SetBk");
		return;
		}
	AssureLegalSel(pca);
	if (!FInsertStBkmk(pca, stBuf, NULL))
		RtError(rerrOutOfMemory);
}


/* %%Function:ElFNilBk %%Owner:bradch */
EL BOOL ElFNilBk(pstName)
char ** pstName;
{
	struct CA ca;

	Assert(selCur.doc != docNil);

	GetCaOfBk(&ca, *pstName);

	return (ca.doc == docNil) ? elFFalse : elFTrue;
}


/* %%Function:ElFEmptyBk %%Owner:bradch */
EL BOOL ElFEmptyBk(pstName)
char ** pstName;
{
	struct CA ca;

	Assert(selCur.doc != docNil);

	GetCaOfBk(&ca, *pstName);

	return (ca.doc != docNil && ca.cpFirst == ca.cpLim)
			? elFTrue : elFFalse;
}


/* %%Function:ElEndOfBk %%Owner:bradch */
EL void ElEndOfBk(pstBk1, pstBk2)
char ** pstBk1, ** pstBk2;
{
	struct CA ca;

	Assert(selCur.doc != docNil);

	if (pstBk2 == 0)
		pstBk2 = pstBk1;

	GetCaOfBk(&ca, *pstBk1);

	if (ca.doc == docNil)
		ErrorEid(eidNoBkmk, "ElEndOfBk");
	else
		{
		ca.cpFirst = ca.cpLim;
		SetBk(&ca, *pstBk2);
		}
}


/* %%Function:ElStartOfBk %%Owner:bradch */
EL void ElStartOfBk(pstBk1, pstBk2)
char ** pstBk1, ** pstBk2;
{
	struct CA ca;

	Assert(selCur.doc != docNil);

	if (pstBk2 == 0)
		pstBk2 = pstBk1;

	GetCaOfBk(&ca, *pstBk1);

	if (ca.doc == docNil)
		ErrorEid(eidNoBkmk, "ElStartOfBk");
	else
		{
		ca.cpLim = ca.cpFirst;
		SetBk(&ca, *pstBk2);
		}
}


/* %%Function:ElCopyBk %%Owner:bradch */
EL ElCopyBk(pstBk1, pstBk2)
char ** pstBk1, ** pstBk2;
{
	struct CA ca;

	Assert(selCur.doc != docNil);

	GetCaOfBk(&ca, *pstBk1);

	if (ca.doc == docNil)
		ErrorEid(eidNoBkmk, "ElCopyBk");
	else
		SetBk(&ca, *pstBk2);
}


/* %%Function:ElWCountBk %%Owner:bradch */
EL int ElWCountBk()
{
	struct STTB ** hsttb;

	Assert(selCur.doc != docNil);

	return ((hsttb = PdodMother(selCur.doc)->hsttbBkmk) != hNil ?
			(*hsttb)->ibstMac : 0);
}


/* %%Function:ElSdBkName %%Owner:bradch */
EL SD ElSdBkName(iBk)
int iBk;
{
	struct STTB ** hsttb;
	CHAR stT[cchMaxSt];

	Assert(selCur.doc != docNil);

	iBk -= 1;

	hsttb = PdodMother(selCur.doc)->hsttbBkmk;

	if (hsttb == hNil || iBk < 0 || iBk >= (*hsttb)->ibstMac)
		{
		RtError(rerrOutOfRange);
		/* NOT REACHED */
		Assert(fFalse);
		}

	GetStFromSttb(hsttb, iBk, stT);
	return SdCopySt(stT);
}


/* %%Function:ElWCmpBk %%Owner:bradch */
EL int ElWCmpBk(pstBk1, pstBk2)
char ** pstBk1, ** pstBk2;
{
	struct CA ca1, ca2;

	Assert(selCur.doc != docNil);

	GetCaOfBk(&ca1, *pstBk1);
	GetCaOfBk(&ca2, *pstBk2);

	if (ca1.doc == docNil || ca2.doc == docNil)
		{
		/* one or both are Nil */
/*		ErrorEid(eidNoBkmk, "ElWCmpBk");*/
		return 13;
		}

	Assert(ca1.doc == ca2.doc);

	if (!FNeRgch(&ca1, &ca2, sizeof (struct CA)))
		return 0;

	if (ca1.cpFirst > ca2.cpLim)
		return 1;

	if (ca1.cpLim < ca2.cpFirst)
		return 2;

	if (ca1.cpFirst == ca2.cpLim && ca1.cpLim > ca2.cpLim)
		return 11;

	if (ca1.cpFirst > ca2.cpFirst && ca1.cpLim > ca2.cpLim)
		return 3;

	if (ca1.cpFirst < ca2.cpFirst && ca1.cpLim == ca2.cpFirst)
		return 12;

	if (ca1.cpFirst < ca2.cpFirst && ca1.cpLim < ca2.cpLim)
		return 4;

	if (ca1.cpFirst < ca2.cpFirst && ca1.cpLim > ca2.cpLim)
		return 5;

	if (ca1.cpFirst > ca2.cpFirst && ca1.cpLim < ca2.cpLim)
		return 6;

	if (ca1.cpFirst == ca2.cpFirst && ca1.cpLim > ca2.cpLim)
		return 7;

	if (ca1.cpFirst == ca2.cpFirst && ca1.cpLim < ca2.cpLim)
		return 8;

	if (ca1.cpFirst < ca2.cpFirst && ca1.cpLim == ca2.cpLim)
		return 9;

	if (ca1.cpFirst > ca2.cpFirst && ca1.cpLim == ca2.cpLim)
		return 10;

	Assert(fFalse);

	return 13;
}


/* S D  F E T C H  P C A */
/*  Fetch the visible contents of pca into an SD.
*/
/* %%Function:SdFetchPca %%Owner:bradch */
SD SdFetchPca(pca)
struct CA *pca;
{
	struct FVB fvb;
	CHAR rgch[cchMaxSt];
	struct CR rgcr[10];
	SD sd;

	InitFvb(&fvb);
	InitFvbBufs(&fvb, rgch+1, cchMaxSt-1, rgcr, 10);
	fvb.doc = pca->doc;
	fvb.cpFirst = pca->cpFirst;
	fvb.cpLim = pca->cpLim;

	sd = 0;

	InvalVisiCache();
	do
		{
		FetchVisibleRgch(&fvb, selCur.ww, fFalse, fTrue);
		if (fvb.cch)
			{
			rgch[0] = fvb.cch;
			if (sd == 0)
				{
				sd = SdCopySt(rgch);
				Assert(sd != 0);
				Assert(sd != sdNil);
				}
			else
				AppendStToSd(rgch, sd);
			}
		} 
	while (fvb.fOverflow);

	return sd;
}



/* %%Function:ElSdBookmark %%Owner:bradch */
EL SD ElSdBookmark(pstName)
char ** pstName;
{
	struct CA ca;
	Assert(selCur.doc != docNil);
	GetCaOfBk(&ca, *pstName);
	if (ca.doc == docNil)
		return SdCopySt(stEmpty);
	else
		return SdFetchPca(&ca);
}





/****/



/* %%Function:ElSdGloss %%Owner:bradch */
EL SD ElSdGloss(pstName, fDot)
char ** pstName;
int fDot;
{
	int doc, iglsy, cch;
	SD sd;
	struct CA ca;
	char st [256];

	if (!FEnsureGlsyInited())
		RtError(rerrOutOfMemory);

	if ((doc = fDot ? DocGlsyLocal() : docGlsy) == docNil)
		{
		RtError(rerrModeError);
		}

	if ((iglsy = IglsyFromSt(doc, *pstName)) == iNil)
		{
		return SdCopySt(stEmpty);
		}
	else
		{
		CaFromIhdd(doc, iglsy, &ca);
		return SdFetchPca(&ca);
		}
}


/* %%Function:ElGloss %%Owner:bradch */
EL ElGloss(pstName, fDot)
char ** pstName;
int fDot;
{
	int doc, iglsy;

	if (!FEnsureGlsyInited())
		RtError(rerrOutOfMemory);

	if ((doc = fDot ? DocGlsyLocal() : docGlsy) == docNil)
		{
		RtError(rerrModeError);
		}

	if ((iglsy = IglsyFromSt(doc, *pstName)) == iNil)
		{
		ErrorEid(eidNoGlsy, "ElGloss");
		return;
		}

	CmdDoExpandGlsy(doc, iglsy, fTrue);
}


/* %%Function:ElWCountGloss %%Owner:bradch */
EL ElWCountGloss(fDot)
BOOL fDot;
{
	int doc;

	if (!FEnsureGlsyInited())
		RtError(rerrOutOfMemory);

	if ((doc = fDot ? DocGlsyLocal() : docGlsy) == docNil)
		return 0;

	return (*PdodDoc(doc)->hsttbGlsy)->ibstMac;
}


/* %%Function:ElSdGlossName %%Owner:bradch */
EL SD ElSdGlossName(iGloss, fDot)
int iGloss;
BOOL fDot;
{
	int doc;
	struct STTB ** hsttb;
	char rgch [cchGlsyMax];

	iGloss -= 1;

	if (!FEnsureGlsyInited())
		RtError(rerrOutOfMemory);

	if ((doc = fDot ? DocGlsyLocal() : docGlsy) == docNil)
		{
		RtError(rerrModeError);
		}

	Assert(doc != docNil);
	hsttb = PdodDoc(doc)->hsttbGlsy;

	if (iGloss < 0 || iGloss >= (*hsttb)->ibstMac)
		{
		RtError(rerrOutOfRange);
		/* NOT REACHED */
		Assert(fFalse);
		}

	return SdCopyHsttbIst(hsttb, iGloss);
}


/* Selection statements */

/* %%Function:ElBlockSel %%Owner:bradch */
EL ElBlockSel()
{
	CMB cmb;
	if (vfBlockSel) return;
	CmdBlkExtend(&cmb);
}


/* E L  I N S  P A R A */
/* %%Function:ElInsPara %%Owner:bradch */
EL ElInsPara()
{
#ifdef NOASM
	extern FC FcAppendRgchToFn();
#endif /* NOASM */
	extern struct CA caPara;
	extern int vstcpMapStc;
	extern int vdocFetch;
	extern CP vcpFetch;
	extern struct FKPDT vfkpdText;
	extern struct FKPD vfkpdPap;
	extern char rgchEop [];

	int doc, cchPapx;
	CP cp, dcpRM;
	FC fc;
	BOOL fDirtyBorders;
	struct CHP chp;
	struct CA caT;
	struct CA caPrl;
	struct PAP papStd, papNew;
	char rgbPrl [3];
	char papx [cchPapxMax+1];

	if (vssc != sscNil || PdodDoc(selCur.doc)->fLockForEdit)
		RtError(rerrModeError);

	if (!FDeleteCheck(fFalse, rpkText, NULL))
		return;

	/* FDeleteCheck() can result in vrerr set... */
	if (vrerr != rerrNil)
		RtError(vrerr);

	FSetUndoBefore(ucmTyping, uccInsert);

	/* Auto-delete */
	CmdAutoDelete(&caT);
	dcpRM = caT.cpLim - caT.cpFirst;

	doc = selCur.doc;
	cp = selCur.cpFirst;

	fDirtyBorders = fFalse;	/* until proven otherwise */
	CachePara(doc, cp);

	papNew = vpapFetch;

	if (papNew.fInTable && !FInTableDocCp(selCur.doc, cp))
		papNew.fInTable = fFalse;

	GetSelCurChp(fTrue /* fGetInsPtProps */);
	chp = selCur.chp;
	chp.fRMark = PdodMother(doc)->dop.fRevMarking;
	if (!FNewChpIns(doc, cp, &chp, stcNil))
		RtError(rerrOutOfMemory);


	MapStc(PdodDoc(doc), papNew.stc, 0, &papStd);
	cchPapx = CbGenPapxFromPap(&papx, &papNew, &papStd, fFalse);

	rgbPrl[1] = ((*(PdodMother(doc)->stsh.hplestcp))->dnstcp)
			[vstcpMapStc].stcNext;

	if (caPara.cpLim == cp + ccpEop && !(*hwwdCur)->fOutline && 
			rgbPrl[1] != papNew.stc)
		{
		fDirtyBorders = FBorderPap(&papNew);

		rgbPrl[0] = sprmPStc;
		PcaSet(&caPrl, doc, caPara.cpLim - ccpEop, 
				caPara.cpLim);

		FSetUndoDelAdd(&caPrl, fTrue);

		rgbPrl[2] = sprmCPlain;
		ApplyGrpprlCa(rgbPrl, 3, &caPrl);

		MapStc(PdodDoc(doc), rgbPrl[1], 0, &papStd);
		fDirtyBorders |= FBorderPap(&papStd);

		if (fDirtyBorders && caPara.cpLim < CpMacDoc(caPara.doc))
			InvalCp(PcaSet(&caT,caPara.doc,caPara.cpLim,caPara.cpLim+1));

		caPara.doc = docNil;
		vdocFetch = docNil;
		}

	fc = FcAppendRgchToFn(fnScratch, rgchEop, (int) ccpEop);
	if (!FReplace(PcaPoint(&caT, doc, cp), fnScratch, fc, (FC) ccpEop))
		RtError(rerrOutOfMemory);

	vcpFetch = cpMax; /* invalidate sequential fetch */

	caPara.doc = docNil;
	vdocFetch = docNil;

	if (!FAddRun(fnScratch, vfkpdText.fcLim, &papx, cchPapx, 
			&vfkpdPap, fTrue, fTrue, fTrue, fFalse))
		RtError(rerrOutOfMemory);

	cp += ccpEop;
	if ((*hwwdCur)->fOutline)
		UpdateHplcpadSingle(doc, cp);

	SetUndoAfter(PcaSet(&caT, selCur.doc, selCur.cpFirst, 
			selCur.cpFirst + ccpEop + dcpRM));

	SelectIns(&selCur, cp);
	SetAgain(bcmTyping);
	SetAgainCp(selCur.doc, caT.cpFirst, caT.cpFirst, caT.cpLim - dcpRM);

	DirtyDoc(doc);
	vfSeeSel = fTrue;
}


/* E L  I N S E R T */
/* %%Function:ElInsert %%Owner:bradch */
EL ElInsert(hpsd)
SD huge * hpsd;
{
	extern CP cpInsert;
	struct CA caRM;

	uns cch;
	CP cp, cpStart, dcpRM;
	char *pch, *pchMac;
	char huge *hpch, huge *hpchMac;
	struct CA caT;
	struct CHP chp;
	struct SELS selsBefore;
	char rgchT [256];

	if (vssc != sscNil || PdodDoc(selCur.doc)->fLockForEdit)
		ModeError();

	selsBefore = *((struct SELS *) &selCur);

	cpStart = selCur.cpFirst;

	if (!FDeleteCheck(fFalse, rpkText, NULL))
		return;

	/* FDeleteCheck() can result in vrerr set... */
	if (vrerr != rerrNil)
		RtError(vrerr);

	if (!FSetUndoBefore(ucmTyping, uccInsert))
		return;

	vuab.selsBefore = selsBefore;


	if ((cch = CchFromSd(*hpsd)) == 0)
		return;

	/* Auto-delete */
	CmdAutoDelete(&caRM);
	dcpRM = caRM.cpLim - caRM.cpFirst;

	GetSelCurChp (fFalse);
	chp = selCur.chp;
	chp.fRMark = PdodMother(selCur.doc)->dop.fRevMarking;

	cp = selCur.cpFirst;
	hpch = HpchFromSd(*hpsd);
	hpchMac = hpch + cch;

	while (hpch < hpchMac)
		{
		int cchT;
		pch = rgchT;
		pchMac = pch + sizeof(rgchT)-1;

		while (pch < pchMac && hpch < hpchMac)
			{
			int ch;

			switch (ch = *hpch++)
				{
			case chReturn:
				if (*hpch == chEop)
					hpch++;
				/* FALL THROUGH */

			case chEop:
				*pch++ = chReturn;
				*pch++ = chEop;
				break;

			default:
				*pch++ = ch;
				break;
				}
			}

		cchT = pch - rgchT;
		Assert(cchT >= 0 && cchT <= sizeof(rgchT));
		InsertMultiLineRgch(selCur.doc, cp, rgchT, cchT, &chp, 0);
		if (vmerr.fMemFail || vmerr.fDiskFail)
			RtError(rerrOutOfMemory);

		cp += cchT;
		}

	SelectIns(&selCur, cp);

	DirtyDoc(selCur.doc);
	cpInsert = cpNil;

	PushInsSelCur();

	SetUndoAfter(PcaSet(&caT, selCur.doc, cpStart, 
			selCur.cpFirst + dcpRM));
	SetAgain(bcmTyping);
	SetAgainCp(selCur.doc, cpStart, cpStart, caT.cpLim - dcpRM);

	if (hwwdCur != hNil && (*hwwdCur)->wk==wkMacro)
		{
		Assert(vhmes != hNil);

		if ((*vhmes)->fAnimate)
			/* macro being traced/stepped from its own window */
			{
			if (cpStart < selMacro.cpFirst) /* chars inserted before
			hilighted line */
				{
				selMacro.cpFirst += (pch - rgchT);
				selMacro.cpLim += (pch - rgchT);
				}
			else  if (cpStart < selMacro.cpLim) /* chars inserted within
				hilighted line */
				selMacro.cpLim += (pch - rgchT);

			}
		}
	vfSeeSel = fTrue;
}


/* E L  M S G  B O X */
/* %%Function:ElMsgBox %%Owner:bradch */
EL ElMsgBox(pstMsg, hpsd, num, mb)
char ** pstMsg;
SD huge * hpsd;
NUM num;
int mb;
{
	char ** pszTitle;
	WORD id;
	char szTitle [256];
	char szMsg [256];
	int cxtT;

	if (hpsd != 0)
		{
		int cch;

		if ((cch = CchFromSd(*hpsd)) > sizeof (szTitle) - 1)
			{
			RtError(rerrStringTooBig);
			/* NOT REACHED */
			Assert(fFalse);
			}
		bltbh(HpchFromSd(*hpsd), (char huge *) szTitle, cch);
		szTitle[cch] = '\0';
		pszTitle = szTitle;
		}
	else
		{
		pszTitle = szAppTitle;
		mb = LWholeFromNum(&num, fFalse);
		}

	if (mb >= 0)
		{
		if ((mb & MB_TYPEMASK) > MB_RETRYCANCEL || 
				(mb & MB_ICONMASK) > MB_ICONASTERISK || 
				(mb & MB_DEFMASK) > MB_DEFBUTTON3 ||
				(mb & MB_MODEMASK) > MB_SYSTEMMODAL ||
				(mb & (MB_TYPEMASK | MB_ICONMASK | 
				MB_DEFMASK | MB_MODEMASK)) != mb)
			{
			RtError(rerrIllegalFunctionCall);
			/* NOT REACHED */
			Assert(fFalse);
			}

		StToSz(*pstMsg, szMsg);

		cxtT = vcxtHelp;
		vcxtHelp = cxtMacroMsgBox;
		id = IdOurMessageBox(szMsg, pszTitle, mb);
		vcxtHelp = cxtT;

		switch (id)
			{
		case IDYES:
		case IDABORT:
		case IDOK:
			return -1;

		case IDCANCEL:
			return (mb & 0xf) == MB_YESNOCANCEL ? 1 : 0;

		case IDRETRY:
			return (mb & 0xf) == MB_ABORTRETRYIGNORE ? 0 : -1;

		case IDIGNORE:
			return 1;

		case IDNO:
			return 0;
			}
		}
	else
		{
		int pdc;

		if (vcElParams == 1)
			pdc = pdcDefault;
		else
			{
			mb = -mb;
			pdc = pdcmSL | pdcmImmed | ((mb & 8) ? 
					pdcmFullWidth : 0);
			switch (mb & ~0x8)
				{
			default:
				RtError(rerrIllegalFunctionCall);
				/* NOT REACHED */
				Assert(fFalse);

			case 0:
				pdc |= pdcmTimeout;
				break;

			case 1:
				pdc |= pdcmPermanent;
				break;

			case 2:
				pdc |= pdcmRestOnInput;
				break;
				}
			}

		CopySt(*pstMsg, szMsg);
		SetPromptSt(szMsg, pdc);

		return 0;
		}
}


/* %%Function:ElBeep %%Owner:bradch */
EL ElBeep(w)
int w;
{
	MessageBeep(w);
}


/* E L  S D  F E T C H */
/* Macro function to get the current selection as a string. */
/* %%Function:ElSdFetch %%Owner:bradch */
EL SD ElSdFetch()
{
	struct CA ca;

	ca = selCur.ca;
	Assert(selCur.doc != docNil);

	/* if selection is cells within a table or a block selection, just
	 * return null string so we don't return more than the user actually
	 * selected */
	if (selCur.fColumn)
		return SdCopySt(stEmpty);

	/* if selection is an insert point, fetch at least one character */
	if (selCur.fIns && ca.cpLim < CpMacDoc(selCur.doc))
		ca.cpLim = CpLimSty(wwCur, selCur.doc, ca.cpLim, styChar, 
				fFalse /* fEnd */ );

	return SdFetchPca(&ca);
}


csconst char csszExe [] = ".EXE";


/* %%Function:ElLaunch %%Owner:bradch */
EL ElLaunch(pst, sw)
char ** pst;
uns sw;
{
	int cch, ch;
	BOOL fDot;
	CHAR * pchS, * pchD;
	char szApp [256 + sizeof (csszExe)];

	if (vcElParams == 1)
		sw = SW_SHOWNORMAL;
	else  if (sw > 4)
		RtError(rerrIllegalFunctionCall);
	else  if (sw == 0)
		sw = SW_SHOWMINIMIZED;

	/* Before attempting to run the command, make sure the command name
		has an extension on it... */

	pchS = *pst;
	cch = *pchS++;
	pchD = szApp;
	fDot = fFalse;

	while (cch > 0 && (ch = *pchS) != ' ')
		{
		cch -= 1;
		if (ch == '.')
			fDot = fTrue;
		else  if (ch == '\\')
			fDot = fFalse;
		*pchD++ = ch;
		pchS += 1;
		}

	if (pchS - (*pst + 1) <= 4 || !fDot)
		{
		CopyCsSz(csszExe, pchD);
		pchD += sizeof (csszExe) - 1;
		}

	while (cch-- > 0)
		*pchD++ = *pchS++;

	*pchD = '\0';

	if (CchSz(szApp) > 128)
		RtError(rerrBadParam);

	if (!FOurRunApp(szApp, sw, eidNoMemRunApp, fFalse))
		ErrorEid(eidCantRunApp, "ElLaunch");
}


/* GENERAL WIN.INI ACCESS */


/*

		GetProfileString$( [app$]  key$ )
		SetProfileString [app$]  key$  value$
*/


/* %%Function:ElSdGetProfileString %%Owner:bradch */
EL SD ElSdGetProfileString(pst1, pst2)
char ** pst1, ** pst2;
{
	char * szAppName, * szKey;
	char szBuf [256];

	if (vcElParams == 1)
		{
		szAppName = szApp;
		szKey = *pst1;
		}
	else
		{
		if (**pst1 == 0)
			{
			szAppName = szApp;
			}
		else
			{
			szAppName = *pst1;
			StToSzInPlace(szAppName);
			}
		szKey = *pst2;
		}

	StToSzInPlace(szKey);

	GetProfileString((LPSTR) szAppName, (LPSTR) szKey, (LPSTR) szEmpty,
			(LPSTR) szBuf, sizeof (szBuf));

	return SdCopySz(szBuf);
}


/* %%Function:ElSetProfileString %%Owner:bradch */
EL ElSetProfileString(pst1, pst2, pst3)
char ** pst1, ** pst2, ** pst3;
{
	char szAppName[cchMaxSt], * szKey, * szValue;

	if (vcElParams == 2)
		{
		CchCopySz(szApp, szAppName);
		szKey = *pst1;
		szValue = *pst2;
		}
	else
		{
		if (**pst1 == 0)
			{
			CchCopySz(szApp, szAppName);
			}
		else
			{
			StToSz(*pst1, szAppName);
			}
		szKey = *pst2;
		szValue = *pst3;
		}

	StToSzInPlace(szKey);
	StToSzInPlace(szValue);

	WriteProfileString((LPSTR) szAppName, (LPSTR) szKey, (LPSTR) szValue);
	SendMessage(0xffff, WM_WININICHANGE, 0, (LPSTR)szAppName); /* HM */
}


/* %%Function:ElViewOutline %%Owner:bradch */
EL ElViewOutline(fOn)
BOOL fOn;
{
	int wk;
	Assert(hwwdCur != hNil);

	if (vfElFunc)
		{
		return ((*hwwdCur)->fOutline ? elFTrue : elFFalse);
		}
/* can't exec outline if in header/footnote/annotation pane */
	else  if (!((wk = (*hwwdCur)->wk) & wkSDoc))
		{
		if (vcElParams == 0 || !(*hwwdCur)->fOutline != !fOn)
			CmdOutline(NULL);
		}

	return 0; /* to keep the stack happy */
}


/* %%Function:ElViewPage %%Owner:bradch */
EL ElViewPage(fOn)
BOOL fOn;
{
	Assert(hwwdCur != hNil);

	if (vfElFunc)
		{
		return ((*hwwdCur)->wk == wkPage ? elFTrue : elFFalse);
		}
/* can't exec pageview if doc is locked or its mother doc is locked */
	else  if (!PdodDoc(selCur.doc)->fLockForEdit && 
			!PdodMother(selCur.doc)->fLockForEdit)
		{
/*                if (vcElParams == 0 || !((*hwwdCur)->wk == wkPage) != !fOn)*/
			{ 
			CmdPageView(vcElParams == 0 ? 2 : 
					(fOn ? fTrue : fFalse));
			}
		}


	return 0; /* to keep the stack happy */
}


/* %%Function:ElViewFullMenus %%Owner:bradch */
EL ElViewFullMenus()
{
	if (vfElFunc)
		return vpref.fShortMenus ? elFFalse : elFTrue;

	CmdLongMenus(NULL);

	return 0; /* keep the stack happy */
}


/* %%Function:ElViewShortMenus %%Owner:bradch */
EL ElViewShortMenus()
{
	if (vfElFunc)
		return vpref.fShortMenus ? elFTrue : elFFalse;

	CmdShortMenus(NULL);

	return 0; /* keep the stack happy */
}


/* %%Function:ElViewRibbon %%Owner:bradch */
EL ElViewRibbon(fOn)
BOOL fOn;
{
	extern BOOL vfRestoreRibbon;
	extern HWND vhwndRibbon;
	extern HWND vhwndAppIconBar;
	BOOL fOnCur;

	fOnCur = (hwwdCur == hNil ? vpref.fRibbon : 
			(vhwndRibbon != NULL || vfRestoreRibbon));

	if (vfElFunc)
		return fOnCur ? elFTrue : elFFalse;

	if (vcElParams == 0)
		fOn = !fOnCur;

	/* Negative logic used here to ensure true==1 */
	if (!fOnCur != !fOn)
		{
		if (hwwdCur == hNil)
			{
			vpref.fRibbon = Usgn(fOn);
			}
		else
			{
			if (!fOn && vfRestoreRibbon)
				{
				vfRestoreRibbon = fFalse;
				return 0;
				}

			if (fOn && vhwndAppIconBar != hNil)
				{
				vfRestoreRibbon = fTrue;
				return;
				}

			CmdRibbon(NULL);
			}
		}

	return 0; /* to keep the stack happy */
}


/* %%Function:ElViewRuler %%Owner:bradch */
EL ElViewRuler(fOn)
BOOL fOn;
{
	extern struct MWD ** hmwdCur;
	BOOL fOnCur;

	fOnCur = (hwwdCur == hNil ? vpref.fRuler : 
			(*hmwdCur)->hwndRuler != hNil);

	if (vfElFunc)
		return fOnCur ? elFTrue : elFFalse;

	if (vcElParams == 0)
		fOn = !fOnCur;

	if (!fOn != !fOnCur)
		{
		if (hmwdCur == hNil)
			{
			vpref.fRuler = Usgn(fOn);
			}
		/* can't exec ruler if in outline */
		else
			{
			if ((*hwwdCur)->fOutline)
				ModeError();

			CmdRuler(NULL);
			}
		}

	return 0; /* to keep the stack happy */
}


/* %%Function:ElViewStatusArea %%Owner:bradch */
EL ElViewStatusArea(fOn)
BOOL fOn;
{
	extern HWND vhwndStatLine;
	BOOL fOnCur;

	fOnCur = vhwndStatLine != NULL;
	if (vfElFunc)
		{
		return fOnCur ? elFTrue : elFFalse;
		}

	if (vcElParams == 0)
		fOn = !fOnCur;

	if (fOnCur != fOn)
		{
		if (!fOnCur != !fOn)
			CmdStatusArea(NULL);
		}

	return 0; /* to keep the stack happy */
}


/* %%Function:ElViewDraft %%Owner:bradch */
EL ElViewDraft(fOn)
BOOL fOn;
{
	if (vfElFunc)
		{
		return vpref.fDraftView ? elFTrue : elFFalse;
		}
	else
		{
		if (vcElParams == 0 || !fOn != !vpref.fDraftView)
			CmdDraftView(NULL);
		}

	return 0; /* to keep the stack happy */
}


/* %%Function:ElViewFieldCodes %%Owner:bradch */
EL ElViewFieldCodes(fOn)
BOOL fOn;
{
	if (vfElFunc)
		{
		return FFromIGrpf(fltgOther, (*hwwdCur)->grpfvisi.grpfShowResults) ? elFFalse : elFTrue;
		}
	else
		{
		if (vcElParams == 0)
			SetViewResultsFltg( wwCur, fltgOther, 
					!((*hwwdCur)->grpfvisi.grpfShowResults & (1<<fltgOther)));
		else
			SetViewResultsFltg( wwCur, fltgOther, fOn ? 0 : 1 );
		}
	MakeSelCurVisi(fTrue /*fForceBlockToIp*/);

	return 0; /* to keep the stack happy */
}


/* %%Function:ElViewFootnotes %%Owner:bradch */
EL ElViewFootnotes(fOn)
BOOL fOn;
{
	BOOL fOnCur;
	int ww;
	int wkIllegal = wkOutline+wkHdr+wkAtn;

	if (hmwdCur == hNil)
		return 0;
	Assert(hwwdCur);
	fOnCur = ((ww = (*hmwdCur)->wwLower) != wwNil &&
			PwwdWw(ww)->wk == wkFtn);

	if (vfElFunc)
		return fOnCur ? elFTrue : elFFalse;

/* can't exec View Footnote if in outline/header/annotation pane */
	if (!((*hwwdCur)->wk & wkIllegal) &&
			(vcElParams == 0 || (!fOn) != (!fOnCur)))
		CmdViewFootnote(NULL);

	return 0; /* to keep the stack happy */
}


/* %%Function:ElViewAnnotations %%Owner:bradch */
EL ElViewAnnotations(fOn)
BOOL fOn;
{
	BOOL fOnCur;
	int ww;
	int wkIllegal = wkOutline+wkFtn+wkHdr;

	if (hmwdCur == hNil)
		return 0;
	Assert(hwwdCur);
	fOnCur = ((ww = (*hmwdCur)->wwLower) != wwNil && 
			PwwdWw(ww)->wk == wkAtn);

	if (vfElFunc)
		return fOnCur ? elFTrue : elFFalse;

/* can't exec View Annotation if in outline/footnote/header pane */
	if (!((*hwwdCur)->wk & wkIllegal) &&
			(vcElParams == 0 || (!fOn) != (!fOnCur)))
		CmdViewAnnotation(NULL);

	return 0;
}


/* %%Function:ElDelete %%Owner:bradch */
EL ElDelete(dcp)
int dcp;
{
	int cty;

	if (vcElParams == 0)
		dcp = 1;
	else  if (dcp == 0)
		return;

	/* Special case for selection: delete the selection and remove
		one from count... */
	if (!selCur.fIns)
		{
		/* Backspace on a selection while auto-delete is 
			off does nothing but collape selection! */
		if (dcp < 0 && !vpref.fAutoDelete)
			{
			if (++dcp == 0)
				return;

			SelectIns(&selCur, selCur.cpFirst);
			}
		else
			{
			KeyCmdDel(fFalse);

			if ((dcp += (dcp < 0) ? 1 : -1) == 0)
				goto LDone;
			}
		}


	/* Now select the remainder and delete it all at once... */
	if (dcp > 0)
		{
		cty = ctyRightChar;
		}
	else
		{
		cty = ctyLeftChar;
		dcp = -dcp;
		}

	FCursCty(dcp, fTrue, cty);
	KeyCmdDel(fFalse);

LDone:
	DirtyDoc(selCur.doc);
}




/* Simple Cursor Movement / Selection Commands */

/* %%Function:ElLeftChar %%Owner:bradch */
EL ElLeftChar(cRepeat, fDrag)
{
	return FCursCty(cRepeat, fDrag, ctyLeftChar);
}


/* %%Function:ElRightChar %%Owner:bradch */
EL ElRightChar(cRepeat, fDrag)
{
	return FCursCty(cRepeat, fDrag, ctyRightChar);
}


/* %%Function:ElUpLine %%Owner:bradch */
EL ElUpLine(cRepeat, fDrag)
{
	return FCursCty(cRepeat, fDrag, ctyUpLine);
}


/* %%Function:ElDownLine %%Owner:bradch */
EL ElDownLine(cRepeat, fDrag)
{
	return FCursCty(cRepeat, fDrag, ctyDownLine);
}


/* %%Function:ElStartOfLine %%Owner:bradch */
EL ElStartOfLine(fDrag)
{
	return FCursCty(1, fDrag, ctyStartOfLine);
}


/* %%Function:ElEndOfLine %%Owner:bradch */
EL ElEndOfLine(fDrag)
{
	return FCursCty(1, fDrag, ctyEndOfLine);
}


/* %%Function:ElUpWindow %%Owner:bradch */
EL ElUpWindow(cRepeat, fDrag)
{
	return FCursCty(cRepeat, fDrag, ctyUpWindow);
}


/* %%Function:ElDownWindow %%Owner:bradch */
EL ElDownWindow(cRepeat, fDrag)
{
	return FCursCty(cRepeat, fDrag, ctyDownWindow);
}


/* %%Function:ElLeftWord %%Owner:bradch */
EL ElLeftWord(cRepeat, fDrag)
{
	return FCursCty(cRepeat, fDrag, ctyLeftWord);
}


/* %%Function:ElRightWord %%Owner:bradch */
EL ElRightWord(cRepeat, fDrag)
{
	return FCursCty(cRepeat, fDrag, ctyRightWord);
}


/* %%Function:ElUpPara %%Owner:bradch */
EL ElUpPara(cRepeat, fDrag)
{
	return FCursCty(cRepeat, fDrag, ctyUpPara);
}


/* %%Function:ElDownPara %%Owner:bradch */
EL ElDownPara(cRepeat, fDrag)
{
	return FCursCty(cRepeat, fDrag, ctyDownPara);
}


/* %%Function:ElLeftSent %%Owner:bradch */
EL ElLeftSent(cRepeat, fDrag)
{
	return FCursCty(cRepeat, fDrag, ctyLeftSent);
}


/* %%Function:ElRightSent %%Owner:bradch */
EL ElRightSent(cRepeat, fDrag)
{
	return FCursCty(cRepeat, fDrag, ctyRightSent);
}


/* %%Function:ElStartOfDoc %%Owner:bradch */
EL ElStartOfDoc(fDrag)
{
	return FCursCty(1, fDrag, ctyStartOfDoc);
}


/* %%Function:ElEndOfDoc %%Owner:bradch */
EL ElEndOfDoc(fDrag)
{
	return FCursCty(1, fDrag, ctyEndOfDoc);
}


/* %%Function:ElStartOfWindow %%Owner:bradch */
EL ElStartOfWindow(fDrag)
{
	return FCursCty(1, fDrag, ctyStartOfWindow);
}


/* %%Function:ElEndOfWindow %%Owner:bradch */
EL ElEndOfWindow(fDrag)
{
	return FCursCty(1, fDrag, ctyEndOfWindow);
}


extern CursBlockUpDown(), CursUpDown();
extern CursBlockLeftRight(), CursLeftRight();

csconst PFN rgpfnCurs [] = 
{
	CursBlockLeftRight, 
			CursLeftRight,
			CursBlockUpDown, 
			CursUpDown
};


/* %%Function:FCursCty %%Owner:bradch */
FCursCty(cRepeat, fDrag, cty)
{
	extern BOOL vfExtendSel;
	BOOL fUp;
	struct CA caSav;
	int dxpSave, dxpBef;
	struct SEL *psel;
	BOOL fBlLR = vfBlockSel && !FUpDownCty(cty);

	if (cRepeat == 0)
		cRepeat = 1;

	ClearInsertLine(PselActive());

	fDrag |= vfExtendSel;

	caSav = PselActive()->ca;

	if (fBlLR)
		{
		psel = PselActive();
		dxpBef = dxpSave = psel->xpLim - psel->xpFirst;
		}

	if (fUp = cRepeat < 0)
		cRepeat = -cRepeat;

	while (cRepeat-- > 0)
		{
		struct SELS selsT;
		int dxpAft;

		if (!fBlLR)
			selsT = *(struct SELS *) PselActive();

		/* This may look funny, but it replaces 30 lines! */
		(*rgpfnCurs[FUpDownCty(cty) * 2 + !vfBlockSel])
				(StyOfCty(cty), FFwdCty(cty) ? 1 : -1, fDrag);

		if (fBlLR)
			{
			psel = PselActive();
			dxpAft = psel->xpLim - psel->xpFirst;
			if (dxpBef == dxpAft)
				break;
			else
				{
				dxpAft = dxpBef;
				continue;
				}
			}

		if (!FNeRgch(PselActive(), &selsT, sizeof (struct SELS)))
			break;
		}

	return (fBlLR ? (PselActive()->xpLim - PselActive()->xpFirst != dxpSave)		      : FNeRgch(&PselActive()->ca, &caSav,
			 sizeof (struct CA)));
}


/* %%Function:ElViewMenus %%Owner:bradch */
EL ElViewMenus()
{
	return !vpref.fShortMenus + 2 * (selCur.doc == docNil);
}


/* %%Function:ElOutlineShowFirstLine %%Owner:bradch */
EL ElOutlineShowFirstLine(fOn)
BOOL fOn;
{
	if (vfElFunc)
		{
		fOn = PwwdWw(wwCur)->fEllip ? elFTrue : elFFalse;
		}
	else
		{
		if (vcElParams == 0)
			fOn = fTrue;

		if ((!fOn) != (!PwwdWw(wwCur)->fEllip))
			CmdOutlnUcm(bcmToggleEllip);
		}

	return fOn;
}


/* %%Function:WElSelType %%Owner:bradch */
EL int WElSelType(wSelType)
int wSelType;
{
	int pa;

	if (vfElFunc)
		{
		if (vcElParams > 0)
			RtError(rerrBadParam);

		Assert(wSelType == 0);
		wSelType = (selCur.cpFirst == selCur.cpLim) ? 1 : 2;
		if (selCur.pa == paDotted)
			wSelType |= 4;
		}
	else
		{
		if (vcElParams == 0 || (wSelType & 7) != wSelType)
			{
			RtError(rerrBadParam);
			}

		TurnOffSel(&selCur);

		if ((wSelType & 3) == 1)
			{
			if (selCur.cpFirst != selCur.cpLim)
				{
				Select(&selCur, selCur.cpFirst, 
						selCur.cpFirst);
				}
			}

		pa = (wSelType & 4) ? paDotted : paInvert;
		if (selCur.pa != pa)
			selCur.pa = pa;

		if (wSelType != 0)
			TurnOnSel(&selCur);
		}

	return wSelType;
}


/* %%Function:ELDisableInput %%Owner:bradch */
EL ELDisableInput(fDisable)
BOOL fDisable;
{
	extern vfElDisableInput;

	if (vcElParams == 0)
		vfElDisableInput = fTrue;
	else
		vfElDisableInput = fDisable;
}


/* %%Function:ElRenameMenu %%Owner:bradch */
EL ElRenameMenu(imnu, hstNewName)
uns imnu;
char ** hstNewName;
{
	extern HMENU vhMenu, vhMenuLongFull;
	extern struct STTB ** hsttbMenu;
	char stBuf [256];

#ifdef DEBUG
	/* Allow renaming the debug menu! */
	if (imnu > imnuDebug)
#else
		if (imnu >= imnuDebug)
#endif /* DEBUG */
			RtError(rerrBadParam);

	CopySt(*hstNewName, stBuf);
	if (!FChangeStInSttb(hsttbMenu, imnu, stBuf))
		RtError(rerrOutOfMemory);
	StToSzInPlace(stBuf);

	if (!ChangeMenu(vhMenuLongFull, imnu + vfSysMenu, (LPSTR) stBuf, 
			GetSubMenu(vhMenuLongFull, imnu + vfSysMenu), 
			MF_CHANGE | MF_BYPOSITION | MF_POPUP))
		RtError(rerrOutOfMemory);

	if (vhMenu == vhMenuLongFull)
		DrawMenuBar(vhwndApp);
}




/* Here we jump through hoops to get the time as a number of seconds because
our "standard" time format (dttm) does not have seconds... */
/* %%Function:LTimeCur %%Owner:bradch */
unsigned long LTimeCur()
{
	extern long LDaysFrom1900Dttm();
	long lTime;
	struct DTTM dttm;
	struct TIM tim;

	OsTime(&tim);
	dttm = DttmCur();
	lTime = tim.sec + 60L * 
			((long)dttm.hr * 60L /* hours in min */ +
			(long) dttm.mint + 1440L * (LDaysFrom1900Dttm(dttm)-1));

	return lTime;
}



OTD ** vhotd = hNil;


/* %%Function:ElOnTime %%Owner:bradch */
ElOnTime(hpsdTime, numTime, hstMacro, numTolerance)
SD huge * hpsdTime;
NUM numTime;
CHAR ** hstMacro;
NUM numTolerance;
{
	struct DTTM dttm;
	OTD * potd;

	if (vhotd != hNil)
		FreeH(vhotd);

	if ((vhotd = HAllocateCb(sizeof (OTD) + **hstMacro)) == hNil)
		RtError(rerrOutOfMemory);

	potd = *vhotd;
	if (hpsdTime != NULL)
		{
		int cch;
		char szTime [80];
		struct DAT dat;
		struct TIM tim;

		/* convert hpsdTime to serial time */
		if ((cch = CchFromSd(*hpsdTime)) > sizeof (szTime) - 1)
			RtError(rerrStringTooBig);
		bltbh(HpchFromSd(*hpsdTime), szTime, cch);
		szTime[cch] = '\0';

		if (!FDttmFromSzPl(szTime, &dttm, &potd->lTime))
			RtError(rerrBadParam);
		}
	else
		{
		potd->lTime = LWholeFromNum(&numTime, fFalse);
		}

	if ((potd->lTolerance = LWholeFromNum(&numTolerance, fFalse)) == 0)
		potd->lTolerance = 24L * (60 * 60); /* one day */

	if (VmnValidMacroName(*hstMacro) == vmnBad)
		RtError(rerrIllegalFunctionCall);

	CopySt(*hstMacro, potd->stMacro);
}


int viMacro = 0;
SD vsdMacro = sdNil;

/* %%Function:EFElMacros %%Owner:bradch */
EFElMacros(lpsy, bsy, stName, wParam)
SY FAR *lpsy;
uns bsy;
char * stName;
int wParam;
{
	if (viMacro == wParam)
		{
		Assert(wParam != -1);
		if ((vsdMacro = SdCopySt(stName)) == sdNil)
			RtError(rerrOutOfMemory);
		viMacro += 1;
		return fFalse;
		}
	viMacro += 1;
	return fTrue;
}


/* %%Function:ElWCountMacros %%Owner:bradch */
EL int ElWCountMacros(fTemplate, fAll)
BOOL fTemplate, fAll;
{
	viMacro = 0;
	FEnumMacros(EFElMacros, -1, 
			fAll ? mctAll : fTemplate ? mctMacro : mctGlobalMacro);

	return viMacro;
}


/* %%Function:ElSdMacroName %%Owner:bradch */
EL SD ElSdMacroName(iMacro, fTemplate, fAll)
uns iMacro;
BOOL fTemplate, fAll;
{
	if (iMacro == 0)
		{
		extern MES ** vhmes;

		if (vhmes != hNil)
			{
			MES * pmes = *vhmes;
			return SdCopySt(*pmes->rgmei[pmes->imeiCur].pstName);
			}
		else
			{
			return SdCopySt(stEmpty);
			}
		}

	iMacro -= 1;
	viMacro = 0;

	vsdMacro = sdNil;

	FEnumMacros(EFElMacros, iMacro, 
			fAll ? mctAll : fTemplate ? mctMacro : mctGlobalMacro);

	if (vsdMacro == sdNil)
		{
		RtError(rerrIllegalFunctionCall);
		/* NOT REACHED */
		Assert(fFalse);
		}

	return vsdMacro;
}



#ifdef DEBUG
/* %%Function:ElRareEvent %%Owner:bradch */
EL ElRareEvent(i, wEventNumber, wRepetitionsMac, wRepetitions, fOn)
uns i;
{
	extern ARG_RAREEVENTS RareEvent [RareEventMax];

	if (i >= RareEventMax)
		RtError(rerrBadParam);

	RareEvent[i].wEventNumber = wEventNumber;
	RareEvent[i].wRepetitionsMac = wRepetitionsMac;
	RareEvent[i].wRepetitions = wRepetitions;
	RareEvent[i].fOn = fOn;
}


#endif /* DEBUG */


/* %%Function:ElSdInkey %%Owner:bradch */
EL SD ElSdInkey()
{
	char stBuf [2];
	MSG msg;

	if (PeekMessage((LPMSG) &msg, NULL, WM_CHAR, WM_CHAR, TRUE))
		{
		stBuf[0] = 1;
		stBuf[1] = msg.wParam;
		}
	else
		stBuf[0] = 0;

	return SdCopySt(stBuf);
}


/* %%Function:ElSdInputBox %%Owner:bradch */
EL SD ElSdInputBox(hstPrompt, hstTitle, hstDefault)
char ** hstPrompt;
char ** hstTitle;
char ** hstDefault;
{
	char szBuf [cchMaxSz];
	char szClean [cchMaxSz];
	char dlt [sizeof (dltPrompt)];
	HCABPROMPT hcab;
	CMB cmb;
	RERR rerr;
	int cch;
	SD sd;

	rerr = rerrOutOfMemory;

	if ((hcab = HcabAlloc(cabiCABPROMPT)) == hNil)
		RtError(rerrOutOfMemory);

	StToSz(*hstPrompt, szBuf);
	SanitizeSz(szBuf, szClean, cchMaxSz, fTrue);
	if (!FSetCabSz(hcab, szClean, Iag(CABPROMPT, hszPrompt)))
		goto LErr;

	if (vcElParams == 3)
		{
		StToSz(*hstDefault, szBuf);
		SanitizeSz(szBuf, szClean, cchMaxSz, fTrue);
		}
	else
		{
		szClean[0] = '\0';
		}
	if (!FSetCabSz(hcab, szClean, Iag(CABPROMPT, hszInput)))
		goto LErr;

	if (hstTitle != NULL)
		StToSz(*hstTitle, szBuf);
	else
		CchCopySz(szAppTitle, szBuf);

	cmb.hcab = hcab;
	cmb.cmm = cmmNoHelp;
	cmb.pv = szBuf;
	cmb.bcm = bcmNil;

	BltDlt(dltPrompt, dlt);
	switch (TmcOurDoDlg(dlt, &cmb))
		{
	case tmcError:
		goto LErr;

	case tmcCancel:
		rerr = rerrCommandFailed;
		goto LErr;

	case tmcOK:
		break;
		}

		{
		char * pchS, * pchD;

	/* Convert CR/LFs to CRJs */
		pchS = pchD = *(*hcab)->hszInput;
		while ((*pchD = *pchS) != '\0')
			{
			if (*pchS == chEop)
				*pchD = chCRJ;
			if (*pchS != '\r')
				pchD += 1;
			pchS += 1;
			}
		}

	if ((cch = CchSz(*(*hcab)->hszInput) - 1) > 0)
		{
		if ((sd = SdFromCch(cch)) == sdNil)
			{
			rerr = rerrOutOfMemory;
			goto LErr;
			}
		bltbh((char huge *) *(*hcab)->hszInput, HpchFromSd(sd), cch);
		}
	else
		sd = 0;

	FreeCab(hcab);

	return sd;

LErr:
	FreeCab(hcab);
	RtError(rerr);
}


/* %%Function:FDlgPrompt %%Owner:bradch */
BOOL FDlgPrompt(dlm, tmc, wNew, wOld, wParam)
DLM dlm;
TMC tmc;
WORD wNew, wOld, wParam;
{
	char * szCaption;

	switch (dlm)
		{
	case dlmInit:
		if ((szCaption = PcmbDlgCur()->pv) == NULL)
			{
			szCaption = szAppTitle;
			/* I am assuming that the null case will happen
				in the fillin field case, not the macro input
				case, and there it is appropriate to limit
				the input to 255 chars since it goes to an st. bz
			*/
			}
		SetDlgCaption(szCaption);
		break;

	case dlmChange:
			/* I am assuming that the null case will happen
				in the fillin field case, not the macro input
				case, and there it is appropriate to limit
				the input to 255 chars since it goes to an st. bz
			*/
		Assert (tmc == tmcInput);
		if (PcmbDlgCur()->pv == NULL && wNew > 255)
			{
			char sz[256];  /* intentional constant bz */

			Beep();
			vfAwfulNoise = fFalse; /* enable beep for illegal looks keys */
			 /* actually truncate the text */
			GetTmcText(tmc, sz, 256);
		    sz[255] = 0;  /* this does the truncation */
			SetTmcText(tmc, sz);
			 /* ins pt at end */
			SetTmcTxs(tmc, TxsOfFirstLim(ichLimLast, ichLimLast));
			}
		break;
		}

	return fTrue;
}


/* %%Function:ElFileSaveAll %%Owner:bradch */
EL ElFileSaveAll(ac)
{
	/* Use same ac as normal Save All if appropriate */
	if (!FConfirmSaveAllDocs(ac == acQuerySave  ? acSaveAll : ac))
		RtError(rerrSuspend);
}


/* Handle the RND function

	R = FRC( 9821 * R + .211327)

	FRC means to return the fractional part of the number

	"This generator was developed by Don Malm as part of an HP-65
	Users' Library program.  It passes the spectral test (Knuth,
	V.2, pp3.4) and, because its parameters satisfy Theorem A
	(op. cit., p. 15), it generates one million distinct random
	numbers between 0 and 1 regardless of the value selected
	for the initial R."

	Adapted from Excel, 9/1/88 TDK

*/

NUM numRndMul={
	0x00,0x00,0x00,0x00,0x80,0x2e,0xc3,0x40}; /*  9821	  */


NUM numRndAdd={
	0x66,0x82,0xe1,0x5c,0xc3,0x0c,0xcb,0x3f}; /* .211327	  */


/* wIgnored is an optional parameter.  Currently it is completely ignored! */
/* %%Function:ElNumRnd %%Owner:bradch */
NUM ElNumRnd(wIgnored)
int wIgnored;
{
	extern NUM numRndSeed;

	LdiNum(&numRndSeed);
	MulINum(&numRndMul);
	AddINum(&numRndAdd);
	StiNum(&numRndSeed);
	LdiNum(&numRndSeed);
	FixNum(0);
	NegNum();
	AddINum(&numRndSeed);
	StiNum(&numRndSeed);
	return numRndSeed;
}



/* %%Function:ElUtilCalculate %%Owner:bradch */
EL NUM ElUtilCalculate(hst)
char ** hst;
{
	int iszErr;
	NUM numRet;
	char szBuf [256];

	if (vcElParams == 0)
		{
		if (!FEvalSelCur(fFalse, fFalse, &numRet, &iszErr))
			goto LError;
		}
	else
		{
		char * pch;

		pch = szBuf;
		if ((*hst)[1] != '=')
			*pch++ = '=';
		StToSz(*hst, pch);
		if (!FEvalExpSz(szBuf, &numRet, &iszErr, fFalse))
			{
LError:
			RtError(rerrBadParam);
			}
		}

	LdiNum(&numRet);
	RoundNum(-cElDigBlwDec);
	StiNum(&numRet);

	return numRet;
}


/* %%Function:FElFileExit %%Owner:bradch */
EL BOOL FElFileExit(ac)
uns ac;
{
	if (ac > 2)
		RtError(rerrBadParam);

	PostMessage(vhwndApp, WM_CLOSE, ac, 0L);
	RtError(rerrHalt);
}


/* %%Function:FElNextPage %%Owner:bradch */
EL BOOL FElNextPage()
{
	return CmdPgvNewPage(fTrue) == cmdOK;
}


/* %%Function:FElPrevPage %%Owner:bradch */
EL BOOL FElPrevPage()
{
	return CmdPgvNewPage(fFalse) == cmdOK;
}


/* %%Function:FElNextObj %%Owner:bradch */
EL BOOL FElNextObj()
{
	int docStart = selCur.doc;
	CP cpStart = selCur.cpFirst;

	return CmdDrCurs1(bcmNextDr, fFalse, fFalse) == cmdOK && 
			selCur.doc != docStart && selCur.cpFirst > cpStart;
}


/* %%Function:FElPrevObj %%Owner:bradch */
EL BOOL FElPrevObj()
{
	int docStart = selCur.doc;
	CP cpStart = selCur.cpFirst;

	return CmdDrCurs1(bcmPrevDr, fFalse, fFalse) == cmdOK && 
			selCur.doc != docStart && selCur.cpFirst < cpStart;
}


/* %%Function:ElDisableAutoMacros %%Owner:bradch */
EL ElDisableAutoMacros(fDisable)
BOOL fDisable;
{
	if (vcElParams == 0)
		fDisable = fTrue;

	vfDisableAutoMacros = fDisable;
}


