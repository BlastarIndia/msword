/*#define DINTERP*/

/* elsubs3.c -- Support for the EL interpreter (less frequently used functions)*/

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "debug.h"
#include "heap.h"
#define RERRSTRINGS
#include "rerr.h"
#include "cmdtbl.h"
#include "el.h"
#include "idd.h"
#include "opuscmd.h"
#include "macrocmd.h"
#include "ch.h"
#include "version.h"
#include "props.h"
#include "prm.h"
#include "doc.h"
#include "sel.h"
#include "disp.h"
#include "dde.h"
#include "keys.h"
#include "help.h"
#include "rareflag.h"
#include "sdmparse.h"
#include "error.h"
#include "dlgcheck.h"

extern CP CpMacroLineIb();
extern struct SEL selCur;
extern int vcxtHelp;
extern HWND vhwndCBT;
extern SB sbTds;
extern struct SEL selMacro;
extern MES	**vhmes;
extern struct MERR vmerr;

BOOL vfElNumFormat;
DPV DpvDttmFromTmcSz();
BOOL vfElOom;
extern WParseDttm();

#define FEditHpeli(hpeli) ((hpeli)->w == 0xffff)
#define ImeiFromEditHpeli(hpeli) (LOWORD((hpeli)->l))



#ifdef DEBUG
csconst char rgchHex [] = "0123456789ABCDEF";
/* %%Function:CommHexRgb %%Owner:bradch */
CommHexRgb(pb, cb)
char * pb;
int cb;
{
	char sz [4];

	while (cb-- > 0)
		{
		sz[0] = rgchHex[*pb >> 4];
		sz[1] = rgchHex[*pb & 0xf];
		sz[2] = ' ';
		sz[3] = '\0';
		CommSz(sz);
		pb += 1;
		}
}


#endif


int vrerr = rerrNil;


/* F L A G  R T  E R R O R */
/* %%Function:FlagRtError %%Owner:bradch */
FlagRtError(eid)
int eid;
{
	if (vrerr == rerrNil)
		vrerr = rerrAppMin + eid;
}


/* F  I N I T  M A C R O  D O C */
/* Make sure the global macro document exists. */
/* %%Function:FInitMacroDoc %%Owner:bradch */
FInitMacroDoc()
{
	extern int docGlobalDot;

	Assert(docGlobalDot != docNil);

	return !(PdodDoc(docGlobalDot)->docMcr == docNil &&
			DocCreateMcr(docGlobalDot) == docNil);
}


/* C C H  P N U M  T O  P C H */
/* REVIEW BradCh: this really needs to be done better... */
/* %%Function:CchPnumToPch %%Owner:bradch */
CchPnumToPch(pnum, pch)
NUM * pnum;
char * pch;
{
	int cch;
	vfElNumFormat=fTrue;
	cch = CchFormatNumPic(pnum, szElNumPic, pch, cchMaxNum*2);
	vfElNumFormat=fFalse;

	if (cch > cchFormatNumMax && fElActive)
		RtError(rerrOutOfRange);

	Assert(cch >= 2);

	/* Hack to eliminate trailing "." or ".0" */
	if (pch[cch - 1] == '.')
		pch[--cch] = '\0';
	else  if (pch[cch - 2] == '.' && pch[cch - 1] == '0')
		{
		cch -= 2;
		pch[cch] = '\0';
		}

	return cch;
}


/* S D  C O P Y  H S T T B  I S T */
/* %%Function:SdCopyHsttbIst %%Owner:bradch */
SD SdCopyHsttbIst(hsttb, ist)
struct STTB **hsttb;
int ist;
{
	CHAR st[cchMaxSt];
	GetStFromSttb(hsttb, ist, st);
	return SdCopySt(st);
}


/* F  E L  E L X */
/* %%Function:FElElx %%Owner:bradch */
FElElx(elx)
ELX elx;
{
	FetchSy(BcmFromIbcm(elx));
	return FElMct(vpsyFetch->mct);
}





