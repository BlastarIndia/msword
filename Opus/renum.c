/*      Renum.c
				Author: Dave Bourne

		Routines that can be called from the outside world

		RenumParas(cpFirst,cpLim,fAll,rpm,hstStartAt,hstFormat)
				Renumbers paragraphs within the passed limits.  If fAll is
				fTrue, those paragraphs without numbers will have numbers
				assigned to them also.  If fAll is fFalse, only those para-
				graphs already numbered will be renumbered.  rpm is the
				the renumber para mode, i.e. single numbers per level,
				multiple numbers per level, or learn by example.
				hstStartAt is a handle to a string to be parsed as the starting
				number.  hstFormat is a handle to a string to be parsed as
				information to be placed in mplevnpp.
*/

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "props.h"
#include "doc.h"
#include "sel.h"
#include "disp.h"
#include "format.h"
#include "ch.h"
#include "renum.h"
#include "prompt.h"
#include "message.h"
#include "strtbl.h"
#include "wininfo.h"
#include "opuscmd.h"
#include "field.h"
#include "debug.h"
#include "error.h"
#include "idle.h"

#include "idd.h"
#include "sdmdefs.h"
#include "sdmver.h"
#include "sdm.h"
#include "sdmtmpl.h"
#include "sdmparse.h"

#include "renum.hs"
#include "renum.sdm"


#ifdef PROTOTYPE
#include "renum.cpt"
#endif /* PROTOTYPE */

extern CP    vcpFetch;
extern int   wwCur;
extern CHAR HUGE  *vhpchFetch;
extern int   vccpFetch;
extern int   vfSeeSel;
extern struct SEL  selCur;
extern struct CA   caPara;
extern struct CHP  vchpFetch;
extern struct PAP  vpapFetch;
extern struct DOD  **mpdochdod[];
extern struct WWD  **hwwdCur;
extern struct UAB  vuab;
extern struct MERR vmerr;
extern int         wwCur;
extern CHAR        szEmpty[];
extern IDF         vidf;

extern CP  CpLimBlock();


/* FUTURE: it would be cool if we could specify the starting punctuation for
			each level so we could do outline numbering according to the rules
			in the Chicago Manual of Style.  Note, we kludged this in the
			auto-renum case, but have no work-around for manual. (davidbo) */
csconst CHAR mprpfM1stFormat [][] =
{
	/* legal */     St("1.1.1.1.1.1.1.1.1."),
			/* outline */   St("I.A.1.a)1)a)i)a)i)"),
			/* sequence */  St("1.1.1.1.1.1.1.1.1.")
};


csconst int mprpfM1flt [] = { 
	fltSeqLevLeg, fltSeqLevOut, fltSeqLevNum };


/* state => formatting map...depends on order of rps codes in renum.h */
csconst char mprpsnfc[6] =
{
	nfcNil, nfcArabic, nfcUCLetter, nfcLCLetter, nfcUCRoman, nfcLCRoman
};


/* %%Function:CmdRenumParas %%Owner:davidmck */
CMD CmdRenumParas(pcmb)
CMB * pcmb;
{
	int tmc;
	int fAll;
	int rpf;
	CP cpLim, cpFirst;
	CABRENUMPARAS *pcabRp;

	if (FCmdFillCab())
		{
		if (!FFillCabRenum(pcmb->hcab))
			return cmdNoMemory;
		}

	if (FCmdDialog())
		{
		CHAR	dlt[sizeof(dltRenumParas)];

		BltDlt(dltRenumParas, dlt);

		if (TmcOurDoDlg(dlt, pcmb) == tmcCancel)
			return cmdCancelled;
		}

	if (FCmdAction())
		{
		TurnOffSel(&selCur);
		cpFirst = selCur.cpFirst;
		if (selCur.fBlock)
			Select(&selCur, cpFirst, CpLimBlock());
		if (!selCur.fIns)
			cpLim = selCur.cpLim;
		else
			{
			cpFirst = cp0;
			cpLim = CpMacDocEdit(selCur.doc);
			}

		pcabRp = (CABRENUMPARAS *) *pcmb->hcab;
		if (pcabRp->rpp == rppRemove)
			{
			UnnumberParas(cpFirst, cpLim);
			return cmdOK;
			}
		else
			{
			CHAR szFormat[cchMaxSz];
			GetCabSz(pcmb->hcab,szFormat,cchMaxSz,Iag(CABRENUMPARAS,hszFormat));
			rpf = IdFromSzgSz (szgRenumFormat, szFormat);
			}

		if (pcabRp->fManual)
			{
			int rpm;
			if (rpf != rpfOther && rpf != rpfLearn)
				{
				CHAR st[cchMaxSz];
				CopyCsSt(mprpfM1stFormat[rpf-1], st);
				/* Heap movement */
				if (!FSetCabSt(pcmb->hcab, st, Iag(CABRENUMPARAS, hszFormat)))
					return cmdNoMemory;
				pcabRp = (CABRENUMPARAS *) *pcmb->hcab;
				if (rpf == rpfLegal)
					pcabRp->fShowAllLev = fTrue;
				}
			rpm = rpf == rpfLearn ? rpmLearn : pcabRp->fShowAllLev;
			RenumParas(cpFirst, cpLim, pcabRp->rpp==rppAll, rpm,
					pcabRp->hszStartAt, pcabRp->hszFormat);
			}
		else  /* automatic */			
			{
			if (rpf == rpfOther || rpf == rpfLearn)
				rpf = PwwdWw (wwCur)->fOutline ? rpfOutline : rpfLegal;
			AutoRenumParas(cpFirst, cpLim, pcabRp->rpp==rppAll, rpf);
			}
		}

	return cmdOK;
}


/* %%Function:FFillCabRenum %%Owner:davidmck */
FFillCabRenum(hcab)
HCAB hcab;
{
	CABRENUMPARAS *pcabRp;
	int cch, lev, clev, n, nfc;
	int rpf = rpfOther;
	CP cpCur;
	char *pchStartAt, *pch, chPunc;
	char szFormat[cchMaxSz], szStartAt[cchMaxSz], sz[cchMaxSz];

	pcabRp = (CABRENUMPARAS *) (*hcab);
	*szFormat = *szStartAt = 0;
	CachePara(selCur.doc, selCur.cpFirst);
	cpCur = caPara.cpFirst;
	pcabRp->fManual = fFalse;
	if (FSkipWhiteSpace(&cpCur, &cpCur) && 
			FFillBuffer(sz, cpCur, &cch, &clev, &rpf))
		{
		if (rpf != rpfOther)
			{
			CchSzFromSzgId (szFormat, szgRenumFormat, rpf, cchMaxSz);
			szFormat [0] = ChUpper (szFormat[0]);
			}
		else
			{
			pcabRp->fManual = fTrue;
			pch = sz;
			pchStartAt = szStartAt;
			if (FIsPunc(*pch))
				pch++;
			for (lev = 0; lev < clev; lev++)
				{
				ScanNumList(&pch, &nfc, &n, &chPunc);
				if (n == nNil)
					break;
				pchStartAt += CchLongToRgchNfc((LONG)(unsigned) n, pchStartAt, nfcArabic, cchMaxLong);
				*pchStartAt++ = '.';
				}
			*pchStartAt = 0;
			}
		}

	if (szStartAt[0])
		{
		CchSzFromSzgId (szFormat, szgRenumFormat, rpfLearn, cchMaxSz);
		szFormat [0] = ChUpper (szFormat[0]);
		}
	pcabRp->rpp = szStartAt[0] ? rppNumbered : rppAll;
	pcabRp->fShowAllLev = fFalse;
	if (!szStartAt[0] && rpf == rpfOther)
		{
		szStartAt[0] = '1';
		szStartAt[1] = 0;
		}
	FSetCabSz(hcab, szStartAt, Iag(CABRENUMPARAS, hszStartAt));
	FSetCabSz(hcab, szFormat, Iag(CABRENUMPARAS, hszFormat));
	/* fMemFail will be true if FSetCabSt fails */
	return (!vmerr.fMemFail);
}


