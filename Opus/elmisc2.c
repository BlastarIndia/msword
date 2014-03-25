/* E L M I S C 2 . C */


#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "debug.h"
#include "props.h"
#include "prm.h"
#include "sel.h"
#include "format.h"
#include "print.h"
#include "disp.h"
#include "el.h"
#include "ourmath.h"
#include "rerr.h"
#include "style.h"
#include "heap.h"
#include "field.h"
#include "doc.h"
#include "screen.h"
#include "ch.h"
#include "keys.h"
#include "ibdefs.h"	/* For IDLKSSHOWALL definition */
#define REPEAT /* So RRF is defined */
#include "ruler.h"
#include "cmdtbl.h"
#include "menu2.h"
#include "opuscmd.h"


#define dxaTabProxi 20	/* distance in twips that a tab can be from a 
position requested and still be considered 
to be at that position */

extern BOOL vfDeactByOtherApp;
extern MUD ** vhmudUser;
extern KMP ** vhkmpUser;
extern int vfElFunc;
extern int vcElParams;
extern struct SEL selCur;
extern struct CHP	vchpGraySelCur;
extern struct PAP	vpapSelCur;
extern struct PAP	vpapGraySelCur;
extern char szEmpty [];
extern struct STTB ** vhsttbFont;
extern struct PRI vpri;
extern struct ESPRM     dnsprm[];
extern RRF rrf;
extern struct ITR vitr;
extern SB sbStrings;
extern SB sbArrays;
extern struct FLI vfli;
extern struct MWD ** hmwdCur;
extern int wwCur;
extern HWND vhwndDeskTop;
extern HWND vhwndApp;
extern HWND vhwndRibbon;
extern struct SCI vsci;
extern BOOL vfElFunc;
extern struct PREF vpref;
extern struct WWD ** hwwdCur;
extern int vcElParams;
extern int vlm;
extern int fFirstPaint;
extern HWND vhwndPgPrvw;
extern struct PAP vpapFetch;

int cElParams;

/* %%Function:ElCharColor %%Owner:bradch */
EL ElCharColor(ico)
unsigned ico;
{
	if (ico >= icoMax)
		RtError(rerrBadParam);
	return ElProp(sprmCIco, ico, 0);
}


/* %%Function:ElBold %%Owner:bradch */
EL ElBold(fOn)
{
	if (fOn != 0)
		fOn = 1;

	return ElProp(sprmCFBold, fOn, 1);
}


/* %%Function:ElItalic %%Owner:bradch */
EL ElItalic(fOn)
{
	if (fOn != 0)
		fOn = 1;

	return ElProp(sprmCFItalic, fOn, 1);
}


/* %%Function:ElSmallCaps %%Owner:bradch */
EL ElSmallCaps(fOn)
{
	if (fOn != 0)
		fOn = 1;

	return ElProp(sprmCFSmallCaps, fOn, 1);
}


/* %%Function:ElVanish %%Owner:bradch */
EL ElVanish(fOn)
{
	int el;

	if (fOn != 0)
		fOn = 1;

	el = ElProp(sprmCFVanish, fOn, 1);
	if (!selCur.fIns)
		MakeSelCurVisi(fTrue/*fForceBlockToIp*/);
	else
		InvalVisiCache();

	return el;
}


/* %%Function:ElUnderline %%Owner:bradch */
EL ElUnderline(fOn)
{
	int kul;

	kul = fOn ? kulSingle : kulNone;

	return ElProp(sprmCKul, kul, kulSingle);
}


/* %%Function:ElDblUnderline %%Owner:bradch */
EL ElDblUnderline(fOn)
{
	int kul;

	kul = fOn ? kulDouble : kulNone;

	return ElProp(sprmCKul, kul, kulDouble);
}


/* %%Function:ElWordUnderline %%Owner:bradch */
EL ElWordUnderline(fOn)
{
	int kul;

	kul = fOn ? kulWord : kulNone;

	return ElProp(sprmCKul, kul, kulWord);
}


/* %%Function:ElSuperscript %%Owner:bradch */
EL ElSuperscript(fOn)
{
	int hps;

	hps = fOn ? 6 : 0;

	return ElProp(sprmCHpsPos, hps, 6);
}


/* %%Function:ElSubscript %%Owner:bradch */
EL ElSubscript(fOn)
{
	int hps;

	hps = fOn ? -6 : 0;

	return ElProp(sprmCHpsPos, hps, -6);
}


/* %%Function:ElCenterPara %%Owner:bradch */
EL ElCenterPara()
{
	return ElProp(sprmPJc, jcCenter, jcCenter);
}


/* %%Function:ElLeftPara %%Owner:bradch */
EL ElLeftPara()
{
	return ElProp(sprmPJc, jcLeft, jcLeft);
}


/* %%Function:ElRightPara %%Owner:bradch */
EL ElRightPara()
{
	return ElProp(sprmPJc, jcRight, jcRight);
}


/* %%Function:ElJustifyPara %%Owner:bradch */
EL ElJustifyPara()
{
	return ElProp(sprmPJc, jcBoth, jcBoth);
}


/* %%Function:ElSpace1 %%Owner:bradch */
EL ElSpace1()
{
	return ElProp(sprmPDyaLine, dyaSpace1, dyaSpace1);
}


/* %%Function:ElSpace2 %%Owner:bradch */
EL ElSpace2()
{
	return ElProp(sprmPDyaLine, dyaSpace2, dyaSpace2);
}


/* %%Function:ElSpace15 %%Owner:bradch */
EL ElSpace15()
{
	return ElProp(sprmPDyaLine, dyaSpace15, dyaSpace15);
}


/* %%Function:ElOpenPara %%Owner:bradch */
EL ElOpenPara()
{
	return ElProp(sprmPDyaBefore, dyaParaOpen, dyaParaOpen);
}


/* %%Function:ElClosePara %%Owner:bradch */
EL ElClosePara()
{
	return ElProp(sprmPDyaBefore, dyaParaClose, dyaParaClose);
}




#define wElGrayProp	(-1)


/* E L  P R O P */
/*
This function handles all of the Opel statements that allow toggling or
setting a property, and the functions that return the state of a property.

sprm indicates the property, val is either the value to set the property
to, or one of the two values to toggle between, valCmp is the value to
compare against when checking if a property is turned on and the second
of two values to toggle between.
*/
/* %%Function:ElProp %%Owner:bradch */
ElProp(sprm, val, valCmp)
int sprm, val, valCmp;
{
	BOOL fGray, fPara;
	int valCur;

	fPara = dnsprm[sprm].sgc == sgcPara;
	Assert (fPara || dnsprm[sprm].sgc == sgcChp);

	/* Get the current state... */
	if (fPara)
		{
		if (selCur.fUpdatePap)
			FGetParaState(fFalse, fFalse);
		}
	else
		{
		if (selCur.fUpdateChpGray)
			{
			FGetCharState(fFalse, fFalse);
			}
		}

	if (!(fGray = (ValFromPropSprm(fPara ? &vpapGraySelCur :
			&vchpGraySelCur, sprm) != 0)))
		{
		valCur = ValFromPropSprm(fPara ? &vpapSelCur :
				&selCur.chp, sprm);
		if (sprm == sprmCHpsPos && (valCur & 0x80))
			valCur -= 256;

		if (sprm == sprmCHpsPos && valCmp != 0 && valCur != 0 &&
				(valCmp < 0) == (valCur < 0))
			{
			valCmp = valCur;
			}
		}
	else
		valCur = 0;

	/* If we were called as a function, return state... */
	if (vfElFunc)
		{
		return fGray ? wElGrayProp : 
				((sprm == sprmCIco) ? valCur : valCur == valCmp);
		}

	if (PdodDoc(selCur.doc)->fLockForEdit)
		ModeError();

	/* Take care of toggling... */
	if (!fPara && vcElParams == 0)
		val = (!fGray && valCur == valCmp) ? val : valCmp;

	/* Apply the property unless there's no need... */
	if (fGray || valCur != val)
		{
		ApplySprmValSelCur(sprm, val);
		}

	return 0; /* to keep the stack happy */
}