/* B S T  R N G  F R O M  H I D */
/* %%Function:BstRngFromHid %%Owner:bradch */
int BstRngFromHid(hid)
HID hid;
{
	static int bstLast = -1, hidLast = -1; /* simple cache */
	int ibst;

	if (hid == hidLast)
		return bstLast;

	for (ibst = 0; ibst < sizeof (mpibstrnghid) / sizeof (int); ibst += 1)
		if (mpibstrnghid[ibst] == hid)
			break;
	Assert(ibst != sizeof (mpibstrnghid) / sizeof (int));

	hidLast = hid;
	bstLast = rgbstRng[ibst];

	return bstLast;
}


/* The rng structure contains wLow and wHigh as the first two elements.  When
these values are the same, it indicates that the rng is "special."  The 
following macros detext these special rngs. */

#define FStringRng(rng) (*((long *) &rng) == 0x00000000L)
#define FParsedRng(rng) (*((long *) &rng) == 0x00010001L)


/* F  C H E C K  C A B */
/* %%Function:FCheckCab %%Owner:bradch */
FCheckCab(pcmb)
CMB * pcmb;
{
	int iag, iagMac;
	int irng;
	CHAR FAR * pchRng;
	int ichRng;
	int cchRng;
	int * pag;
	int bstRng;
	CABH * pcabh;

	FetchSy(pcmb->bcm);
	bstRng = BstRngFromHid(HidFromIeldi(IeldiFromPsy(vpsyFetch)));

	pchRng = &grpstRng[bstRng];
	cchRng = *pchRng++;

	pcabh = (CABH *) *pcmb->hcab;
	iagMac = pcabh->cwSimple /*+ pcabh->cwHandle*/;

	Assert(iagMac == cchRng);

	pag = (int *) *pcmb->hcab + cwCabMin;
	ichRng = 1;
	for (iag = 0; iag < iagMac; iag += 1, pag += 1, ichRng += 1)
		{
		int w, fooNinch, fAllowNinch, wLow, wHigh;

		w = *pag;
		irng = (grpstRng + bstRng)[ichRng];

		if (fAllowNinch = (irng & 0x80))
			{
			irng &= 0x7f;
			fooNinch = rgrng[irng].wLow >= 0 ? uNinch : wNinch;
			}

		Assert((uns) irng < sizeof (rgrng) / sizeof (RNG));

		wLow = rgrng[irng].wLow;
		wHigh = rgrng[irng].wHigh;

		/* Do not check edit controls */
		if (FStringRng(rgrng[irng]))
			continue;

		if ((w < wLow || w > wHigh) && 
				(!fAllowNinch || w != fooNinch))
			{
			return fFalse;
			}
		}

	return fTrue;
}