/* %%Function:FInvalidRPNumTmc %%Owner:davidmck */
FInvalidRPNumTmc(tmc)
int tmc;
{
	int nfc, n;
	char *pch, ch, chPunc, sz[cchMaxSz];

	Assert(tmc == tmcRPStartAt || tmc == tmcRPFormat);

	CchGetTmcText(tmc, sz, cchMaxSz);
	/* skip any white space */
	for (pch = sz; WbFromCh(*pch) == wbWhite; pch++)
		;

	/* skip optional beginning puncuation */
	if (*pch && FIsPunc(*pch))
		pch++;

	/* verify each number in string */
	for ( ; *pch; )
		{
		ScanNumList(&pch, &nfc, &n, &chPunc);
		if (n == nNil)
			break;
		}
	/* ...also okay if last ch before terminator is ' ' */
	if (!(ch = *pch++) || ch == ' ' && *pch == 0)
		return fFalse;
	return fTrue;
}


/* %%Function:FDlgRenumParas %%Owner:davidmck */
BOOL FDlgRenumParas(dlm, tmc, wNew, wOld, wParam)
DLM dlm;
TMC tmc;
WORD wNew, wOld, wParam;
{
	int fEnable, fManual, fIsEnabled, rpf, rpfMac;
	HCAB hcab;
	CABRENUMPARAS *pcabRp;
	CHAR szFormat [cchMaxSz];

	switch (dlm)
		{
	case dlmTerm:
		if (tmc == tmcOK)
			{
			int eid = 0;

			CchGetTmcText(tmcRPFormat, szFormat, cchMaxSz);
			rpf = IdFromSzgSz (szgRenumFormat, szFormat);
			if (fManual = ValGetTmc(tmcRPMode))
				{
				if (FInvalidRPNumTmc(tmcRPStartAt))
					eid = eidInvalidRPStartAt;
				if (rpf == rpfOther && FInvalidRPNumTmc(tmcRPFormat))
					eid = eid ? eidInvalidRPBoth : eidInvalidRPFormat;
				}
			if (eid)
				{
				ErrorEid(eid, "renum.c");
				SetFocusTmc(eid == eidInvalidRPFormat ?
						tmcRPFormat : tmcRPStartAt);
				return fFalse;
				}
			}
		break;

	case dlmInit:
		hcab = HcabDlgCur();  /* a macro; should never fail */
		Assert(hcab);

		pcabRp = (CABRENUMPARAS *) *hcab;
		tmc = pcabRp->rpp == rppAll ? tmcRPAll : tmcRPNumbered;
		fEnable = fTrue;
		fManual = pcabRp->fManual;
		fIsEnabled = fFalse;
		GetCabSz(hcab, szFormat, cchMaxSz, Iag(CABRENUMPARAS, hszFormat));
		rpf = IdFromSzgSz (szgRenumFormat, szFormat);
		goto lblFill;

	case dlmChange:
		if (tmc != (tmcRPFormat & ~ftmcGrouped))
			break;
		fManual = ValGetTmc(tmcRPMode);
		CchGetTmcText(tmcRPFormat, szFormat, cchMaxSz);
		rpf = IdFromSzgSz (szgRenumFormat, szFormat);
		EnableTmc(tmcRPAllLevels, fManual && rpf != rpfLegal && rpf!=rpfLearn);
		break;

	case dlmClick:
		fEnable = tmc != tmcRPRemove;
		if (tmc == tmcRPAutomatic || tmc == tmcRPManual)
			fManual = tmc == tmcRPManual;
		else
			fManual = ValGetTmc(tmcRPMode);
		fIsEnabled = FEnabledTmc(tmcRPAutomatic);

		if (((tmc == tmcRPAll || tmc == tmcRPNumbered) && fIsEnabled) ||
				(tmc == tmcRPRemove && !fIsEnabled))
			goto lblFill;     /* skip renum list fill */

		CchGetTmcText(tmcRPFormat, szFormat, cchMaxSz);

		rpf = IdFromSzgSz (szgRenumFormat, szFormat);
		rpfMac = !fEnable ? rpfMin : fManual ? rpfMax : rpfMaxAutomatic;
		FillRenumList(rpf, rpfMac, szFormat);
lblFill:
		EnableTmc(tmcRPStartAt, fEnable && fManual);
		EnableTmc(tmcRPAllLevels, fEnable && fManual
				&& rpf != rpfLegal && rpf != rpfLearn);
		EnableTmc(tmcRPFormat, fEnable);
		EnableTmc(tmcRPAutomatic, fEnable);
		EnableTmc(tmcRPManual, fEnable);

		if (rpf == rpfLearn && !fManual || !fEnable)
			SetTmcText(tmcRPFormat, szEmpty);

		CchGetTmcText(tmcRPStartAt, szFormat, cchMaxSz);
		if (!fManual)
			{
			if (szFormat[0] != 0)
				SetTmcText(tmcRPStartAt, szEmpty);
			}
		else
			{
			if (szFormat[0] == 0)
				{
				GetCabSz(PcmbDlgCur()->hcab, szFormat, cchMaxSz,
						Iag(CABRENUMPARAS, hszStartAt));
				SetTmcText(tmcRPStartAt, szFormat);
				}
			}

		if (((tmc == tmcRPAll || tmc == tmcRPNumbered) && fIsEnabled) ||
				(tmc == tmcRPRemove && !fIsEnabled))
			break;

		break;
		}
	return fTrue;
}