/* %%Function:ApplySprmValSelCur %%Owner:bradch */
ApplySprmValSelCur(sprm, val)
{
	int cchPrl;
	char prl [4];

	cchPrl = CchPrlFromSprmVal(prl, sprm, val);
	Assert(cchPrl <= sizeof (prl));
	ApplyGrpprlSelCur(prl, cchPrl, fTrue /* fSetUndo */ );
}


#define PpafI(i)	((struct PAF *) PstFromSttb(vpri.hsttbPaf, i))
#define PffnI(i)	((struct FFN *) PstFromSttb(vhsttbFont, i))


/* %%Function:ElFontName %%Owner:bradch */
EL SD ElFontName(i)
int i;
{
	struct PAF * ppaf;
	struct FFN * pffn;

	if (i == 0)
		{
		/* we want name of font at insert point */
		int ftc;
		char * sz;

		if (selCur.fUpdateChpGray)
			FGetCharState(fFalse, fFalse);
		if (ValFromPropSprm(&vchpGraySelCur, sprmCFtc) != 0)
			return SdCopySz(szEmpty);

		ftc = ValFromPropSprm(&selCur.chp, sprmCFtc);
		sz = PffnI(IbstFontFromFtcDoc(ftc, selCur.doc))->szFfn;

		return SdCopySz(sz);
		}

	i -= 1; /* i starts at 1 externaly, 0 internaly */

/* If we have never gotten a printer DC, this is the time to do it */
	if (vpri.hsttbPaf == hNil)
		SetFlm(flmRepaginate);

	Assert(vpri.hsttbPaf != hNil);

	if (vpri.hsttbPaf == hNil || i >= (*vpri.hsttbPaf)->ibstMac)
		{
		return SdCopySz(szEmpty);
		}

	return SdCopySz(PffnI(PpafI(i)->ibst)->szFfn);
}


/* %%Function:ElNumFontSize %%Owner:bradch */
EL NUM ElNumFontSize(numps)
NUM numps;
{
	unsigned long hps;

	if (vfElFunc)
		{
		if (selCur.fUpdateChpGray)
			FGetCharState(fFalse, fFalse);
		hps = (ValFromPropSprm(&vchpGraySelCur, sprmCHps) != 0) ? 0 :
				ValFromPropSprm(&selCur.chp, sprmCHps);
		CNumInt((int) hps);
		CNumInt(2);
		DivNum();
		StiNum(&numps);
		}
	else
		{
		int cch;
		char rgb [10];

		if (PdodDoc(selCur.doc)->fLockForEdit)
			ModeError();

		LdiNum(&numps);
		CNumInt(2);
		MulNum();
		StiNum(&numps);
		hps = LWholeFromNum(&numps, fFalse);
		if ((int)hps < 8 || (int)hps > 254) /* as in TmcLooks */
			{
			RtError(rerrBadParam);
			return;
			}

		cch = CchPrlFromSprmVal(rgb, sprmCHps, (int)hps);
		ApplyGrpprlSelCur(rgb, cch, fFalse);

		rrf.hps = hps;
		ClearRulerRrf();
		}

	return numps;
}


/* %%Function:ElFont %%Owner:bradch */
EL ElFont(hstName, numps)
char ** hstName;
NUM numps;
{
	struct FFN * pffn;
	int ftc;
	CHAR rgchFfn [cbFfnLast];

	Assert(!vfElFunc);

	if (rrf.fSelMoved)
		ClearRrf();

	if (**hstName > LF_FACESIZE)
		{
		RtError(rerrBadParam);
		return;
		}

	pffn = rgchFfn;
	StToSz(*hstName, pffn->szFfn);
	if ((ftc = FtcValidateFont(pffn)) != wNinch && ftc != valNil)
		{
		char rgb [3];

		rgb[0] = sprmCFtc;
		*((int *) &rgb[1]) = ftc;
		ApplyGrpprlSelCur(rgb, 3, fFalse);

		rrf.ftc = ftc;
		ClearRulerRrf();
		}

	if (vcElParams == 2)
		ElNumFontSize(numps);
}


/* %%Function:ElCountFonts %%Owner:bradch */
EL ElCountFonts()
{
/* If we have never gotten a printer DC, this is the time to do it */
	if (vpri.hsttbPaf == hNil)
		SetFlm(flmRepaginate);

	if (vpri.hsttbPaf == hNil)
		return 0;

	return (*vpri.hsttbPaf)->ibstMac;
}


/* %%Function:NumElNextTab %%Owner:bradch */
EL NUM NumElNextTab(numPos)
NUM numPos;
{
	NextPrevTab(&numPos, fFalse);
	return numPos;
}


/* %%Function:NumElPrevTab %%Owner:bradch */
EL NUM NumElPrevTab(numPos)
NUM numPos;
{
	NextPrevTab(&numPos, fTrue);
	return numPos;
}


/* %%Function:NextPrevTab %%Owner:bradch */
NextPrevTab(pnumPos, fPrev)
NUM * pnumPos;
BOOL fPrev;
{
	extern struct PREF vpref;
	extern int DxaPrevTab(), DxaNextTab();
	extern unsigned mputczaUt [];

	CacheParaCa(&selCur.ca);
	LdiNum(pnumPos);
/*	CNumInt(mputczaUt[vpref.ut]);*/
	CNumInt(20);
	MulNum();
	CNumInt((*(fPrev ? DxaPrevTab : DxaNextTab))(CIntNum(), &vpapFetch));
/*	CNumInt(mputczaUt[vpref.ut]);*/
	CNumInt(20);
	DivNum();
	StiNum(pnumPos);
}


/* %%Function:WElOutlineLevel %%Owner:bradch */
EL WElOutlineLevel()
{
	extern struct WWD ** hwwdCur;
	int lvl;

	Assert(hwwdCur != hNil);

	if (!(*hwwdCur)->fOutline)
		return 0;

	if (!FUpdateHplcPad(selCur.doc))
		RtError(rerrOutOfMemory);

	if ((lvl = LvlFromDocCp(selCur.doc, selCur.cpFirst)) == lvlMax)
		lvl = -1;

	return lvl + 1;
}



/* %%Function:ElWCountStyles %%Owner:bradch */
EL int ElWCountStyles (fContext, fAll)
BOOL fContext;	/* 0 = document, 1 = document template */
BOOL fAll;	/* include all built-in styles */
{
	int doc;
	extern int vistLbMac;
	extern int docGlobalDot;

	if (fContext)
		{
		if ((doc = DocDotMother(selCur.doc)) == docNil)
			doc = docGlobalDot;
		}
	else
		doc = DocMother(selCur.doc);

	Assert(doc != docNil);	/* If this fails, need to add a check for
	docNil and return RtError(rerrModeError)
	if check is true */

	GenLBStyMapForDoc(doc, fAll, fFalse);
	return vistLbMac;
}