/* S E T  C A B  V A L */
/* %%Function:SetCabVal %%Owner:bradch */
SetCabVal(hid, hcab, iag, hpval, elv)
HID hid;
HCAB hcab;
int iag;
void huge * hpval;
ELV elv;
{
	extern SB sbStrings;
	int * pag;
	RNG rng;
	NUM numT;
	int w;
	BOOL fAllowNinch;
	int fooNinch;

	if (hid == IDDUsrDlg)
		{
		if ((iag < ((CABH *) *hcab)->cwHandle) != (elv == elvSd))
			RtError(rerrTypeMismatch);

		switch (elv)
			{
		case elvSd:
			/* make string appear as unparsed */
			rng.wHigh = rng.wLow = 0;
			break;

		case elvNum:
			BLTBH(hpval, &numT, sizeof (NUM));
			LdiNum(&numT);
			w = CIntNum();
			goto LSetVal;

		case elvInt:
			w = *((int huge *) hpval);
			goto LSetVal;
			}
		}
	else
		{
		uns bstRng;
		int irng;

		bstRng = BstRngFromHid(hid);
		irng = (grpstRng + bstRng)[1 + iag];
		rng = rgrng[irng & 0x7f];
		fAllowNinch = irng & 0x80;
		fooNinch = rng.wLow >= 0 ? uNinch : wNinch;
		}

	if (FStringRng(rng) && elv != elvSd)
		{
		RtError(rerrTypeMismatch);
		}

	switch (elv)
		{
	default:
		RtError(rerrTypeMismatch);
		Assert(fFalse); /* NOT REACHED */

	case elvSd:
			{
			SD sd;
			int cch;
			void * pv;
			PFN pfnWParse;
			TMC tmc;
			WORD opt;
			BOOL fPt;
			char szBuf [cchMaxSz];

			sd = *((SD huge *) hpval);
			if ((cch = CchFromSd(sd)) >= cchMaxSz)
				RtError(rerrStringTooBig);

			BLTBH(HpchFromSd(sd), (HP) szBuf, cch);
			szBuf[cch] = '\0';

			if (FStringRng(rng))
				{
			/* Unparsed edit item... */

				if (!FSetCabSz(hcab, szBuf, iag))
					RtError(rerrOutOfMemory);
				return;
				}


		/* Parsed item... */

			if (FParsedRng(rng))
				{
				struct DTTM dttm;

				pv = &dttm;
				if (!WParseDttm(tmmParse, szBuf, &pv, 0, tmcNull, 0))
					RtError(rerrHalt);
				*((struct DTTM *)
						((int *) *hcab + cwCabMin + iag)) = dttm;
				return;
				}
			else
				{
			/* Parse the string to get w... */
				if ((pfnWParse = PfnTmcOptFromHidIag(hid, iag, 
						&tmc, &opt, &fPt)) == NULL)
					RtError(rerrTypeMismatch);
				pv = &w;
				if (!(*pfnWParse)(tmmParse, szBuf, &pv, 0, tmc, opt))
					RtError(rerrHalt);
				goto LSetVal;
				}

			Assert(fFalse); /* NOT REACHED */
			}

	case elvInt:
		w = *((int huge *) hpval);

		if (fAllowNinch && w == fooNinch)
			goto LSetVal;

		if (rng.wMult != 0 || rng.wDiv != 0)
			{
			CNumInt(w);
			goto LScale;
			}
		break;

	case elvNum:
		BLTBH(hpval, &numT, sizeof (NUM));

		if (fAllowNinch)
			{
			LdiNum(&numT);
			w = CIntNum();
			if (w == fooNinch)
				goto LSetVal;
			}

		LdiNum(&numT);

LScale:
		if (rng.wMult != 0)
			{
			CNumInt(rng.wMult);
			MulNum();
			}

		if (rng.wDiv != 0)
			{
			CNumInt(rng.wDiv);
			DivNum();
			}

		w = CIntNum();
		break;
		}

	if (w < rng.wLow || w > rng.wHigh)
		{
		RtError(rerrIllegalFunctionCall);
		Assert(fFalse); /* NOT REACHED */
		}

LSetVal:
	*((int *) *hcab + cwCabMin + iag) = w;
}


#ifdef FUTURE
/* W  G E T  C A B */
/* %%Function:WGetCab %%Owner:bradch */
WORD WGetCab(hcab, iag, bstRng)
HCAB hcab;
int iag;
uns bstRng;
{
}


#endif


/* %%Function:PpvAllocCbWW %%Owner:bradch */
void ** PpvAllocCbWW(sb, cb)
SB sb;
unsigned cb;
{
	void ** ppv;

	if ((ppv = PpvAllocCb(sb, cb)) == 0)
		{
		ShrinkSwapArea();
		ppv = PpvAllocCb(sb, cb);
		GrowSwapArea();
		}

	return ppv;
}


csconst char csszWordBasic [] = SzKey("WordBASIC", WordBASIC);
csconst char csszErrNum [] = SzKey(" Err=", ErrNum);


/* F  M A C R O  E R R  M S G */
/* Displays an interpreter error message and returns fTrue iff macro
	may be continued. */