/* %%Function:FillRenumList %%Owner:davidmck */
FillRenumList(rpf, rpfMac, szFormat)
int rpf, rpfMac;
CHAR szFormat[];
{
	/* refill list box */
	StartListBoxUpdate(tmcRPFormat); /* empties box and starts redraw */
	for (rpf = rpfMin; rpf < rpfMac; rpf++)
		{
		CchSzFromSzgId (szFormat, szgRenumFormat, rpf, cchMaxSz);
		szFormat [0] = ChUpper (szFormat [0]);
		AddListBoxEntry(tmcRPFormat, szFormat);
		}
	EndListBoxUpdate(tmcRPFormat);
}



/* %%Function:WListRenum %%Owner:davidmck */
EXPORT WORD WListRenum(tmm, sz, isz, filler, tmc, wParam)
TMM tmm;
char * sz;
int isz;
WORD filler;
TMC tmc;
WORD wParam;
{
	HCAB	hcab;
	CHAR    szFormat [cchMaxSz];
	int     rpf, rpfMac;
	CABRENUMPARAS *pcabRp;

	/* this function is only used the first time the box is filled */
	if (tmm == tmmCount)
		{
		/* Set the initial text */
		if ((hcab = HcabDlgCur()) != NULL)
			{
			GetCabSz(hcab, szFormat, cchMaxSz, Iag(CABRENUMPARAS,hszFormat));
			rpf = IdFromSzgSz (szgRenumFormat, szFormat);

			pcabRp = (CABRENUMPARAS *) *hcab;
			rpfMac = pcabRp->fManual ? rpfMax : rpfMaxAutomatic;
			FillRenumList(rpf, rpfMac, szFormat);
			}
#ifdef BZ
		else
			CommSzSz(SzShared("WListRenum hcabDlgCur failure "), "");
#endif /* BZ */
		}
	return 0;
}










/*
*  Convert roman numeral to decimal.  The second row is for constructions such
*  as IX; when X is parsed, I has already been added, so adding 10 - 1 - 1
*  gives the correct value.
*/

	int     mpromval[2][7] = { 
	100, 500, 1, 50, 5, 10, 1000,
	80, 300, 0, 30, 3,  8,  800 	};


/*
*  NFromRoman scans a string of presumedly Roman numerals and
*  returns the value of those numerals.
*/

/* %%Function:NFromRoman %%Owner:davidmck */
NFromRoman(szRoman)
char *szRoman;
{
	int rom, valCur,valSum,valMin;
	char ch, *pch = szRoman;

	for (valSum = 0, valMin = 32000; (ch = *pch) != 0; pch++)
		{
		rom = ((ch = ChUpper(ch)) >> 2) & 3;
		rom += (ch > 'L') ? 3 : 0;
		valCur = mpromval[0][rom];
		valSum += valCur > valMin ? mpromval[1][rom] : valCur;
		valMin = valCur;
		}
	return(valSum);
}


/*
*  NFromLetter scans an alphabetic string and returns its numeric
*  value based on the following rules:
*               a,b,c,...,z ==> 1,2,3,...,26
*               aa,bb,cc,...,zz ==> 27,28,29,...,52
*               aaa,bbb,ccc,...,zzz ==> 53,54,55,...,78     etc., etc.
*  If the alphabetic string is not "well formed", -1 is returned.
*/

/* %%Function:NFromLetter %%Owner:davidmck */
NFromLetter(szLetter)
char *szLetter;
{
	int sum;
	char ch, *pch = szLetter;

	sum = (FUpper(*pch)) ? (*pch - 64) : (*pch - 96);
	ch = *pch;
	for (pch++; *pch; sum += 26, pch++)
		if (*pch != ch)
			return(nNil);
	return(sum);
}


/*
*  Returns fTrue if "ch" is an upper case Roman numeral.
*/

/* %%Function:FIsUCRoman %%Owner:davidmck */
FIsUCRoman(ch)
register char ch;
{
	switch (ch)
		{
	case 'I':
	case 'V':
	case 'X':
	case 'L':
	case 'C':
	case 'D':
	case 'M':
		return(fTrue);
		}
	return(fFalse);
}


/*
*  Returns fTrue if "ch" is a lower case Roman numeral.
*/

/* %%Function:FIsLCRoman %%Owner:davidmck */
FIsLCRoman(ch)
register char ch;
{
	switch (ch)
		{
	case 'i':
	case 'v':
	case 'x':
	case 'l':
	case 'c':
	case 'd':
	case 'm':
		return(fTrue);
		}
	return(fFalse);
}


/*
*  Returns fTrue if "ch" is a punctuation character, generally any
*  printable character other than a number or digit.
*/

/* %%Function:FIsPunc %%Owner:davidmck */
FIsPunc(ch)
register char ch;
{
	return (WbFromCh(ch) != wbText);
}


/*
*  FRareRoman returns fTrue if the character pointed to by "pch"
*  is a rare Roman numeral.  A rare number is defined as a single,
*  large Roman numeral.  The point is "C." and "D." are more likely
*  to be alphabetic numbers whereas "I.", "V." and "X." are more
*  likely to be Roman numbers.
*/

/* %%Function:FRareRoman %%Owner:davidmck */
FRareRoman(pch)
register char *pch;
{
	register char chNext = *(pch+1);

	if (FIsPunc(chNext) || WbFromCh(chNext) == wbWhite)
		switch (*pch)
			{
		case 'M':
		case 'm':
		case 'D':
		case 'd':
		case 'C':
		case 'c':
		case 'L':
		case 'l':
			return(fTrue);
			}
	return(fFalse);
}


/*
*  RpsFromCh returns an rps based on the passed ch.  This rps, which
*  stands for "renumber paragraph state", is used during lexical
*  scanning to determine state transistions of the lexical scanning
*  graph used in RenumParas.
*/

/* %%Function:RpsFromCh %%Owner:davidmck */
RpsFromCh(ch)
char    ch;
{
	if (FDigit(ch))
		return(rpsArabic);
	else  if (FIsPunc(ch))
		return(rpsPunc);
	else  if (FUpper(ch))
		return((FIsUCRoman(ch)) ? rpsUCRoman : rpsUCLetter);
	else  if (FLower(ch))
		return((FIsLCRoman(ch)) ? rpsLCRoman : rpsLCLetter);
	else
		return(rpsEnd);
}


/*
*  ScanNumList scans a number list that has been accumulated in some sz.
*  On entry, pch points to the first character of a number.  On
*  exit, pch points to the first character of the next number
*  (for the next time around), while nfc, n, and chPunc refer
*  to the number just scanned.  n and nfc will be nNil while chPunc
*  will be undefined if there are no numbers to be scanned.
*/