/* %%Function:ElSdStyleName %%Owner:bradch */
EL SD ElSdStyleName(wCount, fContext, fAll)
int wCount;	/* 0 = style name of current para, otherwise ranges from 1
to CountStyles(fContext, fAll). */
BOOL fContext;	/* 0 = document, 1 = document template */
BOOL fAll;	/* include all built-in styles */
{
	extern int docGlobalDot;
	extern int vistLbMac;
	extern char (**vhmpiststcp)[];
	extern BOOL vfShowAllStd;
	extern int vdocStsh;

	int doc, stcp;
	char stName[cchMaxStyle];
	char (**vhmpiststcpPrev)[];
	BOOL fShowAllSav;
	int istLbMacSav;

	if (fContext)
		{
		if ((doc = DocDotMother(selCur.doc)) == docNil)
			doc = docGlobalDot;
		}
	else
		doc = DocMother(selCur.doc);

	Assert(doc != docNil);

	SetVdocStsh(selCur.doc);

	fShowAllSav = vfShowAllStd;
	vfShowAllStd = fAll;

	if (wCount == 0)
		{
		/* If wCount is 0, only selCur.doc is considered, fContext
			and fAll are ignored. */
		CachePara(selCur.doc, selCur.cpFirst);
		stcp = StcpFromStc(vpapFetch.stc, PdodDoc(doc)->stsh.cstcStd);
		}
	else
		{
		vhmpiststcpPrev = vhmpiststcp;	/* Save current value */
		if ((vhmpiststcp = HAllocateCw(cwMpiststcp)) == hNil)
			{
			vhmpiststcp = vhmpiststcpPrev;	/* Restore value */
			vfShowAllStd = fShowAllSav;
			RtError(rerrOutOfMemory);
			}

		istLbMacSav = vistLbMac;
		GenLBStyMapForDoc(doc, fAll, fTrue);
		wCount--;	/* Since Lb map is zero-based */
		if (wCount >= vistLbMac)
			{
			FreeH(vhmpiststcp);
			vhmpiststcp = vhmpiststcpPrev;	/* Restore value */
			vfShowAllStd = fShowAllSav;
			vistLbMac = istLbMacSav;
			RtError(rerrOutOfRange);
			}

		stcp = (**vhmpiststcp)[wCount];

		FreeH(vhmpiststcp);
		vhmpiststcp = vhmpiststcpPrev;	/* Restore value */
		vistLbMac = istLbMacSav;
		}

	GenStyleNameForStcp(stName, PdodDoc(doc)->stsh.hsttbName, 
			PdodDoc(doc)->stsh.cstcStd, stcp);

	vfShowAllStd = fShowAllSav;

	if (stName[0] == 0)
		RtError(rerrOutOfRange);

	return SdCopySt(stName);
}


/* %%Function:IntElLenHpsd %%Owner:bradch */
EL int IntElLenHpsd(hpsd)
SD huge * hpsd;
{
	return CchFromSd(*hpsd);
}


/* %%Function:SdElStrNum %%Owner:bradch */
EL SD SdElStrNum(num)
NUM num;
{
	char stBuf [cchMaxNum*2+1];

	stBuf[0] = CchPnumToPch(&num, stBuf + 1);

	return SdCopySt(stBuf);
}


#define spMant	0	/* We are parsing the mantissa */
#define spExp1	1	/* We got a 'D' or 'E' and are expecting exponent 
This char may be '-' or '+' or a digit */
#define spExp2	2	/* Parsing exponent, this char must be a digit */

/* %%Function:NumElValHpsd %%Owner:bradch */
EL NUM NumElValHpsd(hpsd)
SD huge * hpsd;
{
	CHAR FAR * lpch;
	NUM num;
	int cchSz, cch, ich, ch, sp;
	BOOL fDecimal;
	char szBuf [256];

	if ((cchSz = CchFromSd(*hpsd)) >= sizeof (szBuf))
		{
		RtError(rerrBadParam);
		}

	/* BASIC 3.0 skips leading spaces, tabs, and line feeds */
	/* BASIC 4.0 also skips spaces, etc. in the middle of the */
	/* string, so we'll do the same. */

	/* Copy string to local buffer, skipping any chars <= a space */
	lpch = LpOfHp(HpchFromSd(*hpsd));
	sp = spMant;
	fDecimal = fFalse;
	for (cch = 0, ich = 0; cch < cchSz; cch++)
		{
		ch = ChUpper(*(lpch + cch));
		switch (sp)
			{
		case spMant:
			if (ch == 'D' || ch == 'E')
				{
				sp += 1;
				break;
				}
			
			if (ch == '-')
				{
				if (ich != 0)
					goto LEnd;
				break;
				}

			if (ch == '.')
				{
				if (fDecimal)
					goto LEnd;

				fDecimal = fTrue;
				break;
				}
			/* FALL THROUGH */

		case spExp2:
			if ((ch < '0' || ch > '9') && ch > ' ')
				goto LEnd;
			break;

		case spExp1:
			if (ch == '-' || ch == '+' || (ch >= '0' && ch <= '9'))
				sp += 1;
			else
				goto LEnd;
			}

		if (ch > ' ')
			szBuf[ich++] = ch;
		}
LEnd:
	szBuf[ich] = '\0';

	if (!CchPackSzPnum(szBuf, &num))
		RtError(rerrBadParam);

	return num;
}


/* %%Function:IntElAscHpsd %%Owner:bradch */
EL IntElAscHpsd(hpsd)
SD huge * hpsd;
{
	if (CchFromSd(*hpsd) == 0)
		{
		RtError(rerrBadParam);
		}

	return ((int) *HpchFromSd(*hpsd)) & 0x00FF;
}


/* %%Function:SdElChrInt %%Owner:bradch */
EL SD SdElChrInt(i)
unsigned i;
{
	char stBuf [2];

	if (i > 0xff)
		{
		RtError(rerrBadParam);
		}

	stBuf[0] = 1;
	stBuf[1] = i;

	return SdCopySt(stBuf);
}


/* %%Function:IntElIntInt %%Owner:bradch */
EL int IntElIntInt(i)
int i;
{
	return i; /* EL already did the conversion for us! */
}


/* %%Function:NumElAbsNum %%Owner:bradch */
EL NUM NumElAbsNum(num)
NUM num;
{
	CMPNUMRET cmp;

	LdiNum(&num);
	CNumInt(0);
	cmp = CmpNum();
	if (cmp.w2 >= 0)
		return num;

	LdiNum(&num);
	NegNum();
	StiNum(&num);

	return num;
}


/* %%Function:IntElSgnNum %%Owner:bradch */
EL int IntElSgnNum(num)
NUM num;
{
	CMPNUMRET cmp;

	LdiNum(&num);
	CNumInt(0);
	cmp = CmpNum();

	return cmp.w2;
}


/* %%Function:SdElLeftHpsdInt %%Owner:bradch */
EL SD SdElLeftHpsdInt(hpsd, i)
SD huge * hpsd;
unsigned i;
{
	SD sdRet;
	int cchSrc;

	cchSrc = CchFromSd(*hpsd);
	if (i > cchSrc)
		i = cchSrc;

	if ((sdRet = SdFromCch(i)) == sdNil)
		RtError(rerrOutOfMemory);
	BLTBH(HpchFromSd(*hpsd), HpchFromSd(sdRet), i);

	return sdRet;
}


/* %%Function:SdElRightHpsdInt %%Owner:bradch */
EL SD SdElRightHpsdInt(hpsd, i)
SD huge * hpsd;
unsigned i;
{
	SD sdRet;
	int cchSrc;

	cchSrc = CchFromSd(*hpsd);
	if (i > cchSrc)
		i = cchSrc;

	if ((sdRet = SdFromCch(i)) == sdNil)
		RtError(rerrOutOfMemory);
	BLTBH(HpchFromSd(*hpsd) + cchSrc - i, HpchFromSd(sdRet), i);

	return sdRet;
}


/* %%Function:SdElMidHpsdIntOInt %%Owner:bradch */
EL SD SdElMidHpsdIntOInt(hpsd, ich, cchRet)
SD huge * hpsd;
unsigned ich, cchRet;
{
	SD sdRet;
	int cchSrc;

	if ((int) (ich -= 1) < 0)
		{
		RtError(rerrBadParam);
		}

	cchSrc = CchFromSd(*hpsd);
	if (ich >= cchSrc)
		{
		RtError(rerrBadParam);
		}

	/* If count is omitted, remainder of string is returned.
		Also, make sure there are cchRet chars in the source. */
	if (cElParams == 2 || ich + cchRet > cchSrc)
		cchRet = cchSrc - ich;

	if ((sdRet = SdFromCch(cchRet)) == sdNil)
		RtError(rerrOutOfMemory);
	if (cchRet > 0)
		BLTBH(HpchFromSd(*hpsd) + ich, HpchFromSd(sdRet), cchRet);

	return sdRet;
}