/* %%Function:FMacroErrMsg %%Owner:bradch */
FMacroErrMsg(heli, rerr)
ELI ** heli;
RERR rerr;
{
	BOOL fCanCont;
	char far * lpsz;
	char szBuf [256];
	int ierd;
	int erdrerr;
	ELI huge * hpeli;
	long lib;

	hpeli = HpeliOfHeli(heli);
	lib = hpeli->lib;

	fCanCont = fFalse;
	lpsz = (LPSTR) 0;

	switch (rerr)
		{
	case rerrSuspend: /* single step break */
		fCanCont = fTrue;
		/* FALL THROUGH */

	case rerrNil: /* normal end */
	case rerrHalt: /* some error */
		break;

	case rerrStop:
		lpsz = (LPSTR) SzSharedKey("Macro interrupted",	MacroInterrupted);
		fCanCont = fTrue;
		goto LMsg;

	case rerrOutOfMemory:
		if (vhwndCBT)
			{
			vmerr.fCBTMacroMemErr = fTrue;
			return;
			}
		/* else fall through */

	default:
		if (rerr >= rerrAppMin)
			break; /* message already displayed! */

		/* FUTURE: error message search should be
			changed to something more efficient. */

		ierd = 0;
		while ((erdrerr = rgerd[ierd].rerr) != rerr && erdrerr != -1)
			ierd++;

		if (erdrerr == -1)
			erdrerr = rerrInternalError;

		lpsz = rgerd[ierd].sz; /* rgerd is a csconst */
LMsg:
		if (lpsz != (LPSTR) 0)
			{
			extern char szApp [];
			char far * lpch;
			char * pch;
			int cxtT;
			char szTitle [50];

			lpch = lpsz;
			pch = szBuf;
			while ((*pch++ = *lpch++) != '\0')
				;

			cxtT = vcxtHelp;
			vcxtHelp = cxtMacroError;
			CopyCsSz(csszWordBasic, szTitle);
			pch = szTitle + sizeof (csszWordBasic) - 1;
			if ((int) rerr > 0)
				{
				CopyCsSz(csszErrNum, pch);
				pch += sizeof (csszErrNum) - 1;
				CchIntToPpch(rerr, &pch);
				}
			*pch = '\0';
			vfElOom = (rerr == rerrOutOfMemory);
			IdOurMessageBox(szBuf, szTitle,
					MB_OK | MB_ICONEXCLAMATION);
			vfElOom = fFalse;
			vcxtHelp = cxtT;
			}
		}

	if (FEditHpeli(hpeli))
		{
		int imei;
		CP cp;

		Assert(vhmes != hNil);

		imei = LOWORD(hpeli->l);


		if (!PmeiImei(imei)->fRestartable || !fCanCont || 
				!FOnlyHeli(heli))
			{
			FreeEditMacro(imei);
			fCanCont = fFalse;
			}

		switch (rerr)
			{
		default:
			if ((cp = CpMacroLineIb(imei, (uns) lib)) == cpNil)
				{
				FreeEditMacro(imei);
				fCanCont = fFalse;
				}
			else
				HilightMacroPara(imei, cp);
			/* FALL THROUGH */

		case rerrNil: /* end of macro */
			if ((*vhmes)->fAnimate)
				{
				/* Remove animation highlighting */
				KillSelMacro();
				}
			break;

		case rerrSuspend: /* single step */
			if (selCur.doc == selMacro.doc)
				{
				Assert(selMacro.doc != docNil);
				Select(&selCur, 
						selMacro.cpFirst, selMacro.cpLim);
				selCur.fHidden = fFalse;
				selMacro.doc = docNil;
				}
			break;
			}

		/* Make sure the macro that would be continued is
			the current one! */
		if (fCanCont && imei != (*vhmes)->imeiCur)
			SetImeiCur(imei);
		}

	if (!fCanCont)
		CancelDyadic();
	
	return fCanCont;
}


/* M O D E  E R R O R */
/* This function is used by GetInfoElx() when the command requested is
illegal given the current state of Opus (i.e. an fNeedsDoc command when
there is no current document).  It is also used by various commands to
beep, or signal an error if el is running.  This will NOT return if el
is active! */
/* %%Function:ModeError %%Owner:bradch */
void ModeError()
{
	if (fElActive)
		RtError(rerrModeError);
	else
		Beep();
}