/* %%Function:ScanNumList %%Owner:davidmck */
ScanNumList(ppch, pnfc, pn, pchPunc)
char **ppch,*pchPunc;
int *pnfc,*pn;
{
	int rpsNext, rpsCur, nfcCur, n;
	char *pch, *pchStart, chPunc;

	pchStart = pch = *ppch;
	rpsCur = rpsStart;
	for (; ; pch++)
		{
		if (rpsCur == rpsStart && FIsPunc(*pch))
			rpsNext = rpsCur = rpsEnd;
		else
			rpsNext = (WbFromCh(*pch) == wbWhite) ? rpsPunc : RpsFromCh(*pch);

		if (rpsCur == rpsStart)
			{
			if (FRareRoman(pch))
				rpsNext = (rpsNext == rpsUCRoman) ? rpsUCLetter : rpsLCLetter;
			if (rpsNext < rpsTextMax)
				nfcCur = mprpsnfc[rpsNext];
			rpsCur = rpsNext;
			}
		else  if (rpsNext < rpsTextMax && rpsNext != rpsCur)
			rpsCur = rpsEnd;

		if (rpsNext == rpsPunc)
			{
			rpsCur = rpsNext;
			break;
			}
		if (rpsCur == rpsEnd)
			{
			nfcCur = nfcNil;
			break;
			}
		}

	/*
	*  The recognized number string, be it Roman, Arabic, or
	*  English, is converted into its numeric equivalent.
	*/

	if (rpsCur == rpsPunc)
		{
		/* Remember terminating character */
		chPunc = *pch;
		/* terminate number string so conversion will work */
		*pch = 0;
		}
	/* else rpsCur == rpsEnd, nfcCur == nfcNil, therfore n = nNil */
	else
		chPunc = chNix;

	switch (nfcCur)
		{
	case nfcArabic:
		n = WFromSzNumber(pchStart);
		break;
	case nfcUCRoman:
	case nfcLCRoman:
		n = NFromRoman(pchStart);
		break;
	case nfcUCLetter:
	case nfcLCLetter:
		n = NFromLetter(pchStart);
		break;
	default:
		n = nNil;
		break;
		}
	*pnfc = (n == nNil) ? nfcNil : nfcCur;
	*pn = n;
	/* put punctuation back */
	if (rpsCur == rpsPunc)
		*pch = chPunc;
	/*
	*  Point past terminating character unless chNix, in which case
	*  at end of string and don't want to point past end!
	*/
	if (chPunc != chNix)
		pch++;
	*pchPunc = (WbFromCh(chPunc) == wbWhite) ? chNix : chPunc;
	*ppch = pch;
	return;
}


/*
*  Fill buffer sz with characters pointed to by pch and whose CP
*  is cp.  These characters are the numbers of a numbered paragraph.
*  Along the way, count how many characters are stuffed into sz and
*  also count the number of level numbers.  Returns true if a seemingly
*  valid number string is returned.
*/

/* %%Function:FFillBuffer %%Owner:davidmck */
FFillBuffer(sz, cp, pcch, pclev, prpf)
char *sz;
int *pcch, *pclev, *prpf;
CP cp;
{
	int ccp, fSawArabic, fChLastWasPunc, cch, clev, fFirstCh;
	int nfc, n;
	char ch, *pch;
	char HUGE *hpchFetch;

	CachePara(selCur.doc,cp);
	FetchCp(selCur.doc,cp,fcmChars + fcmProps);
	pch = sz;
	fSawArabic = fChLastWasPunc = fFalse;
	for (cch = clev = 0, fFirstCh = fTrue; ; )
		{
		if ((vchpFetch.fVanish || vchpFetch.fFldVanish) &&
				!(PwwdWw(selCur.ww)->grpfvisi.fSeeHidden || 
				PwwdWw(selCur.ww)->grpfvisi.fvisiShowAll))
			goto LFillDone;
		if (vchpFetch.fSpec)
			{
			if (fFirstCh && *vhpchFetch == chFieldBegin)
				{
				struct FLCD flcd;
				int rpfM1, ifld = IfldFromDocCp (selCur.doc, vcpFetch, fTrue);
				if (ifld != ifldNil)
					{
					GetIfldFlcd (selCur.doc, ifld, &flcd);
					for (rpfM1 = rpfMin-1; rpfM1 < rpfMaxAutomatic-1; rpfM1++)
						if (flcd.flt == mprpfM1flt [rpfM1])
							{
							*prpf = rpfM1+1;
							*pcch = flcd.dcpInst + flcd.dcpResult;
							return fTrue;
							}
					}
				}
			/* not an autonumber field */
			goto LFillDone;
			}

		ccp = CpMin((CP) vccpFetch, caPara.cpLim - vcpFetch);
		for (hpchFetch = vhpchFetch; ccp-- > 0; *pch++ = ch, fFirstCh = fFalse, cch++)
			{
			/* Overflow...hose the user */
			if (pch >= sz + cchMaxSz)
				goto LNoNum;
			ch = *hpchFetch++;
			if (FDigit(ch))
				{
				fSawArabic = fTrue;
				fChLastWasPunc = fFalse;
				}
			/*
			*  allow the last number to end with white
			*  space as well as the usual terminators iff
			*  an arabic number is somewhere in the string.
			*/
else  if (WbFromCh(ch) == wbWhite)
{
	if (!fChLastWasPunc && !fSawArabic)
		goto LNoNum;
	if (ch == chSpace || ch == chNonBreakSpace || ch == chTab)
		{
		*pch++ = ch;
		if (!fChLastWasPunc)
			cch++;
		}

	if (!fChLastWasPunc)
		clev++;
	fChLastWasPunc = fTrue;
	goto LFillDone;
}
else  if (FIsPunc(ch))
{
	if (fChLastWasPunc)
		goto LFillDone;
	if (!fFirstCh)
		clev++;
	fChLastWasPunc = fTrue;
}
else
	fChLastWasPunc = fFalse;
			}
		FetchCp(docNil, cpNil, fcmChars + fcmProps);
		}

LFillDone:
	/* terminate string! */
	*pch = 0;
	*pclev = clev;
	*pcch = cch;

	pch = sz;
	if (FIsPunc(*pch))
		pch++;
	while (*pch && WbFromCh(*pch) != wbWhite)
		{
		ScanNumList(&pch, &nfc, &n, &ch);
		if (n == nNil)
			{
LNoNum:     
			*sz = *pcch = *pclev = clev = 0;
			break;
			}
		}
	return(clev > 0);
}


/*
*  Returns fTrue if paras contains characters other than white space.
*  In this case, *pcpCur is the cp where you want to place the
*  paragraph number.
*  *pcpStart is the cp of the first non-white space character
*  Returns fFalse if para contains nothing but white space.  In this
*  case, *pcp is cp of first character of the next paragraph
*/