/* %%Function:SdElStringInt %%Owner:bradch */
EL SD SdElStringInt(cch, hpsd, num)
int cch;
SD huge * hpsd;
NUM num;
{
	int ch;
	SD sdRet;
	char huge * hpch;

	if (hpsd == hpNil)
		{
		LdiNum(&num);
		ch = CIntNum();
		}
	else
		{
		if (CchFromSd(*hpsd) == 0)
			{
			RtError(rerrBadParam);
			}
		ch = *HpchFromSd(*hpsd);
		}

	if ((sdRet = SdFromCch(cch)) == sdNil)
		RtError(rerrOutOfMemory);

	hpch = HpchFromSd(sdRet);
	while (cch-- > 0)
		*hpch++ = ch;

	return sdRet;
}



/* SdCurDateOrTime
* Local Function.
*  Calls DttmCur to get the current date & time, then gets the default
*  date format via GetDefaultSzPic, then the formatted string via
*  CchFormatDttmPic.
* Called by ElSdTime and ElSdDate.
*/
/* %%Function:SdCurDateOrTime %%Owner:bradch */
SD SdCurDateOrTime(fDate)
{
	struct DTTM dttm;
	CHAR szResult[cchMaxSz];
	CHAR szPic[cchMaxPic];

	dttm = DttmCur();

	GetDefaultSzPic(szPic, fDate);
	szResult[CchFormatDttmPic(&dttm, szPic, szResult, cchMaxSz)] = '\0';

	return SdCopySz(szResult);
}



/* ElSdTime()
* EL function TIME$()
*/
/* %%Function:ElSdTime %%Owner:bradch */
EL SD ElSdTime()
{
	return SdCurDateOrTime(fFalse /* for time */);
}



/* ElSdDate()
* EL function DATE$()
*/
/* %%Function:ElSdDate %%Owner:bradch */
EL SD ElSdDate()
{
	return SdCurDateOrTime(fTrue /* for date */);
}



/* EL function INSTR([start], string1$, string2$)
*	Note that either two or three arguments are allowed: an optional number,
*	followed by two strings.  The interpreter attempts to model this by
*	allowing a string or number, followed by a string, followed by an optional
*	string.  This allows four argument types to reach the IntElInstr function:
*	  1) String, String    3) String, String, String
*	  2) Number, String    4) Number, String, String
*	Note that 2 and 3 are invalid, so they need to be rejected.
*	If the first argument is a number, hpsd1 will be hNil.  We can use this
*	to determine the invalid cases.
*/
/* %%Function:IntElInstr %%Owner:bradch */
EL int IntElInstr(hpsd1, num, hpsd2, hpsd3)
SD huge * hpsd1;
NUM num;
SD huge * hpsd2;
SD huge * hpsd3;
{
	SD sd1, sd2;
	uns ichStart;
	int cchRange, cchCmp;
	char huge * hpchCmp;
	char huge * hpch;

	/* Three strings, or a number and a string, are invalid calls */
	if ((cElParams == 3 && hpsd1 != hNil) ||
		 (cElParams == 2 && hpsd1 == hNil))
		{
		RtError(rerrBadParam);
		}

	if (cElParams == 2)
		{
		Assert(hpsd1 != hNil && hpsd2 != hNil);
		sd1 = *hpsd1;
		sd2 = *hpsd2;
		ichStart = 0;
		}
	else
		{
		Assert(cElParams == 3);
		Assert(hpsd2 != hNil && hpsd3 != hNil);
		sd1 = *hpsd2;
		sd2 = *hpsd3;
		LdiNum(&num);
		ichStart = CIntNum() - 1;
		if ((int) ichStart == -1)
			RtError(rerrIllegalFunctionCall);
		}

	cchRange = CchFromSd(sd1);
	cchCmp = CchFromSd(sd2);

	if (cchCmp > cchRange)
		return 0;

	while (ichStart <= cchRange - cchCmp)
		{
		hpchCmp = HpchFromSd(sd2);
		hpch = HpchFromSd(sd1) + ichStart;

		while (*hpch == *hpchCmp)
			{
			if (--cchCmp <= 0)
				return ichStart + 1; /* we found it */

			hpch += 1;
			hpchCmp += 1;
			}

		ichStart += 1;

		cchCmp = CchFromSd(sd2);
		}

	return 0;
}



/* Clears strings from one dimensional array.   RtError if not 1-D. */
/* %%Function:Clear1DArray %%Owner:bradch */
Clear1DArray(ad)
AD ad;
{
	SD huge * hpsd;
	int as;

	/* Make sure array is one dimension */
	if (HpahrOfAd(ad)->cas != 1)
		{
		RtError(rerrBadParam);
		/* NOT REACHED */
		Assert(fFalse);
		}

	hpsd = (SD huge *) &HpahrOfAd(ad)->rgas[1];
	for (as = 0; as < HpahrOfAd(ad)->rgas[0]; as++, hpsd++)
		{
		if (*hpsd != 0)
			FreeSd(*hpsd);
		*hpsd = 0;
		}
}




/* ElShowAll(fOn)
*  With no argument, toggles the state, otherwise sets the state.
*  Stolen shamelessly from CmdShowAll in cmdcore.c.
* EL procedure ShowAll [On].
*/

/* %%Function:ELShowAll %%Owner:bradch */
ELShowAll(fOn)
BOOL fOn;
{
	Assert( wwCur != wwNil );

	if (vcElParams == 1)
		{
		vpref.grpfvisi.fvisiShowAll = (fOn);	/* set or reset */
		(*hwwdCur)->grpfvisi.fvisiShowAll = (fOn);
		}
	else
		{
		vpref.grpfvisi.fvisiShowAll ^= 1;	/* toggle */
		(*hwwdCur)->grpfvisi.fvisiShowAll ^= 1;
		}

	TrashWw(wwCur);
	TrashFltg(fltgOther);

	/* only update button if it was clicked on, wait for idle if from kbd */
	if (vhwndRibbon != NULL)
		UpdateLHProp (vpref.grpfvisi.fvisiShowAll, 0, IDLKSSHOWALL);
	/* if doing show all in a pageview pane,
		do show all in all of the doc's pageview panes. */
	if ((*hwwdCur)->fPageView)
		SyncPageViewGrpfvisi(DocMother(selCur.doc), (*hwwdCur)->grpfvisi, (*hwwdCur)->fDisplayAsPrint);
}


#ifdef ELWINDBG		/* Debugging Routines */

/* %%Function:WToHexRgch %%Owner:bradch */
WToHexRgch(rgch, cch, w)
char rgch[];
int cch;
int w;
{
	int wm16;

	while (cch > 0)
		{
		wm16 = w % 16;
		if (wm16 <= 9)
			rgch[--cch] = '0' + wm16;
		else
			rgch[--cch] = 'A' + wm16 - 10;
		w /= 16;
		}
}


/* %%Function:CommHexPwCw %%Owner:bradch */
CommHexPwCw(pw, cw)
int * pw;
int cw;
{
	int w;
	char sz[5];

	sz[4] = '\0';

	while (cw-- > 0)
		{
		w = *(pw++);
		WToHexRgch(sz, 4, w);
		CommSz(sz);
		CommSz(" ");
		}
	CommSz("\r\n");
}


/* %%Function:CommHexPbCb %%Owner:bradch */
CommHexPbCb(pb, cb)
int * pb;
int cb;
{
	int b;
	char sz[3];

	sz[2] = '\0';

	while (cb-- > 0)
		{
		b = *(pb++);
		WToHexRgch(sz, 2, (unsigned int) b);
		CommSz(sz);
		CommSz(" ");
		}
	CommSz("\r\n");
}


#endif



/* %%Function:ElWResetChar %%Owner:bradch */
EL int ElWResetChar()
{
	extern struct PAP vpapFetch;
	extern struct CHP vchpFetch;
	extern BOOL vfElFunc;
	struct CHP chp;
	struct PAP papT;
	int stc;

	if (vfElFunc)	/* Called as a function */
		{
		extern int vccpFetch;
		CP cp;

		CacheParaCa(&selCur.ca);
		MapStc(PdodDoc(selCur.doc), stc = vpapFetch.stc, &chp, &papT);

		if (selCur.fIns)
			return FNeRgw(&chp, &selCur.chp, cwCHP) ? 0 : 1;

		for (cp = selCur.cpFirst; cp < selCur.cpLim; cp += vccpFetch)
			{
			CachePara(selCur.doc, cp);
			FetchCp(selCur.doc, cp, fcmProps);
			if (vpapFetch.stc != stc ||
					FNeRgw(&chp, &vchpFetch, cwCHP))
				{
				return 0;
				}
			}

		return 1;
		}

	CmdPlainText(NULL);
	return 0;	/* for stack */
}