/* %%Function:FSkipWhiteSpace %%Owner:davidmck */
FSkipWhiteSpace(pcpStart, pcpCur)
CP *pcpStart;
CP *pcpCur;
{
	CP cpCur = *pcpCur;
	CP cpStart;
	CP cpLim = *pcpCur;
	CP cpFirst = *pcpCur;
	int ccp;
	int ifld;
	struct FLCD flcd;
	char HUGE *hpchFetch;

	/* first skip over hidden text */

	if (pcpStart == NULL)
		goto LFetchNext;

	cpFirst = cpCur = cpLim = *pcpStart;

LFetchNext:
	CachePara(selCur.doc, cpLim);
	cpLim = caPara.cpLim;
	FetchCpPccpVisible(selCur.doc, cpCur, &ccp, selCur.ww, fFalse);
	cpStart = vcpFetch;

	for ( ; ; )
		{
		FetchCpPccpVisible(selCur.doc, cpCur, &ccp, selCur.ww, fFalse);

		if ((cpCur = vcpFetch) >= cpLim || ccp == 0)
			{
LReturnF:
			*pcpCur = cpLim;
			if (pcpStart) *pcpStart = cpLim;
			return fFalse;
			}

		ifld = IfldFromDocCp(selCur.doc, vcpFetch, vchpFetch.fSpec);

		if (ifld != ifldNil)
			{
			GetIfldFlcd(selCur.doc, ifld, &flcd);
			if (cpFirst <= flcd.cpFirst)
				{
				if (pcpStart)
					cpCur = cpStart = flcd.cpFirst;
				else
					cpCur = flcd.cpFirst;
				}
			goto LReturn;
			}

		if (vchpFetch.fSpec)
			{
LReturn:
			if (pcpStart) *pcpStart = cpStart;
			*pcpCur = cpCur;
			return fTrue;
			}

		/* skip over white space (but not hidden text) */

		for (hpchFetch = vhpchFetch; ccp > 0; hpchFetch++, cpCur++, ccp--)
			if (WbFromCh(*hpchFetch) != wbWhite)
				goto LReturn;

		}

}


/*
*  RenumParas renumbers the paragraphs in the passed selection range.
*  The basic plan is this:
*
*       For each paragraph in the selection range
*               Skip white space at beginning of paragraph
*               Place numbers in buffer
*                 (generates szDel, clev, and cchDel)
*               Determine on what level to start the renumbering process
*                 (generates levIndent and levMacNew)
*               Using previously generated data, fill in holes in NPP map
*                 (maintains mplevnpp)
*               For each level number of this paragraph
*                       Fill in buffer with appropriate data from NPP map
*                         (generates szIns and cchIns from mplevnpp)
*/

/* %%Function:RenumParas %%Owner:davidmck */
RenumParas(cpFirst,cpLim,fAll,rpm,hszStartAt,hszFormat)
CP cpFirst,cpLim;
int fAll,rpm;
char **hszFormat,**hszStartAt;
{
	CP      cpStart, cpStartPara, cpLimPara, cpCur, dcp;
	int     ccp, cchDel, cchIns, cch, n, nfcCur, fWhiteSpace, fSawArabic;
	int     lev, clev, clevStart, levMac, levMacNew, levIndent;
	int     fLevel = fFalse, fLevelLast = fFalse, fUseLevel, dxaIndent;
	int     fOutline = (*hwwdCur)->fOutline;
	int     levPrev, cnpp, levT, rpf;
	int     chStart, chPunc;
	char    *pch,  *pchStart, chDivider;
	struct NPP *pnpp, *pnppIndent;
	struct CA caUndo;
	char    szIns[cchMaxSz], szDel[cchMaxSz];
	struct NPP mplevnpp[levMax];
	struct CHP chp;
	struct PLC **hplcpad;
	struct PAD pad;
	struct CA caT;
	struct PPR **hppr;
	CP cpNextRpt = 0;

	/* Set-up for Undo, long op, abort */
	StartLongOp ();
	caUndo.doc = selCur.doc;
	CachePara(selCur.doc, CpMax(cp0, cpLim - 1));
	caUndo.cpLim = CpMin(caPara.cpLim, CpMacDocEdit(selCur.doc));
	CachePara(selCur.doc, cpFirst);
	caUndo.cpFirst = caPara.cpFirst;
	AssureLegalSel(&caUndo);

	hppr = HpprStartProgressReport (mstRenumber, NULL, nIncrPercent, fTrue);
	if (!FSetUndoB1(imiRenumParas, uccPaste, &caUndo) || FQueryAbortCheck())
		goto LRenumEnd;

	/* Init npp map to nNil for all entries */
	SetWords(mplevnpp, nNil, cwNPP*levMax);
	levPrev = levMac = 0;

	/* read StartAt string, filling in mplevnpp where appropriate. */
	CchCopySz(*hszStartAt, szDel);
	pch = szDel;
	if (*pch != chNix && FIsPunc(*pch))
		pch++;
	for (lev = 0; lev < levMax; lev++)
		{
		ScanNumList(&pch, &nfcCur, &n, &chPunc);
		if (n == nNil)
			break;
		mplevnpp[lev].n = n;
		}
	if (lev > 0)
		{
		mplevnpp[lev-1].clev = clevStart = lev;
		/*
		*  When paragraph is renumbered, n will be incremented.
		*  Decrement now so the increment produces the correct value.
		*/
		mplevnpp[lev-1].n--;
		}
	else
		clevStart = 0;

	/* read Format string, filling in mplevnpp where appropriate. */
	cchDel = CchCopySz(*hszFormat, szDel);
	if (cchDel && szDel[cchDel - 1] == ' ')
		{
		chDivider = ' ';
		szDel[cchDel - 1] = 0;
		}
	else  if (rpm == rpmLearn)
		chDivider = chNix;
	else
		chDivider = chTab;

	pch = szDel;
	chStart = (FIsPunc(*pch) ? *pch++ : chNix);
	for (lev = 0; lev < levMax; lev++)
		{
		ScanNumList(&pch, &nfcCur, &n, &chPunc);
		chPunc &= 0xff;
		if (nfcCur == nfcNil)
			break;
		mplevnpp[lev].nfc = nfcCur;
		mplevnpp[lev].chEnd = (chPunc == chNix) ? '.' : chPunc;
		}

	/* fill in more of mplevnpp based on rpm */
	if (rpm != rpmLearn)
		for (lev = 0; lev < levMax; lev++)
			mplevnpp[lev].clev = (rpm == rpmSingle) ? 1 : lev+1;

	/* For each paragraph within the selection range.... */
	CachePara(selCur.doc, cpFirst);
	fUseLevel = (vpapFetch.stc <= stcLevLast && vpapFetch.stc >= stcLevMin);
	while (!vmerr.fMemFail && !FQueryAbortCheck() && !vmerr.fDiskFail)
		{
		levMacNew = levMac;
		cpStartPara = cpCur = caPara.cpFirst;
		cpLimPara = caPara.cpLim;

		/* in outline mode, ignore hidden paras */
		if (fOutline)
			{
			hplcpad = PdodMother(selCur.doc)->hplcpad;
			GetPlc( hplcpad, IInPlc( hplcpad, cpStartPara ), &pad );
			if (!pad.fShow)
				goto LNextPara;
			}
		if (!FSkipWhiteSpace(&cpStartPara, &cpCur))
			{
			if (cpCur >= cpLim)
				break;
			CachePara(selCur.doc, cpCur);
			continue;
			}

		Assert (DcpCa(&caUndo) > 0 && cpCur < caUndo.cpLim);

		if (cpCur >= cpNextRpt)
			ProgressReportPercent (hppr, caUndo.cpFirst, caUndo.cpLim, 
					cpCur, &cpNextRpt);

		if (FInTableDocCp(selCur.doc, cpCur) && vpapFetch.fTtp)
			goto LNextPara;

		/*
		*  Place the number(s) of a numbered paragraph in szDel and
		*  calculate clev, the number of level numbers for the paragraph.
		*  If szDel should overflow, the paragraph is ignored and is not
		*  renumbered.  The length of szDel, cchDel, is also calculated.
		*/

		cpStart = cpCur;
		cchDel = clev = 0;
		rpf = rpfOther;

		if (FFillBuffer(szDel, cpStart, &cchDel, &clev, &rpf))
			{
			if (rpf == rpfOther)
				{
				Assert(clev > 0);
				clev = min(clev, levMac == 0 ? levMax : min(levMax, levMac+1));
				}
			else
				clev = 1;
			}
		else  if (!fAll)
			goto LNextPara;
		else
			{
			if (fWhiteSpace = cpStart != cpStartPara)
				{
				cpStart = cpStartPara;
				/* allow a single page break before; put number after it */
				CachePara(selCur.doc, cpStartPara);
				FetchCp(selCur.doc, cpStart, fcmChars);
				if (*vhpchFetch == chSect || *vhpchFetch == chColumnBreak)
					fWhiteSpace = (++cpStart != cpStartPara + 1);
				}
			*szDel = 0;
			cchDel = 0;
			}

		/*
		*  Determine what level to start the renumbering process based
		*  on either the style code of the paragraph (vpapFetch.stc),
		*  or how much the paragraph is indented (vpapFetch.dxaLeft).
		*  If fLevel is true, our dxa's are stcLev based, else dxaIndent
		*  based.  This is wrong if renumbering starts with body text
		*  and then hits stcLev based paragraphs.  The fLevelLast flag
		*  is used to generate the correct dxaIndent for body text when
		*  dxa's are stcLev based.
		*/

		if (vpapFetch.stc <= stcLevLast && vpapFetch.stc >= stcLevMin)
			{
			fLevel = fLevelLast = fTrue;
			dxaIndent = abs(vpapFetch.stc - stcLevLast);
			}
		else  if (fUseLevel)
			/* don't number body text */
			goto LNextPara;
		else
			{
			dxaIndent = max(0, (fLevel) ? mplevnpp[levPrev].dxa + fLevelLast :
					vpapFetch.dxaLeft + vpapFetch.dxaLeft1);
			fLevelLast = fFalse;
			}

		if (levMac == 0)
			{
			/* first paragraph */
			if (clevStart == 0)
				clevStart = clev;
			levIndent = (clevStart == 0) ? 0 : clevStart - 1;
			levMac = levMacNew = (clevStart == 0) ? 1 : clevStart;
			}
		else  if (dxaIndent > mplevnpp[levMac-1].dxa)
			{
			/* indented beyond anything seen so far */
			Assert(levMac > 0);
			levIndent = min(levMac, levMax-1);
			levMacNew = min(levMac+1, levMax);
			}
		else
			/* outdent or match... */
			for (lev = levMac-1; ; )
				if (lev < 0)
					{
					/* outdent beyond anything seen so far...make room */
					levIndent = (rpm == rpmLearn) ? max(0, clev - 1) : 0;
					cnpp = levMax - (levIndent+1);
					blt(mplevnpp,&mplevnpp[levIndent+1],cnpp*cwNPP);
					SetWords(mplevnpp,nNil,(levIndent+1)*cwNPP);
					if (rpm != rpmLearn)
						{
						pnpp = mplevnpp;
						for (levT = 0; levT <= levIndent; levT++, pnpp++)
							pnpp->clev = (rpm==rpmSingle) ? 1 : levT+1;
						}
					levMacNew = min(levMac+levIndent+1, levMax);
					break;
					}
				else  if (mplevnpp[lev].dxa == nNil)
					{
					/* found an available slot */
					if (lev >= clev -1 || rpm != rpmLearn)
						goto LHaveLev;
					levIndent = clev - 1;
					cnpp = clev - lev - 1;
					blt(mplevnpp, &mplevnpp[cnpp], (levMax - cnpp)*cwNPP);
					SetWords(mplevnpp, nNil, cnpp*cwNPP);
					levMacNew = min(levMac + cnpp, levMax);
					break;
					}
				else  if (dxaIndent < mplevnpp[lev].dxa)
					/* keep looking */
					lev--;
				else  if (dxaIndent == mplevnpp[lev].dxa)
					{
					/* indents match */
LHaveLev:           
					levIndent = lev;
					break;
					}
				else
					{
					/* goes in between two levels...blt space for one slot */
					levIndent = lev + 1;
					cnpp = levMax - (levIndent + 1);
					pnpp = &mplevnpp[levIndent];
					if (levIndent < levMax-1)
						{
						blt(pnpp, pnpp+1, cnpp*cwNPP);
						levMacNew = min(levMac+1, levMax);
						}
					SetWords(pnpp,nNil,cwNPP);
					if (rpm != rpmLearn)
						pnpp->clev = (rpm == rpmSingle) ? 1 : levIndent+1;
					break;
					}

		/* if we moved to a higher level, reset the lower levels so they
			start counting from 1. */
		if (levIndent < levPrev)
			{
			pnpp = &mplevnpp[levIndent+1];
			for (lev = levIndent+1; lev < levMac; lev++, pnpp++)
				pnpp->n = 0;
			}

		pnppIndent = &mplevnpp [levIndent];
		if (pnppIndent->dxa == nNil)
			pnppIndent->dxa = dxaIndent;
		/* rpm == rpmLearn...other cases filled in above */
		if (pnppIndent->clev == levNil)
			pnppIndent->clev = (clev == 0) ? 1 : clev;

		/*
		*  At this point, we know levIndent for the paragraph and now
		*  must either update current values in mplevnpp or fill in
		*  any unknown values.
		*/

		pch = szDel;
		if (*pch && FIsPunc(*pch))
			pch++;
		lev = levIndent - pnppIndent->clev + 1;
		pnpp = &mplevnpp[lev];
		for (; lev <= levIndent; lev++, pnpp++)
			{
			Assert(lev >= 0 && lev < levMacNew);
			ScanNumList(&pch, &nfcCur, &n, &chPunc);
			chPunc &= 0xff;
			if (pnpp->n == nNil)
				pnpp->n = (n == nNil) ? 1 : n;
			else  if (lev == levIndent)
				pnpp->n += 1;

			/* catch bogus user input of stupidly large numbers... */
			if ((unsigned)pnpp->n > 65530)
				pnpp->n = 1;

			if (lev == levIndent && pnpp->chEnd == chNil)
				pnpp->chEnd = (!FIsPunc(chPunc) || n == nNil) ? '.' : chPunc;

			if (pnpp->nfc == nfcNil)
				pnpp->nfc = (nfcCur == nfcNil) ? nfcArabic : nfcCur;
			}

		if (chDivider == chNix)
			{
			chPunc = (chPunc == chNix) ? *--pch : *pch;
			if (WbFromCh(chPunc) == wbWhite)
				chDivider = chPunc;
			else
				chDivider = chTab;
			}

		/*
		*  Build a new number list for the paragraph.  This is a
		*  simple loop since the previous action block guarantees
		*  that there is valid data in all the relevant locations of
		*  mplevnpp.  The new number list is built in szIns and the
		*  length of szIns, cchIns, is calculated.
		*/

LGenNum:
		pch = szIns;
		cchIns = 0;
		if (chStart != chNix)
			{
			*pch++ = chStart;
			cchIns++;
			}
		fSawArabic = fFalse;
		lev = levIndent - pnppIndent->clev + 1;
		pnpp = &mplevnpp[lev];
		for (; lev <= levIndent; lev++, pnpp++)
			{
			Assert(lev >= 0 && lev < levMacNew);
			cch = CchLongToRgchNfc((LONG)(unsigned) pnpp->n,pch,pnpp->nfc,cchMaxLong);
			if (pnpp->nfc == nfcArabic)
				fSawArabic = fTrue;
			pch += cch;
			cchIns += cch;
			chPunc = (lev != levIndent) ? '.' : pnpp->chEnd;
			if (chPunc != chNix)
				{
				*pch++ = chPunc == chNil ? '.' : chPunc;
				cchIns++;
				}
			}

		/* Fix last terminator if user specified format breaks the rules! */
		if (chPunc == chNix)
			{
			if (fSawArabic)
				{
				chPunc = chDivider;
				fWhiteSpace = fTrue;
				}
			else
				chPunc = '.';
			*pch++ = chPunc;
			cchIns++;
			}

		/*
		*  If a non-numbered paragraph has no leading white space,
		*  insert a space to separate the new number list from the
		*  paragraph text.
		*/
		if (clev == 0 && !fWhiteSpace)
			{
			*pch++ = chDivider;
			cchIns++;
			}
		*pch = 0;

		/*
		*  Replace old paragraph numbers with new paragraph numbers and
		*  don't forget to adjust the local cp vars.
		*/

		/*  get the correct insertion chp */
		GetPchpDocCpFIns (&chp, selCur.doc, cpStart, fTrue, selCur.ww);
		chp.fRMark = PdodMother(selCur.doc)->dop.fRevMarking;
		if (!FDelete( PcaSet(&caT, selCur.doc, cpStart, cpStart + cchDel )) ||
				!FInsertRgch(selCur.doc, cpStart, szIns, cchIns, &chp, 0))
			break;
		dcp = cchIns - cchDel;
		cpLim += dcp;
		cpLimPara += dcp;
		caUndo.cpLim += dcp;

/*
*  If not at the end of the selecton range, cache the next paragraph.
*/

LNextPara:
		levMac = levMacNew;
		levPrev = levIndent;
		if (cpLimPara >= cpLim)
			break;
		if (!fOutline)
			{
			/* Skip to the next visible para */
			GetCpFirstCpLimDisplayPara(selCur.ww, selCur.doc, cpCur, &cpCur, &cpLimPara);
			}
		CachePara(selCur.doc, cpLimPara);
		}

	Select(&selCur,caUndo.cpFirst, caUndo.cpLim);
	SetUndoAfter( &caUndo );

/* The RenumEnd label is for quick exits from unrecoverable errors */
LRenumEnd:
	ChangeProgressReport (hppr, 100);
	StopProgressReport (hppr, pdcRestoreImmed);
	vfSeeSel = fTrue;
	EndLongOp (fFalse);
}