/* %%Function:ElWResetPara %%Owner:bradch */
EL int ElWResetPara()
{
	struct PAP pap;
	extern struct PAP vpapFetch;
	extern BOOL vfElFunc;

	if (vfElFunc)	/* Called as a function */
		{
		CachePara(selCur.doc, selCur.cpFirst);
		MapStc(PdodDoc(selCur.doc), vpapFetch.stc, 0, &pap);

		/* compare pap to vpapFetch: first check "base" (up 
			to tab arrays) */
		if (FNeRgw((int *) &pap, (int *) &vpapFetch, cwPAPBase))
			return elFFalse;

		/* Now check tab arrays: positions & types.   We do this 
			because we only want to compare a valid number of tabs, 
			otherwise we may be comparing garbage. */
		if (pap.itbdMac)
			{
			if (FNeRgw((int *) &pap.rgdxaTab, 
					(int *) &vpapFetch.rgdxaTab, pap.itbdMac))
				return elFFalse;

			/* Now check tab types */
			if (FNeRgch((char *) &pap.rgtbd, 
					(char *) &vpapFetch.rgtbd, pap.itbdMac))
				return elFFalse;
			}
		}
	else
		CmdParaNormal(NULL);

	return elFTrue;
}



/* ItbdGetTabAt(pnumPos)
* Note that numPos is assumed to be in Points.
*  Finds closest tab to numPos, then sees if it is within dxaTabProxi twips
*  of numPos.   If so, returns the number of that tab, otherwise returns
*  -1.
* Locally used function.
* Called by ElTabType & ElSdTabLeader.
*/
/* %%Function:ItbdGetTabAt %%Owner:bradch */
int ItbdGetTabAt(pnumPos)
NUM * pnumPos;
{
	int dxa;
	int itbd = 0;
	int itbdMac;
	extern struct PAP vpapFetch;
	extern struct PREF vpref;

	/* get dxa (in twips) from *pnumPos (points) */
	/* 1 Point = 20 Twips */
	LdiNum(pnumPos);
	CNumInt(20);
	MulNum();
	dxa = CIntNum();

	itbdMac = vpapFetch.itbdMac;

	while (itbd < itbdMac && vpapFetch.rgdxaTab[itbd] < dxa)
		{
		itbd++;
		}

	if (itbd == itbdMac)
		{
		/* if we're on a default tab stop, return -2 */
		return (dxa % PdodDoc(selCur.doc)->dop.dxaTab == 0) ? -2 : -1;
		}

	/* If this tab is past dxa and the previous tab was closer, use the
		previous tab instead. */
	if (itbd > 0 && dxa < vpapFetch.rgdxaTab[itbd]
			&& dxa - vpapFetch.rgdxaTab[itbd - 1] < vpapFetch.rgdxaTab[itbd] - dxa)
		itbd--;

	/* Check if outside range to match requested position */
	if (vpapFetch.rgdxaTab[itbd] < dxa - dxaTabProxi
			|| vpapFetch.rgdxaTab[itbd] > dxa + dxaTabProxi)
		{
		return -1;
		}

	return itbd;
}



/* ElWTabType(numPos)
* Note that numPos is assumed to be Points
*/
/* %%Function:ElWTabType %%Owner:bradch */
EL int ElWTabType(numPos)
NUM numPos;
{
	int itbd;
	struct TBD tbd;
	extern struct PAP vpapFetch;

	CachePara(selCur.doc, selCur.cpFirst);
	itbd = ItbdGetTabAt(&numPos);
	if (itbd == -1)
		return -1;
	else  if (itbd == -2)
		return jcLeft; /* default tab stop */
	else
		{
		tbd = vpapFetch.rgtbd[itbd];
		return tbd.jc;	/* Tab Type code */
		}
}



/* ElSdTabLeader(numPos)
* Note that numPos is assumed to be Points
*/
/* %%Function:ElSdTabLeader %%Owner:bradch */
EL SD ElSdTabLeader(numPos)
NUM numPos;
{
	struct TBD tbd;
	int itbd;
	extern struct PAP vpapFetch;
	char sz[2];

	CachePara(selCur.doc, selCur.cpFirst);
	itbd = ItbdGetTabAt(&numPos);
	sz[0] = sz[1] = '\0';

	if (itbd >= 0)	/* tbd's are zero-based */
		{
		tbd = vpapFetch.rgtbd[itbd];
		switch (tbd.tlc)
			{
		case tlcNone:
			sz[0] = ' ';
			break;
		case tlcDot:
			sz[0] = '.';
			break;
		case tlcHyphen:
			sz[0] = '-';
			break;
		case tlcSingle:
			sz[0] = '_';
			break;
		default:
			break;
			}
		}
	else  if (itbd == -2)
		sz[0] = ' '; /* default tab leader */

	return SdCopySz(sz);
}


/* %%Function:ElWAppMaximize %%Owner:bradch */
EL int ElWAppMaximize(fOn)
BOOL fOn;
{
	BOOL fMax;	/* true if application window maximized */

	fMax = ((GetWindowLong(vhwndApp, GWL_STYLE) & WS_MAXIMIZE) != 0);

	if (vcElParams == 0)
		fOn = !fMax;

	if (vfElFunc)
		return (fMax) ? elFTrue : elFFalse;

	if ((!fOn) != (!fMax))
		{
		SendMessage(vhwndApp, WM_SYSCOMMAND, 
				fOn ? SC_MAXIMIZE : SC_RESTORE, 0L);
		}

	return 0;
}


/* %%Function:ElWAppMinimize %%Owner:bradch */
EL int ElWAppMinimize(fOn)
BOOL fOn;
{
	BOOL fMin;	/* true if application window minimized */

	fMin = ((GetWindowLong(vhwndApp, GWL_STYLE) & WS_MINIMIZE) != 0);

	if (vcElParams == 0)
		fOn = !fMin;

	if (vfElFunc)
		return (fMin) ? elFTrue : elFFalse;

	if ((!fOn) != (!fMin))
		{
		SendMessage(vhwndApp, WM_SYSCOMMAND, 
				fOn ? SC_MINIMIZE : SC_RESTORE, 0L);
		}

	return 0;
}


/* %%Function:ElWAppRestore %%Owner:bradch */
EL int ElWAppRestore()
{
	BOOL fRest;	/* true if application window both not maximized
	and not minimized */

	fRest = (((GetWindowLong(vhwndApp, GWL_STYLE) & WS_MAXIMIZE) == 0) &&
			((GetWindowLong(vhwndApp, GWL_STYLE) & WS_MINIMIZE) == 0));

	if (vfElFunc)
		return (fRest) ? elFTrue : elFFalse;

	if (!fRest)
		SendMessage(vhwndApp, WM_SYSCOMMAND, SC_RESTORE, 0L);

	return 0;
}


/* %%Function:ElWDocMaximize %%Owner:bradch */
EL int ElWDocMaximize(fOn)
BOOL fOn;
{
	if (!vfElFunc && (vcElParams == 0 || (!fOn) != (!vpref.fZoomMwd)))
		CmdZoomWnd(NULL);

	return (vpref.fZoomMwd) ? elFTrue : elFFalse;
}


/* %%Function:ElEditFNSep %%Owner:bradch */
EL ElEditFNSep()
{
	if (!FOpenHeaderIhtst(0, fFalse))
		{
		RtError(rerrOutOfMemory);
		Assert(fFalse);	/* NOT REACHED */
		}
}