/*
*  UnnumberParas removes number from the paragraphs in the passed selection
*/

/* %%Function:UnnumberParas %%Owner:davidmck */
UnnumberParas(cpFirst, cpLim)
CP cpFirst, cpLim;
{
	CP      cpStart, cpLimPara, cpCur, ccpT;
	int     cchDel, clev, rpf;
	int     fOutline = (*hwwdCur)->fOutline;
	struct PLC **hplcpad;
	struct PAD pad;
	struct CA caUndo;
	struct CA caT;
	char    szDel[cchMaxSz];
	struct PPR **hppr;
	CP cpNextRpt = 0;

	/* Set-up for Undo, long op, abort */
	StartLongOp ();
	caUndo.doc = selCur.doc;
	CachePara(selCur.doc, CpMax(cp0, cpLim - 1));
	caUndo.cpLim = CpMin(caPara.cpLim, CpMacDocEdit(selCur.doc));
	CachePara(selCur.doc,cpFirst);
	caUndo.cpFirst = caPara.cpFirst;
	AssureLegalSel(&caUndo);

	hppr = HpprStartProgressReport (mstRenumber, NULL, nIncrPercent, fTrue);
	if (!FSetUndoB1(imiRenumParas, uccPaste, &caUndo) || FQueryAbortCheck())
		goto LUnnumEnd;

	/* For each paragraph within the selection range.... */
	CachePara(selCur.doc,cpFirst);
	while (!vmerr.fMemFail && !FQueryAbortCheck() && !vmerr.fDiskFail)
		{
		cpStart = caPara.cpFirst;
		cpLimPara = caPara.cpLim;
		/* in outline mode, ignore hidden paras */
		if (fOutline)
			{
			hplcpad = PdodMother(selCur.doc)->hplcpad;
			GetPlc( hplcpad, IInPlc( hplcpad, cpStart), &pad );
			}
		if ((!fOutline || pad.fShow) &&
				FSkipWhiteSpace(&cpStart, &cpStart) &&
				FFillBuffer(szDel, cpStart, &cchDel, &clev, &rpf))
			{
			/* take number and all following white space */
			cpCur = cpStart + cchDel;

			Assert (DcpCa(&caUndo) > 0 && cpCur <= caUndo.cpLim);

			if (cpCur >= cpNextRpt)
				ProgressReportPercent (hppr, caUndo.cpFirst, caUndo.cpLim, 
						cpCur, &cpNextRpt);

			if (cpCur > cpLimPara)
				{
				CachePara(selCur.doc, cpCur);
				cpLimPara = caPara.cpLim;
				}

			FSkipWhiteSpace(NULL, &cpCur);

			if (!FLegalSel(selCur.doc, cpStart, cpCur))
				cpCur = cpStart+cchDel;

			ccpT = ccpEop;
			if (ChFetch(selCur.doc, cpLimPara - 1, fcmChars) == chSect)
				ccpT = ccpSect;

			cchDel = (int) (CpMin(cpCur, cpLimPara - ccpT) - cpStart);

#ifdef PCJ
			CommSzLong(SzShared("deleting: cpStart = "), cpStart);
			CommSzNum(SzShared("\tcchDel = "), cchDel);
#endif /* PCJ */

			if (!FDelete(PcaSet(&caT, selCur.doc, cpStart, cpStart + cchDel)))
				break;
			caUndo.cpLim -= cchDel;
			cpLim -= cchDel;
			cpLimPara -= cchDel;
			}

		if (cpLimPara >= cpLim)
			break;
		CachePara(selCur.doc, cpLimPara);
		}

	Select(&selCur,caUndo.cpFirst, caUndo.cpLim);
	SetUndoAfter(&caUndo);
LUnnumEnd:
	ChangeProgressReport (hppr, 100);
	StopProgressReport (hppr, pdcRestoreImmed);
	vfSeeSel = fTrue;
	EndLongOp (fFalse);
}