/* %%Function:ElEditFNContSep %%Owner:bradch */
EL ElEditFNContSep()
{
	if (!FOpenHeaderIhtst(1, fFalse))
		{
		RtError(rerrOutOfMemory);
		Assert(fFalse);	/* NOT REACHED */
		}
}


/* %%Function:ElEditFNContNotice %%Owner:bradch */
EL ElEditFNContNotice()
{
	if (!FOpenHeaderIhtst(2, fFalse))
		{
		RtError(rerrOutOfMemory);
		Assert(fFalse);	/* NOT REACHED */
		}
}



/* %%Function:ElRsetFNSep %%Owner:bradch */
EL ElRsetFNSep()	/* EL Reset Footnote Separator */
{
	ResetFNItemIhdt(ihdtTFtn);
	/* ihdtTFtn means window open for editing footnote separator */
}


/* %%Function:ElRsetFNContSep %%Owner:bradch */
EL ElRsetFNContSep()	/* EL Reset Footnote Continuation Separator */
{
	ResetFNItemIhdt(ihdtTFtnCont);
	/* ihdtTFtnCont means window open for editing footnote continuation separator */
}


/* %%Function:ElRsetFNContNotice %%Owner:bradch */
EL ElRsetFNContNotice()	/* EL Reset Footnote Continuation Notice */
{
	ResetFNItemIhdt(ihdtBFtnCont);
	/* ihdtBFtnCont means window open for editing footnote continuation notice */
}


/* ResetFNItemIhdt resets the footnote separator, continuation separator,
* or continuation notice.   It requires that the appropriate footnote
* editing window be open.   It checks this by comparing the ihdt it gets
* to the ihdtWanted that is passed in. */
/* %%Function:ResetFNItemIhdt %%Owner:bradch */
ResetFNItemIhdt(ihdtWanted)
int ihdtWanted;
{
	CMB cmb;
	CMB *pcmb = &cmb;

	int docMom;
	int ihdt;

	/* must be in a header or footnote separator window */
	if (PwwdWw(wwCur)->wk != wkHdr)
		{
		RtError(rerrModeError);
		/* NOT REACHED */
		Assert(fFalse);
		}

	Assert(PdodDoc(selCur.doc)->fDispHdr);

	docMom = DocMother(selCur.doc);
	ihdt = PdodDoc(selCur.doc)->ihdt;

	/* Check ihdt == ihdtWanted, i.e. window open for
		editing appropriate footnote notice: separator, continuation
		separator, or continuation notice */
	if (ihdt != ihdtWanted)
		{
		RtError(rerrModeError);
		/* NOT REACHED */
		Assert(fFalse);
		}

	AssertDo(CmdHdrLinkPrev(pcmb) == cmdOK);
}


/* Toolbox conservation???!! */
#define IsZoomed(hwnd) (GetWindowLong((hwnd), GWL_STYLE) & WS_MAXIMIZE)


/* %%Function:ElAppMove %%Owner:bradch */
EL ElAppMove(x, y)
int x, y;
{
	int xp, yp;
	struct RC rc;
	struct PT pt;

	if (IsZoomed(vhwndApp))
		ModeError();

	/* Convert from points to pixels */
	xp = NMultDiv(x * dyaPoint, vfli.dxsInch, dxaInch);
	yp = NMultDiv(y * dyaPoint, vfli.dysInch, dxaInch);

	GetWindowRect(vhwndApp, (LPRECT) &rc);
	pt.xp = xp;
	pt.yp = yp;
	MoveWindow(vhwndApp, pt.xp, pt.yp,
			rc.xpRight - rc.xpLeft, rc.ypBottom - rc.ypTop, fTrue);
}


/* %%Function:ElAppSize %%Owner:bradch */
EL ElAppSize(dx, dy)
int dx, dy;
{
	int dxp, dyp;
	struct RC rc;

	if (IsZoomed(vhwndApp))
		ModeError();

	/* Convert from points to pixels */
	dxp = NMultDiv(dx * dyaPoint, vfli.dxsInch, dxaInch);
	dyp = NMultDiv(dy * dyaPoint, vfli.dysInch, dxaInch);

	GetWindowRect(vhwndApp, (LPRECT) &rc);
	MoveWindow(vhwndApp, rc.xpLeft, rc.ypTop, dxp, dyp, fTrue);
}


/* %%Function:ElMoveWindow %%Owner:bradch */
EL ElMoveWindow(x, y)
int x, y;
{
	int xp, yp;
	HWND hwnd;
	struct RC rc;

	if (hmwdCur == hNil)
		{
		RtError(rerrModeError);
		return;
		}

	hwnd = (*hmwdCur)->hwnd;

	if (vpref.fZoomMwd)
		ModeError();

	/* Convert from points to pixels */
	xp = NMultDiv(x * dyaPoint, vfli.dxsInch, dxaInch);
	yp = NMultDiv(y * dyaPoint, vfli.dysInch, dxaInch);

	GetWindowRect(hwnd, (LPRECT) &rc);
	MoveWindow(hwnd, xp, yp,
			rc.xpRight - rc.xpLeft, rc.ypBottom - rc.ypTop, fTrue);
}


/* %%Function:ElSizeWindow %%Owner:bradch */
EL ElSizeWindow(dx, dy)
int dx, dy;
{
	int dxp, dyp;
	HWND hwnd;
	struct RC rc;

	if (hmwdCur == hNil)
		{
		RtError(rerrModeError);
		return;
		}

	hwnd = (*hmwdCur)->hwnd;

	if (vpref.fZoomMwd)
		ModeError();

	/* Convert from points to pixels */
	dxp = NMultDiv(dx * dyaPoint, vfli.dxsInch, dxaInch);
	dyp = NMultDiv(dy * dyaPoint, vfli.dysInch, dxaInch);

	GetWindowRect(hwnd, (LPRECT) &rc);
	ScreenToClient(vhwndDeskTop, (LPPOINT) &rc.ptTopLeft);
	MoveWindow(hwnd, rc.xpLeft, rc.ypTop, dxp, dyp, fTrue);
}


/* %%Function:ElWSplitWindow %%Owner:bradch */
EL int ElWSplitWindow(dy)
int dy;
{
	int yp;

	if (hmwdCur == hNil)
		{
		RtError(rerrModeError);
		return 0;
		}

	if (vfElFunc)
		{
		struct MWD * pmwd;

		pmwd = *hmwdCur;
		if (pmwd->fSplit) yp=pmwd->ypSplit;
		else  
			return 0;
		dy=NMultDiv(yp, 100, pmwd->dyp);
		return dy;
		}
	else
		{
		if (dy == 0 && !(*hmwdCur)->fSplit)
			return 0;
		if (dy < 0 || dy > 100)
			RtError(rerrOutOfRange);
		yp=NMultDiv(dy, (*hmwdCur)->dyp, 100);
		if (!FSplitMwd(hmwdCur, yp, (*hwwdCur)->wk))
			/*RtError(rerrBadParam)*/;
		}
	return 0;
}


/* %%Function:ElVLine %%Owner:bradch */
EL ElVLine(ddl)
int ddl;
{
	extern void ScrollUp();
	extern void ScrollDown();
	PFN pfn;
	int dysMinScroll=1; /* min ht of a line in points */

	if (wwCur == wwNil)
		{
		RtError(rerrModeError);
		return;
		}

	if (vcElParams == 0)
		ddl = 1;

	if ((*hwwdCur)->fPageView)
		{
		int dye;

		dye = vsci.dysMinAveLine;
		FScrollPageDyeHome(wwCur, ddl * dye, fTrue, 0, fFalse);
		UpdateWindowWw(wwCur);
		return;
		}

	if (ddl < 0)
		{
		pfn = ScrollDown;
		ddl = -ddl;
		}
	else  
		{
		pfn = ScrollUp;
		}


	while (--ddl >= 0) 
		{
		(*pfn)( wwCur, dysMinScroll, vsci.dysMacAveLine);
		UpdateWindowWw(wwCur);
		}
}


/* %%Function:ElHPage %%Owner:bradch */
EL ElHPage(d)
int d;
{
	extern void ScrollRight();
	extern void ScrollLeft();
	struct WWD * pwwd;
	int dxpScroll;
	PFN pfn;

	if (wwCur == wwNil)
		{
		RtError(rerrModeError);
		return;
		}

	if (vcElParams == 0)
		d = 1;

	pwwd = PwwdWw(wwCur);
	dxpScroll = d * (DxOfRc(&pwwd->rcwDisp) / 
			(dxpMinScrollSci / 2) * (dxpMinScrollSci / 2));

	if (dxpScroll == 0)
		return;

	if (d < 0)
		{
		pfn = ScrollRight;
		dxpScroll = -dxpScroll;
		if (!pwwd->fPageView && DxpScrollHorizWw(wwCur) <= 0 &&
				!FAnyDlNotInXw(wwCur, fTrue))
			{
			/* Can't do scroll... */
			return;
			}
		}
	else
		pfn = ScrollLeft;

	(*pfn)(wwCur, dxpScroll);
	UpdateWindowWw(wwCur);
}


/* %%Function:ElVPage %%Owner:bradch */
EL ElVPage(d)
int d;
{
	extern void ScrollUp();
	extern void ScrollDown();
	extern struct WWD ** hwwdCur;
	PFN pfn;
	int dyp;
	struct WWD * pwwd;
	WORD wParam;
	MSG  msg;

	if (wwCur == wwNil)
		{
		RtError(rerrModeError);
		}

	if (vcElParams == 0)
		d = 1;

	if ((*hwwdCur)->fPageView)
		{
		int dye;

		pwwd = PwwdWw(wwCur);
		dye = DyOfRc(&pwwd->rcwDisp) - vsci.dysMinAveLine;
		if (d < 0) 
			{
			d = -d;
			dye = -dye;
			}
		while (--d >= 0) 
			{
			FScrollPageDyeHome(wwCur, dye, fTrue, 0, fFalse);
			UpdateWindowWw(wwCur);
			}
		return;
		}

	if (d < 0)
		{
		pfn = ScrollDown;
		wParam = SB_PAGEUP;
		d = -d;
		}
	else  
		{
		pfn = ScrollUp;
		wParam = SB_PAGEDOWN;
		}

	while (--d >= 0) 
		{
		if (vlm == lmPreview)
			{
			SendMessage(vhwndPgPrvw, WM_VSCROLL, wParam, 0L);
			}
		else  
			{
			pwwd = *hwwdCur;
			dyp = pwwd->ywMac - pwwd->ywMin;
			if (dyp > vsci.dysMacAveLine)
				dyp -= vsci.dysMacAveLine;
			(*pfn)(wwCur, dyp, dyp + vsci.dysMacAveLine - vsci.dysMinAveLine);
			UpdateWindowWw(wwCur);
			}
		}
}


/* %%Function:ElHLine %%Owner:bradch */
EL ElHLine(d)
int d;
{
	extern void ScrollRight();
	extern void ScrollLeft();
	struct WWD * pwwd;
	int dxpScroll;
	PFN pfn;

	if (wwCur == wwNil)
		{
		RtError(rerrModeError);
		}

	if (vcElParams == 0)
		d = 1;

	pwwd = PwwdWw(wwCur);
	dxpScroll = d * vsci.dxpMinScroll;

	if (dxpScroll == 0)
		return;

	if (d < 0)
		{
		pfn = ScrollRight;
		dxpScroll = -dxpScroll;
		if (!pwwd->fPageView && DxpScrollHorizWw(wwCur) <= 0 &&
				!FAnyDlNotInXw(wwCur, fTrue))
			{
			/* Can't do scroll... */
			return;
			}
		}
	else
		pfn = ScrollLeft;

	(*pfn)(wwCur, dxpScroll);
	UpdateWindowWw(wwCur);
}


/* NumScroll is a local function, called by ElNumVScroll and ElNumHScroll. */
/* %%Function:NumScroll %%Owner:bradch */
NUM NumScroll(numScrollToPercent, fHoriz)
NUM numScrollToPercent;
BOOL fHoriz;
{
	uns pos, posMost, posReq;
	NUM numPosPercent;

	NumFromL(0L, &numPosPercent);

	pos = SpsFromHwndRSB(fHoriz ? (*hmwdCur)->hwndHScroll : (*hwwdCur)->hwndVScroll);
	posMost = SpsLimFromHwndRSB(fHoriz ? (*hmwdCur)->hwndHScroll : (*hwwdCur)->hwndVScroll);
	/* Note that 'posMin' is assumed to be zero */

	if (vfElFunc)
		{
		/* Called as a function */

		if (posMost > 0)
			{
			/* numPosPercent = (pos * 100) / (posMost - posMin) using num math */
			/* Remember that posMin is assumed zero */
			CNumInt(pos);
			CNumInt(100);
			MulNum();
			CNumInt(posMost);
			DivNum();
			StiNum(&numPosPercent);
			}
		return numPosPercent;
		}

	/* Check that numScrollToPercent is not greater than 100 */
	LdiNum(&numScrollToPercent);
	CNumInt(100);
	if (CmpNum() > 0L)
		RtError(rerrOutOfRange);

	/* Check that numScrollToPercent is not less than 0 */
	LdiNum(&numScrollToPercent);
	CNumInt(0);
	if (CmpNum() < 0L)
		RtError(rerrOutOfRange);

	/* new position: posReq = posMost * numPercent / 100 */
	CNumInt(posMost);
	LdiNum(&numScrollToPercent);
	MulNum();
	CNumInt(100);
	DivNum();
	posReq = CIntNum();

	if (posReq != pos)	/* If we're already there, don't move */
		{
		if (fHoriz)
			MwdHorzScroll(wwCur, SB_THUMBPOSITION, posReq);
		else
			MwdVertScroll(wwCur, SB_THUMBPOSITION, posReq);
		}

	return numPosPercent;
}


/* %%Function:ElNumVScroll %%Owner:bradch */
EL NUM ElNumVScroll(numPercent)
NUM numPercent;
{

	if (wwCur == wwNil || hwwdCur == hNil || (*hwwdCur)->hwndVScroll == hNil)
		{
		RtError(rerrModeError);
		Assert(fFalse);	/* NOT REACHED */
		}

	return NumScroll(numPercent, fFalse /* fHoriz */);
}


/* %%Function:ElNumHScroll %%Owner:bradch */
EL NUM ElNumHScroll(numPercent)
NUM numPercent;
{
	if (wwCur == wwNil || hwwdCur == hNil || hmwdCur == hNil || (*hmwdCur)->hwndHScroll == hNil)
		{
		RtError(rerrModeError);
		Assert(fFalse);	/* NOT REACHED */
		}

	return NumScroll(numPercent, fTrue /* fHoriz */);
}


/* %%Function:ElWCountWindows %%Owner:bradch */
EL ElWCountWindows()
{
	extern struct STTB ** vhsttbWnd;

	Assert(vhsttbWnd != hNil);

	return (*vhsttbWnd)->ibstMac;
}


/* %%Function:ElSdWindowName %%Owner:bradch */
EL SD ElSdWindowName(i)
int i;
{
	extern struct STTB ** vhsttbWnd;
	char st [ichMaxBufDlg];

	Assert(vhsttbWnd != hNil);

	if (i < 0 || i > (*vhsttbWnd)->ibstMac)
		{
		RtError(rerrBadParam);
		return;
		}

	if (i == 0)
		{
		i = ElWWindow();
		}

	/* Deal with wierd strings! */
	GetStFromSttb(vhsttbWnd, i-1, st);
	st[0] -= 1;

	return SdCopySt(st);
}


/* %%Function:ElWPane %%Owner:bradch */
EL ElWPane()
{
	return (hmwdCur != hNil && (*hmwdCur)->fSplit &&
			wwCur == (*hmwdCur)->wwLower) ? 3 : 1;
}