/* A U T O  R E N U M  P A R A S */

/* %%Function:AutoRenumParas %%Owner:davidmck */
AutoRenumParas (cpFirst, cpLim, fAll, rpfNew)
CP cpFirst, cpLim;
BOOL fAll;
int rpfNew;

{
	CP      cpStart, cpStartPara, cpLimPara, cpCur, dcp, dcpT;
	int     ccp, cchDel, cchKwd, clev, flt;
	int     fAddTab, rpf;
	int     fOutline = (*hwwdCur)->fOutline;
	int		fSeeHidden;
	struct CA caUndo;
	char    sz[cchMaxSz], szKeyword[cchMaxFieldKeyword+2];
	struct CHP chp;
	struct PLC **hplcpad;
	struct PAD pad;
	struct CA caT;
	struct PPR **hppr;
	CP cpNextRpt = 0;

	/* Set-up for Undo, long op, abort */
	StartLongOp ();
	caUndo.doc = selCur.doc;
	CachePara(selCur.doc, CpMax(cp0, cpLim - 1));
	caUndo.cpLim = CpMin(caPara.cpLim, CpMacDocEdit(selCur.doc));
	CachePara(selCur.doc,cpFirst);
	caUndo.cpFirst = caPara.cpFirst;
	AssureLegalSel(&caUndo);
	fSeeHidden = PwwdWw(selCur.ww)->grpfvisi.fSeeHidden || 
				PwwdWw(selCur.ww)->grpfvisi.fvisiShowAll;

	hppr = HpprStartProgressReport (mstRenumber, NULL, nIncrPercent, fTrue);
	if (!FSetUndoB1(imiRenumParas, uccPaste, &caUndo) || FQueryAbortCheck())
		goto LAutoRenumEnd;

	Assert (rpfNew >= rpfMin && rpfNew < rpfMaxAutomatic);
	flt = mprpfM1flt [rpfNew-1];
	GetFltKeyword (flt, szKeyword);
	cchKwd = CchSz (szKeyword)-1;
	szKeyword [cchKwd] = chTab;

	/* For each paragraph within the selection range.... */
	CachePara(selCur.doc, cpFirst);
	while (!vmerr.fMemFail && !FQueryAbortCheck() && !vmerr.fDiskFail)
		{
		cpStartPara = cpCur = caPara.cpFirst;
		cpLimPara = caPara.cpLim;

		/* in outline mode, ignore hidden paras */
		if (fOutline)
			{
			hplcpad = PdodMother(selCur.doc)->hplcpad;
			GetPlc( hplcpad, IInPlc( hplcpad, cpStartPara ), &pad );
			if (!pad.fShow)
				goto LNextPara;
			}

		if (!FSkipWhiteSpace(&cpStartPara, &cpCur))
			{
			if (cpCur >= cpLim)
				break;
			CachePara(selCur.doc, cpCur);
			continue;
			}

		Assert (DcpCa(&caUndo) > 0 && cpCur < caUndo.cpLim);

		if (cpCur >= cpNextRpt)
			ProgressReportPercent (hppr, caUndo.cpFirst, caUndo.cpLim, 
					cpCur, &cpNextRpt);

		if (FInTableDocCp(selCur.doc, cpCur) && vpapFetch.fTtp)
			goto LNextPara;

		cpStart = cpCur;
		cchDel = 0;
		rpf = rpfOther;

		if (!FFillBuffer(sz,cpStart,&cchDel,&clev, &rpf))
			{
			if (!fAll)
				goto LNextPara;
			else
				{
				if (cpStart != cpStartPara)
					{
					cpStart = cpStartPara;
					/* allow a single page break before; put number after it */
					CachePara(selCur.doc, cpStartPara);
					FetchCp(selCur.doc, cpStart, fcmChars);
					if (*vhpchFetch == chSect || *vhpchFetch == chColumnBreak)
						++cpStart;
					}
				cchDel = 0;
				fAddTab = fTrue;
				}
			}
		else  if (rpf == rpfNew)
			{
			if (cpStart + cchDel > cpLimPara)
				{
				CachePara(selCur.doc, cpStart + cchDel);
				cpLimPara = caPara.cpLim;
				}
			goto LNextPara;
			}
		else
			fAddTab = fFalse;

		GetPchpDocCpFIns (&chp, selCur.doc, cpStart, fTrue/*fIns*/,
				selCur.ww);
		chp.fRMark = PdodMother(selCur.doc)->dop.fRevMarking;
		if (!FDelete( PcaSet(&caT, selCur.doc, cpStart, cpStart + cchDel )) ||
				!FInsertRgch(selCur.doc,cpStart,szKeyword,cchKwd+fAddTab,&chp,NULL))
			break;
		dcpT = cchKwd;
		FPutCpsInField (flt, selCur.doc, cpStart, &dcpT, cp0, &chp);
		dcp = cchKwd + fAddTab + 2 - cchDel;
		cpLim += dcp;
		cpLimPara += dcp;
		caUndo.cpLim += dcp;

/*
*  If not at the end of the selecton range, cache the next paragraph.
*/

LNextPara:
		if (cpLimPara >= cpLim)
			break;
		if (!fOutline)
			{
			/* Skip to the next visible para */
			GetCpFirstCpLimDisplayPara(selCur.ww, selCur.doc, cpCur, &cpCur, &cpLimPara);
			}
		CachePara(selCur.doc, cpLimPara);
		}

	Select( &selCur, caUndo.cpFirst, caUndo.cpLim );
	SetUndoAfter( &caUndo );

/* The RenumEnd label is for quick exits from unrecoverable errors */
LAutoRenumEnd:
	ChangeProgressReport (hppr, 100);
	StopProgressReport (hppr, pdcRestoreImmed);
	vfSeeSel = fTrue;
	PdodDoc(selCur.doc)->fInvalSeqLev = vidf.fInvalSeqLev = fTrue;
	EndLongOp (fFalse);
}