/* %%Function:ElWWindow %%Owner:bradch */
EL int ElWWindow()
{
	extern struct STTB ** vhsttbWnd;
	extern int mwCur;
	char * pch;
	int ibst;

	if (hmwdCur == hNil)
		return 0;

	for (ibst = 0; ibst < (*vhsttbWnd)->ibstMac; ++ibst)
		{
		pch = PstFromSttb(vhsttbWnd, ibst);
		if (pch[*pch] == mwCur)
			return ibst + 1;
		}

	Assert(fFalse);
	return 0;
}


/* %%Function:ElActivate %%Owner:bradch */
ElActivate(hst, iPane)
char ** hst;
uns iPane;
{
	extern struct STTB ** vhsttbWnd;
	int ibst, ibstMac, iTry;
	char stzBuf [256 + 10];

	iPane = (iPane - 1) >> 1;

	if (vcElParams == 2 && iPane > 1)
		goto LOutOfRange;

	StToSz(*hst, stzBuf + 1);
	stzBuf[0] = (*hst)[0];

	ibstMac = (*vhsttbWnd)->ibstMac;

	for (iTry = 0; iTry < 2; ++iTry)
		{
		for (ibst = 0; ibst < ibstMac; ++ibst)
			{
			char stWw [ichMaxFile + 5];

			GetStFromSttb(vhsttbWnd, ibst, stWw);
			Assert(stWw[0] < sizeof (stWw));
			Assert(stWw[0] > 1);
			stWw[0] -= 1;
			if (FEqNcSt(stzBuf, stWw))

				{
				extern int wwCur, mwCur;
				struct MWD * pmwd;
				int mw, ww;

				mw = stWw[stWw[0] + 1];
				pmwd = PmwdMw(mw);
				ww = pmwd->wwUpper;

				if (vcElParams == 2)
					{
					if (iPane == 1)
						{
						if (!pmwd->fSplit)
							{
LOutOfRange:
							RtError(rerrOutOfRange);
							/* NOT REACHED */
							Assert(fFalse);
							}

						ww = pmwd->wwLower;
						}
					}

				if (vfDeactByOtherApp)
					SetActiveWindow(vhwndApp);
				
				if (ww != wwCur)
					NewCurWw(ww, fFalse /*fDoNotSelect*/);

				return;
				}
			}

		stzBuf[0] = CchSzFileAbsToSzFileRel(stzBuf + 1, stzBuf + 1);
		}

	/* Not found */
	RtError(rerrNoSuchWin);
}


/* %%Function:ElCountMenuItems %%Owner:bradch */
ElCountMenuItems(imnu, fTemplate)
{
	MUD ** hmud;
	int docDot;
	int cItems;
	MTM * pmtm;

	imnu -= 1;

	if (fTemplate)
		{
		if (selCur.doc == docNil || 
				(docDot = DocDotMother(selCur.doc)) == docNil)
			{
			ModeError();
			}
		hmud = HmudUserPdod(PdodDoc(docDot));
		}
	else
		hmud = vhmudUser;

	for (pmtm = (*hmud)->rgmtm; pmtm->imnu < imnu; pmtm += 1)
		;
	cItems = 0;
	for ( ; pmtm->imnu == imnu; pmtm += 1)
		cItems += 1;

	return cItems;
}


/* %%Function:ElSdMenuMacro %%Owner:bradch */
SD ElSdMenuMacro(imnu, imtm, fTemplate)
uns imnu;
uns imtm;
{
	MUD ** hmud;
	int docDot;
	BCM bcm;
	MTM * pmtm;

	imnu -= 1; /* 1 based externaly */
	imtm -= 1; /* 1 based externaly */

	if (fTemplate)
		{
		if (selCur.doc == docNil || 
				(docDot = DocDotMother(selCur.doc)) == docNil)
			{
			ModeError();
			}
		hmud = HmudUserPdod(PdodDoc(docDot));
		}
	else
		hmud = vhmudUser;

	for (pmtm = (*hmud)->rgmtm; pmtm->imnu < imnu; pmtm += 1)
		;
	pmtm += imtm;
	if (pmtm->imnu != imnu)
		{
		RtError(rerrIllegalFunctionCall);
		}

	if ((int) pmtm->bcm < 0)
		return SdCopySt(StShared("-"));
	else
		{
		char stName [cchMaxSyName];

		GetNameFromBsy(stName, pmtm->bcm);
		return SdCopySt(stName);
		}
}


/* %%Function:ElSdMenuText %%Owner:bradch */
SD ElSdMenuText(imnu, imtm, fTemplate)
{
	MUD ** hmud;
	int docDot;
	BCM bcm;
	MTM * pmtm;
	char szBuf [256];

	imnu -= 1; /* 1 based externaly */
	imtm -= 1; /* 1 based externaly */

	if (fTemplate)
		{
		if (selCur.doc == docNil || 
				(docDot = DocDotMother(selCur.doc)) == docNil)
			{
			ModeError();
			}
		hmud = HmudUserPdod(PdodDoc(docDot));
		}
	else
		hmud = vhmudUser;

	for (pmtm = (*hmud)->rgmtm; pmtm->imnu < imnu; pmtm += 1)
		;
	pmtm += imtm;
	if (pmtm->imnu != imnu)
		RtError(rerrIllegalFunctionCall);

	if (pmtm->fRemove)
		szBuf[0] = '\0';
	else
		{
		extern struct STTB ** hsttbMenu;
		if (pmtm->ibst != 0)
			GetSzFromSttb(hsttbMenu, pmtm->ibst, szBuf);
		else
			CchBuildMenuSz(pmtm->bcm, szBuf, grfBldMenuDef);
		}

	return SdCopySz(szBuf);
}


/* %%Function:ElSdMacroDesc %%Owner:bradch */
SD ElSdMacroDesc(hstName)
char ** hstName;
{
	BCM bcm;
	char stName [256];

	CopySt(*hstName, stName);
	if ((bcm = BcmOfSt(stName)) == bcmNil)
		RtError(rerrIllegalFunctionCall);

	if (bcm < bcmMacStd)
		return 0;

	/* Using stName as an sz here! */
	GetMenuHelpSz(bcm, stName);
	return SdCopySz(stName);
}


/* %%Function:ElCountKeys %%Owner:bradch */
ElCountKeys(fTemplate)
{
	KMP ** hkmp;
	int docDot;

	if (fTemplate)
		{
		if (selCur.doc == docNil || 
				(docDot = DocDotMother(selCur.doc)) == docNil)
			{
			ModeError();
			}

		hkmp = PdodDoc(docDot)->hkmpUser;
		}
	else
		hkmp = vhkmpUser;

	return (*hkmp)->ikmeMac;
}


/* %%Function:ElKeyCode %%Owner:bradch */
ElKeyCode(iKey, fTemplate)
uns iKey;
{
	KMP ** hkmp;
	int docDot;

	iKey -= 1; /* 1 based externally */

	if (fTemplate)
		{
		if (selCur.doc == docNil || 
				(docDot = DocDotMother(selCur.doc)) == docNil)
			{
			ModeError();
			}

		hkmp = PdodDoc(docDot)->hkmpUser;
		}
	else
		hkmp = vhkmpUser;

	if (iKey > (*hkmp)->ikmeMac)
		RtError(rerrIllegalFunctionCall);

	return (*hkmp)->rgkme[iKey].kc;
}


/* %%Function:ElSdKeyMacro %%Owner:bradch */
SD ElSdKeyMacro(iKey, fTemplate)
uns iKey;
{
	KMP ** hkmp;
	KME * pkme;
	int docDot;

	iKey -= 1; /* 1 based externally */

	if (fTemplate)
		{
		if (selCur.doc == docNil || 
				(docDot = DocDotMother(selCur.doc)) == docNil)
			{
			ModeError();
			}

		hkmp = PdodDoc(docDot)->hkmpUser;
		}
	else
		hkmp = vhkmpUser;

	if (iKey > (*hkmp)->ikmeMac)
		RtError(rerrIllegalFunctionCall);

	pkme = &(*hkmp)->rgkme[iKey];
	if (pkme->kt != ktMacro)
		return 0; /* null string */
	else
		{
		FetchSy(pkme->bcm);
		return SdCopySt(vpsyFetch->stName);
		}
}
