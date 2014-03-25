#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "version.h"
#include "heap.h"
#include "doc.h"
#include "props.h"
#include "debug.h"
#include "error.h"
#include "sel.h"
#include "file.h"
#include "ch.h"
#include "inter.h" 
#include "field.h"
#include "ifld.h"
#include "insfield.h"

#define STRTBL
#include "fltexp.h"
#include "dde.h"
#include "renum.h"
#include "strtbl.h"
#include "strus.h"
#define cMaxArgs 50 /* Max no of arguments in a single field */ 
extern struct	MERR	vmerr;
extern struct ITR	vitr;

/* The following structure holds information for a single field as it is
	localized. It is repeatedly filled up as fields are scanned in a doc.
*/

typedef struct _translflds
	{
	int doc;
	CP rgCpFirst[cMaxArgs];
	CP rgCpLim[cMaxArgs];
	char **rghszOpt[cMaxArgs];
	int rgCch[cMaxArgs];
	int iMax;
} TRFL;

/* The following array is to be updated as more and more switches are to be
	localized.
*/
csconst struct FSITL rgfsitl[] =
{ 	/* flt,			chSwUS,					chSwLocal */
	{ fltXe,			chFldXeUSBold,			chFldXeBold 	 },
	{ fltXe,			chFldXeUSItalic,		chFldXeItalic	 },
	{ fltToc,		chFldTocUSOutline,	chFldTocOutline	 },
	{ fltToc,		chFldTocUSFields,		chFldTocFields	 },

};


#define iRgFsitlMax	4 /* no of entries in the above array */

/* 	F  T R A N S L  F I E L D S 
*
*  Translates fields, switches and arguments of a doc as per intl specs. 
*  fLocal implies whether fields are foreign or English.
*  
*		!fLocal =>	Eng -> local
*		 fLocal =>	local -> Eng
*
*  returns success flag.
*/

/* %%Function:FTranslFields %%Owner:krishnam */
BOOL FTranslFields (doc, fLocal)
int doc;
BOOL fLocal;
{
	int ifld = ifldNil;
	int fltT, cchNew;
	struct FLCD flcd;
	struct FFB ffb, ffbT;
	struct EFLT eflt;
	CHAR szKwd [cchMaxFieldKeyword+1];
	CHAR rgch [cchArgumentFetch+1], chSw, chNew;
	CHAR szOpt [cchArgumentFetch+2];
	int (*pfnCchSz)(), (*pfnId)();
	extern int CchSzFromSzgId();
	extern int CchSzEngFromSzgId();
	extern int IdFromSzgSz();
	extern int IdEngFromSzgSz();
	TRFL trfl;
	CP cpSave, cpFirstArg, cpSw;
	int focSave;

	pfnCchSz = fLocal ? CchSzEngFromSzgId : CchSzFromSzgId;
	pfnId = fLocal ? IdFromSzgSz : IdEngFromSzgSz;
	while ((ifld = IfldAfterFlcd (doc, ifld, &flcd)) != ifldNil)
		{
		if (vmerr.fMemFail || vmerr.fDiskFail)
			goto LError;

		trfl.doc = doc;
		trfl.iMax = 0;

		InitFvb (&ffb.fvb);
		SetFfbIfld (&ffb, doc, ifld);

		if (flcd.fDirty)
			flcd.flt = FltParseDocCp (doc, flcd.cpFirst, ifld, fFalse, !fLocal);

	/* translate keyword: as per specs szgFltSingle are not translated */
		if ((cchNew = (*pfnCchSz)(szKwd, szgFltNormal, flcd.flt, 
				cchMaxFieldKeyword)) > 1 &&
				!FAddArgTrfl(&ffb, doc, szKwd, cchNew-1, &trfl))
			goto LError;

	/* Now translate the arguments */
		cpFirstArg = (flcd.flt == fltExp) ? ffb.cpFirst+1 : ffb.cpFirst;

		/*  assure all switches were fetched */
		ExhaustFieldText(&ffb);
		cpSave = ffb.cpFirst;
		focSave = ffb.foc;

	/* translate non-system switches */
		while ((chSw = ChSpFetchSw(&ffb, &cpSw, fFalse /* !fSys */)) != chNil)
			{
			if ((chNew=ChItlSw(flcd.flt, chSw, fLocal)) != chNil &&
					!FAddSwTrfl(&ffb, doc, chNew, cpSw, &trfl))
				goto LError;

		/* as per specs their args need not be localized */
			}

	/* translate args of system switches */
		ffb.foc = focSave;
		ffb.cpFirst = cpSave;
		while ((chSw = ChSpFetchSw(&ffb, &cpSw, fTrue /* fSys */)) != chNil)
			{
		/* as per specs system switches need not be localized */

			bltb(&ffb, &ffbT, sizeof(struct FFB));
			InitFvbBufs(&ffbT.fvb, rgch, cchArgumentFetch, NULL, 0);

			if (!FSpFetchArgText(&ffbT))
				continue; /* for switches wo text */

			switch (chSw)
				{
		/* as per specs only the following three types of args to
			system switches need to be localized */
			case chFldSwSysNumeric:
				if ((cchNew = CchSzItlNum(szOpt, rgch, fLocal)) > 1 &&
						!FAddArgTrfl(&ffb, doc, szOpt, cchNew-1, &trfl))
					goto LError;
				break;

			case chFldSwSysDateTime:
				if ((cchNew = CchSzItlDtm(szOpt, rgch, fLocal)) > 1 &&
						!FAddArgTrfl(&ffb, doc, szOpt, cchNew-1, &trfl))
					goto LError;
				break;

			case chFldSwSysFormat:
				if ((cchNew = (*pfnCchSz)(szOpt, szgFormatFunc, 
						(*pfnId)(szgFormatFunc, rgch),
						cchArgumentFetch)) > 1)
					{
					*szOpt=(FLower(rgch[0]) ? ChLower(*szOpt):
							ChUpper(*szOpt));
					if (!FAddArgTrfl(&ffb,doc,szOpt,cchNew-1,&trfl))
						goto LError;
					}

				break;

			default:
				break;
				}
			}

	/* Now translate non-switch args */
			{
			BOOL fFetch = fFalse;
		 	int cLevelNest = 0;	/* it is assumed that for a field without syntax
											error, nestings of args will be correct for
											recognition of list separators.
										 */

			ffb.cpFirst = cpFirstArg;
			ffb.foc = focNormal;

			do
				{
				bltb(&ffb, &ffbT, sizeof(struct FFB));
				InitFvbBufs(&ffbT.fvb, rgch, cchArgumentFetch, NULL, 0);
				fFetch = FSpFetchArgText(&ffbT);

#ifdef KCM
				CommSzSz(SzShared("After FFetch : rgch= "), rgch);
#endif

				if (fFetch)
					{
					cchNew = 0;
					switch (flcd.flt)
						{
					case fltExp:
					case fltIf:
					case fltPMSkipIf:
					case fltPMNextIf:
					case fltFormula: /* the formula does not have functions,
		 					              does not recognize ambig thou seps.
											*/
							{
							char *pchLead, *pchFollow;
							char chListSepOld, chListSepNew;
							char chDecOld, chDecNew;
							char chThSepOld, chThSepNew;
							char ch;
							BOOL fScanFn = flcd.flt != fltFormula;
							BOOL fAmbig;

							if (vitr.fInvalidChSetting)
								break;

							chListSepOld = fLocal ? vitr.chList : chListUS;
							chListSepNew = fLocal ? chListUS : vitr.chList;
							chDecOld=fLocal ? vitr.chDecimal : chDecimalUS;
							chDecNew=fLocal ? chDecimalUS : vitr.chDecimal;
							chThSepOld = fLocal ? vitr.chThousand : chThousSepUS;
							chThSepNew = fLocal ? chThousSepUS : vitr.chThousand;

							Assert(!FAlpha(chDecOld));
							Assert(!FAlpha(chDecNew));
							Assert(!FAlpha(chThSepOld));
							Assert(!FAlpha(chThSepNew));
							Assert(chThSepOld != chDecOld);
							Assert(chListSepOld != chDecOld);
							Assert(CchSz(rgch) >= 0);
							Assert(CchSz(rgch) <= sizeof(szOpt));

							bltb(rgch, szOpt, CchSz(rgch));
							pchLead = pchFollow = szOpt;
							fAmbig = (chListSepOld==chThSepOld && flcd.flt!=fltFormula);
							while (*pchLead != '\0')
								{
								Assert(pchFollow <= pchLead);

								ch = *pchLead;
								if (ch == '(' || ch == ')')
									{
									if (fScanFn && 
									  	!FTranslFn(szOpt, &pchLead, pchFollow, pfnCchSz,
											pfnId))
										goto LSkip;
									++pchLead;
									pchFollow = pchLead;
									if (ch == '(')
										{
										++cLevelNest;
										fScanFn = flcd.flt != fltFormula;
										}
									else
										{
										--cLevelNest;
										fScanFn = fFalse;
										}
									continue;
									}

								if (cLevelNest > 0 && ch == chListSepOld)
									{
									if  (fScanFn && !FTranslFn(szOpt, &pchLead, pchFollow,
											 pfnCchSz, pfnId))
										goto LSkip;

									if (fAmbig && FDigit(*(pchLead+1)) && FDigit(*(pchLead+2)) && FDigit(*(pchLead+3)))
										{
										*pchLead = chThSepNew;
										pchLead += 4;
										pchFollow = pchLead;
										fScanFn = fFalse;
										continue;
										}

									*pchLead = chListSepNew;
									++pchLead;
									pchFollow = pchLead;
									fScanFn = flcd.flt != fltFormula;
									continue;
									}

								if (FAlpha(ch) || FSpace(ch))
									{
									++pchLead;
									continue;
									}

								if (ch == chDecOld && FDigit(*(pchLead-1)))
									*pchLead = chDecNew;
								if (ch == chThSepOld && FDigit(*(pchLead-1)))
									*pchLead = chThSepNew;

								fScanFn = FInfixCh(ch) && flcd.flt != fltFormula;
								++pchLead;
								pchFollow = pchLead;
								}
							if (pchFollow < pchLead && fScanFn &&
									!FTranslFn(szOpt,&pchLead,pchFollow,pfnCchSz,pfnId))
								goto LSkip;
							cchNew = CchSz(szOpt);
							}
						break;

					case fltDde:
					case fltDdeHot:
						cchNew = (*pfnCchSz)(szOpt, szgDdeSysTopic, 
								(*pfnId)(szgDdeSysTopic, rgch),
								cchArgumentFetch);
						break;

					case fltPrint:
						cchNew = (*pfnCchSz)(szOpt, szgPrintPs, 
								(*pfnId)(szgPrintPs, rgch),
								cchArgumentFetch);
						break;

					case fltInfo:
						cchNew = (*pfnCchSz)(szOpt, szgFltNormal,
								(*pfnId)(szgFltNormal, rgch),
								cchArgumentFetch);
						break;

					default:
				/* from the list in strtbl.h, it does not
					appear that other flt's need to be
					considered. */
						break;
						}

					if (cchNew > 1)
						{
						if (!FAddArgTrfl(&ffb, doc, szOpt, cchNew-1, &trfl))
							goto LError;
						}
					else
LSkip:
						SkipArgument(&ffb);
					}
				}
			while (fFetch);
			}

		if (!FReplNewText(&trfl))
			goto LError;

		fltT = flcd.flt;
		GetIfldFlcd (doc, ifld, &flcd);
		flcd.flt = fltT;
		SetFlcdCh (doc, &flcd, chFieldBegin);

		DirtyDoc (doc);
		}


	/* translate sub-docs */
	if (!PdodDoc (doc)->fShort)
		{
		if (PdodDoc (doc)->docFtn != docNil &&
				!FTranslFields (PdodDoc (doc)->docFtn, fLocal))
			goto LError;

		if (PdodDoc (doc)->docAtn != docNil &&
				!FTranslFields (PdodDoc (doc)->docAtn, fLocal))
			goto LError;

		if (PdodDoc (doc)->docHdr != docNil &&
				!FTranslFields (PdodDoc (doc)->docHdr, fLocal))
			goto LError;

		if (PdodDoc (doc)->fDot)
			{
			if (PdodDoc (doc)->docGlsy != docNil &&
					!FTranslFields (PdodDoc (doc)->docGlsy, fLocal))
				goto LError;

			if (PdodDoc (doc)->docMcr != docNil &&
					!FTranslFields (PdodDoc (doc)->docMcr, fLocal))
				goto LError;
			}
		}

	return fTrue;

LError:
	return fFalse;
}


/* F  A D D  A R G  T R F L */
/* adds details of a field argument to the TRFL */
/* returns success flag. */
/* %%Function:FAddArgTrfl %%Owner:krishnam */
BOOL FAddArgTrfl(pffb, doc, sz, cch, ptrfl)
struct FFB *pffb;
int doc;
char sz[];
int cch;
TRFL *ptrfl;
{
	struct CA ca;
	char **hsz;

	ca.doc = doc;
	if (!FFetchArgExtents (pffb, &ca.cpFirst, &ca.cpLim, fTrue))
		{
		Assert (fFalse);
		return fFalse;
		}

	AssureLegalSel(&ca);
	Assert(ptrfl->iMax < cMaxArgs - 1);

	ptrfl->rgCpFirst[ptrfl->iMax] = ca.cpFirst;
	ptrfl->rgCpLim[ptrfl->iMax] = ca.cpLim;
	if ((hsz = HszCreate(sz)) != hNil)
		ptrfl->rghszOpt[ptrfl->iMax] = hsz;
	else
		return fFalse;
	ptrfl->rgCch[ptrfl->iMax] = cch;
	ptrfl->iMax++;

	return fTrue;
}



/* F  A D D  S W  T R F L */
/* adds details of a switch to a TRFL */
/* returns success flag. */

/* %%Function:FAddSwTrfl %%Owner:krishnam */
BOOL FAddSwTrfl(pffb, doc, chSw, cp, ptrfl)
struct FFB *pffb;
int doc;
char chSw;
CP cp;
TRFL *ptrfl;
{
	char sz[2];
	char **hsz;

	Assert(ptrfl->iMax < cMaxArgs - 1);

	sz[0] = chSw;
	sz[1] = '\0';

	ptrfl->rgCpFirst[ptrfl->iMax] = cp;
	ptrfl->rgCpLim[ptrfl->iMax] = cp+1;
	if ((hsz = HszCreate(sz)) != hNil)
		ptrfl->rghszOpt[ptrfl->iMax] = hsz;
	else
		return fFalse;
	ptrfl->rgCch[ptrfl->iMax] = 1;
	ptrfl->iMax++;

	return fTrue;
}


/* F  R E P L  N E W  T E X T */
/* Inserts translated text into doc. Dump out the TRFL for a field. */
/* returns success flag. */

/* %%Function:FReplNewText %%Owner:krishnam */
BOOL FReplNewText(ptrfl)
TRFL *ptrfl;
{
	FC fcIns;
	struct CHP chp;
	int i, icp, doc, cch, rgSortIcp[cMaxArgs];
	CP cpF, cpL;
	struct CA ca;
	char rgch[cchArgumentFetch + 1];
	int dcpAdj = 0;


	for (i=0; i < ptrfl->iMax; ++i)
		rgSortIcp[i]=i;
	SortIcp(ptrfl, rgSortIcp);
	doc = ptrfl->doc;
	for (i=0; i < ptrfl->iMax; ++i)
		{
		icp = rgSortIcp[i];
		cpF = ptrfl->rgCpFirst[icp];
		cpL = ptrfl->rgCpLim[icp];

		Assert(cpL >= cpF);

		GetPchpDocCpFIns (&chp, doc, cpF, fFalse, wwNil);

		if (!FNewChpIns (doc, cpF, &chp, stcNil))
			return fFalse;

		cch = ptrfl->rgCch[icp];
		bltb(*(ptrfl->rghszOpt[icp]), rgch, cch);
		fcIns = FcAppendRgchToFn(fnScratch, rgch, cch);
		ca.doc = doc;
		ca.cpFirst = cpF+dcpAdj;
		ca.cpLim = cpL+dcpAdj;
		if (vmerr.fDiskFail || !FReplace(&ca, fnScratch, fcIns, (FC)cch))
			return fFalse;
		dcpAdj += cch - (int)(cpL - cpF);
		FreeH(ptrfl->rghszOpt[icp]);
		}

	return fTrue;
}


/* C C H  S Z  I T L  N U M */
/* translates the elements of a numeric pic as per specs. */
/* returns no of chars in the resulting string including null terminator. */

/* %%Function:CchSzItlNum %%Owner:krishnam */
int CchSzItlNum(sz, rgch, fLocal)
char sz[], rgch[];
BOOL fLocal;
{
	char chThOld, chThNew;
	char chDecOld, chDecNew;
	char rgchCurOld[cchMaxCurrency], rgchCurNew[cchMaxCurrency];
	int cchCurOld, cchCurNew, ich;
	char *pch, *psz;
	BOOL fInQuote = fFalse;
	BOOL fInSequence = fFalse;

	if (vitr.fInvalidChSetting)
		return 0;

	if (fLocal)
		{
		chThOld = vitr.chThousand;
		chThNew = chThousSepUS;
		chDecOld = vitr.chDecimal;
		chDecNew = chDecimalUS;

#ifdef CONVRTCURR
		/* we do not localize $ to other currencies as that may
			involve value conversions.
		*/
		bltb(vitr.szCurrency, rgchCurOld, CchSz(vitr.szCurrency)-1);
		cchCurOld = CchSz(vitr.szCurrency) - 1;
		rgchCurNew[0]=chCurrUS;
		cchCurNew = 1;
#endif /* CONVRTCURR */

		}
	else
		{
		chThNew = vitr.chThousand;
		chThOld = chThousSepUS;
		chDecNew = vitr.chDecimal;
		chDecOld = chDecimalUS;

#ifdef CONVRTCURR
		bltb(vitr.szCurrency, rgchCurNew, CchSz(vitr.szCurrency)-1);
		cchCurNew = CchSz(vitr.szCurrency) - 1;
		rgchCurOld[0]=chCurrUS;
		cchCurOld = 1;
#endif /* CONVRTCURR */

		}

	Assert(!FAlpha(chThOld));
	Assert(!FAlpha(chThNew));
	Assert(!FAlpha(chDecOld));
	Assert(!FAlpha(chDecNew));

	for (pch = rgch, psz = sz; *pch != '\0';)
		{
		Assert(!(fInQuote && fInSequence));
		if (!fInQuote && !fInSequence)
			{
			/* check for quotes */
			if (*pch == chFldPicSequence)
				{
				*psz++ = *pch++;
				fInSequence = fTrue;
				continue;
				}
			if (*pch == chFldPicQuote)
				{
				*psz++ = *pch++;
				fInQuote = fTrue;
				continue;
				}

#ifdef CONVRTCURR
		/* convert currency */
			for (ich = 0; 
					ich < cchCurOld && !WCompChCh(*(pch+ich), rgchCurOld[ich]);
					++ich);
			if (ich == cchCurOld)
				{
				bltb(rgchCurNew, psz, cchCurNew);
				pch += ich;
				psz += cchCurNew;
				continue;
				}
#endif /* CONVRTCURR */


			/* the following char comps are direct as the separators
			or decimal notations are non-alphas.
			*/

		/* convert thousand sep */
			if (*pch == chThOld)
				{
				*psz = chThNew;
				++psz; 
				++pch;
				continue;
				}

		/* convert dec sign */
			if (*pch == chDecOld)
				{
				*psz = chDecNew;
				++psz; 
				++pch;
				continue;
				}

			*psz++ = *pch++;
			}
		else
			{
			Assert(fInQuote || fInSequence);
			if (*pch == chFldPicSequence)
				{
				fInSequence = fFalse;
				*psz++ = *pch++;
				continue;
				}
			if (*pch == chFldPicQuote)
				{
				fInQuote = fFalse;
				*psz++ = *pch++;
				continue;
				}
			*psz++ = *pch++;
			}

		}

	Assert(*pch == '\0');
	*psz = '\0';

	return (psz - sz + 1);
}





/* C C H  S Z  I T L  D T T M */
/* translates the elements of a date time pic as per the specs. */
/* returns no of chars in the resulting string including null terminator. */

/* %%Function:CchSzItlDtm %%Owner:krishnam */
int CchSzItlDtm(sz, rgch, fLocal)
char sz[], rgch[];
BOOL fLocal;
{
	char chDateSepOld, chDateSepNew;
	char chTimeSepOld, chTimeSepNew;
	char chDayOld, chDayNew;
	char chMonthOld, chMonthNew;
	char chYearOld, chYearNew;
	char chHourOld, chHourNew;
	char chMinuteOld, chMinuteNew;
	int cchAmOld, cchAmNew, cchPmOld, cchPmNew, ich;
	char *pch, *psz;
	char rgchAmOld[cchMaxMornEve], rgchAmNew[cchMaxMornEve];
	char rgchPmOld[cchMaxMornEve], rgchPmNew[cchMaxMornEve];
	BOOL fInQuote = fFalse;
	BOOL fInSequence = fFalse;

	if (fLocal)
		{
		chDateSepOld = vitr.chDate;
		chDateSepNew = chDateSepUS;
		chTimeSepOld = vitr.chTime;
		chTimeSepNew = chTimeSepUS;
		bltb(vitr.rgszMornEve[iszMorn], rgchAmOld, cchMaxMornEve);
		cchAmOld = CchSz(rgchAmOld) - 1;
		bltb(szAmUS, rgchAmNew, cchMaxMornEve);
		cchAmNew = CchSz(szAmUS) - 1;
		bltb(vitr.rgszMornEve[iszEve], rgchPmOld, cchMaxMornEve);
		cchPmOld = CchSz(rgchPmOld) -1;
		bltb(szPmUS, rgchPmNew, cchMaxMornEve);
		cchPmNew = CchSz(szPmUS) - 1;
		chDayOld = chFldPicDay;
		chDayNew = chDayUS;
		chMonthOld = chFldPicMonth;
		chMonthNew = chMonthUS;
		chYearOld = chFldPicYear;
		chYearNew = chYearUS;
		chHourOld = chFldPicHour;
		chHourNew = chHourUS;
		chMinuteOld = chFldPicMinute;
		chMinuteNew = chMinuteUS;
		}
	else
		{
		chDateSepNew = vitr.chDate;
		chDateSepOld = chDateSepUS;
		chTimeSepNew = vitr.chTime;
		chTimeSepOld = chTimeSepUS;
		bltb(vitr.rgszMornEve[iszMorn], rgchAmNew, cchMaxMornEve);
		cchAmNew = CchSz(rgchAmNew) - 1;
		bltb(szAmUS, rgchAmOld, cchMaxMornEve);
		cchAmOld = CchSz(szAmUS) - 1;
		bltb(vitr.rgszMornEve[iszEve], rgchPmNew, cchMaxMornEve);
		cchPmNew = CchSz(rgchPmNew) - 1;
		bltb(szPmUS, rgchPmOld, cchMaxMornEve);
		cchPmOld = CchSz(szPmUS) - 1;
		chDayNew = chFldPicDay;
		chDayOld = chDayUS;
		chMonthNew = chFldPicMonth;
		chMonthOld = chMonthUS;
		chYearNew = chFldPicYear;
		chYearOld = chYearUS;
		chHourNew = chFldPicHour;
		chHourOld = chHourUS;
		chMinuteNew = chFldPicMinute;
		chMinuteOld = chMinuteUS;
		}

	/* probably ignorable but notes of caution */
	Assert(!FAlpha(chDateSepOld) && !FAlpha(chDateSepNew));
	Assert(!FAlpha(chTimeSepOld) && !FAlpha(chTimeSepNew));
	Assert(!FAlpha(chFldPicAMPMSep));

	for (pch = rgch, psz = sz; *pch != '\0';)
		{
		Assert(!(fInQuote && fInSequence));
		if (!fInQuote && !fInSequence)
			{
			/* check for quotes */
			if (*pch == chFldPicSequence)
				{
				*psz++ = *pch++;
				fInSequence = fTrue;
				continue;
				}
			if (*pch == chFldPicQuote)
				{
				*psz++ = *pch++;
				fInQuote = fTrue;
				continue;
				}

		/* convert AM/PM strings */
		/* The check is as follows:
			(1) upper/lower case of the first char of AM/PM string
				matches with current char.
			(2) separator at distance x chars away from the 
				starting char but before the termination of string.
			(3) there are at least x chars after the separator and 
				before the termination of the string.
			(4) 2x+1 chars are rounded off as a composite AM/PM
				string.
		*/
				{
				char *pchT;
				int wBefSep, w;

				if (FCompChCh(*pch, rgchAmOld[0]) || 
						FCompChCh(*pch, rgchPmOld[0]))
					{
					pchT = pch+1;
					while (*pchT != '\0' && *pchT != chFldPicAMPMSep)
						++pchT;
					if (*pchT == chFldPicAMPMSep)
						{
						wBefSep = pchT - pch;
						pchT++;
						for (w=0; w < wBefSep && *pchT != '\0';
								++pchT, ++w);
						if (w == wBefSep)
							{
							BOOL fLower = FLower(*pch);

							bltb(rgchAmNew, psz, cchAmNew);
							*(psz+cchAmNew) = chFldPicAMPMSep;
							bltb(rgchPmNew,psz+cchAmNew+1,cchPmNew);
							for (w=0; w < cchAmNew+cchPmNew+1; ++w)
								{
								if (fLower)
									*psz=ChLower(*psz);
								else
									*psz=ChUpper(*psz);
								++psz;
								}
							pch += pchT - pch;
							continue;
							}
						}
					}
				}

		/* convert date separators */
			if (*pch == chDateSepOld)
				{
				*psz = chDateSepNew;
				++psz; 
				++pch;
				continue;
				}

		/* convert time separators */
			if (*pch == chTimeSepOld)
				{
				*psz = chTimeSepNew;
				++psz; 
				++pch;
				continue;
				}

		/* convert day, month, year chars */
			if (FCompChCh(*pch, chDayOld))
				{
				*psz = FLower(*pch) ? ChLower(chDayNew) : 
						ChUpper(chDayNew);
				++psz; 
				++pch;
				continue;
				}

			if (*pch == ChUpper(chMonthOld)
					/* case sensitive comparison for US */
#ifndef MONTHMINUTESAME
			|| (fLocal && *pch == ChLower(chMonthOld))
#endif
					)
				{
				*psz = FLower(*pch) ? ChLower(chMonthNew) :
						ChUpper(chMonthNew);
				++psz; 
				++pch;
				continue;
				}

			if (FCompChCh(*pch, chYearOld))
				{
				*psz = FLower(*pch) ? ChLower(chYearNew) :
						ChUpper(chYearNew);
				++psz; 
				++pch;
				continue;
				}

		/* convert hour, minute chars */
			if (FCompChCh(*pch, chHourOld))
				{
				*psz = FLower(*pch) ? ChLower(chHourNew) :
						ChUpper(chHourNew);
				++psz; 
				++pch;
				continue;
				}

			if (*pch == ChLower(chMinuteOld)
					/* case sensitive comparison for US */
#ifndef MONTHMINUTESAME
			|| (fLocal && *pch == ChUpper(chMinuteOld))
#endif
					)
				{
				*psz = FLower(*pch) ? ChLower(chMinuteNew) :
						ChUpper(chMinuteNew);
				++psz; 
				++pch;
				continue;
				}

			*psz++ = *pch++;
			}
		else
			{
			Assert(fInQuote || fInSequence);
			if (*pch == chFldPicSequence)
				{
				fInSequence = fFalse;
				*psz++ = *pch++;
				continue;
				}
			if (*pch == chFldPicQuote)
				{
				fInQuote = fFalse;
				*psz++ = *pch++;
				continue;
				}
			*psz++ = *pch++;
			}
		}

	Assert(*pch == '\0');
	*psz = '\0';

	return (psz - sz + 1);
}


/* F  S P  F E T C H  A R G  T E X T */
/* fetches text of an argument for translation purposes. */

/* %%Function:FSpFetchArgText %%Owner:krishnam */
BOOL FSpFetchArgText (pffb)
struct FFB * pffb;
{
	struct FBB fbb;
	int cch;

	fbb = pffb->fbb;

	pffb->ccrMax = 0;
	pffb->rgcr = NULL;

	pffb->cchMax--;

	FetchFromField (pffb, fTrue /* fArgument */, fTrue /* fRTF */);

	cch = pffb->cch;
	pffb->rgch [cch] = '\0';

	Assert (cch == pffb->cchMax || !pffb->fOverflow);

	if (pffb->fOverflow)
		{
		pffb->cchMax = 0;
		pffb->rgch = NULL;

		FetchFromField (pffb, fTrue, fTrue);

		/*  nothing to overflow!! */
		Assert (!pffb->fOverflow);
		}

	pffb->fbb = fbb;  /* restore orginal buffers */

	pffb->cch = cch;
	pffb->ccr = 0;

	return !pffb->fNoArg;

}


/* C H  S P  F E T C H  S W */
/* used for fetching the switch character and its CP for translation
* purposes.
*/
/* %%Function:ChSpFetchSw %%Owner:krishnam */
CHAR ChSpFetchSw(pffb, pcp, fSys)
struct FFB *pffb;
CP *pcp;
BOOL fSys;
{
	int ich;
	int foc = pffb->foc;
	struct FSF *pfsf;

	Assert (pffb->cpFirst == pffb->cpLim || foc > focNormal);

	if (foc < focNormal)
		/*  nothing (should be) cached */
		return chNil;

	if (fSys)
		{
		ich = foc / (cSwitchMax+1);
		foc += cSwitchMax+1;
		pfsf = &pffb->fsfSys;
		}
	else
		{
		ich = foc % (cSwitchMax+1);
		foc++;
		pfsf = &pffb->fsfFlt;
		}

	pffb->foc = foc;

	if (ich >= pfsf->c)
		/*  nothing left */
		return chNil;

	if (pfsf->rgcp [ich] != cpNil)
		{
		/*  set up to fetch the argument */
		pffb->cpFirst = pfsf->rgcp [ich];
		pffb->fOverflow = fFalse;
		}
	else
		/*  no argument */
		pffb->cpFirst = pffb->cpLim;

	*pcp = pfsf->rgcpSw [ich];
	return pfsf->rgch [ich];
}


/* C H  I T L  S W */
/* fetches the translated form of the switch. */

/* %%Function:ChItlSw %%Owner:krishnam */
CHAR ChItlSw(flt, chSw, fLocal)
int flt;
char chSw;
BOOL fLocal;
{
	int iFsitl = 0;

	while (iFsitl < iRgFsitlMax)
		{
		if (flt == rgfsitl[iFsitl].flt && 
				FCompChCh(chSw, (fLocal ? rgfsitl[iFsitl].chSwLocal :
				rgfsitl[iFsitl].chSwUS))
				)
			return (fLocal ? 
					(FLower(chSw) ? ChLower(rgfsitl[iFsitl].chSwUS)
					: ChUpper(rgfsitl[iFsitl].chSwUS))
:
					(FLower(chSw) ? ChLower(rgfsitl[iFsitl].chSwLocal)
					: ChUpper(rgfsitl[iFsitl].chSwLocal))
					);
		++iFsitl;
		}

	return chNil;
}


/* S O R T  I C P */
/* sorts the array rgw according to the staring cp's in trfl->rgCpFirst */

/* %%Function:SortIcp %%Owner:krishnam */
SortIcp(ptrfl, rgw)
TRFL *ptrfl;
int *rgw;
{
	int iw, iwMac;

	iwMac = ptrfl->iMax;
	if (iwMac < 2) return;
	for (iw = iwMac>>1; iw >= 2; --iw)
		SortSiftUpIcp(ptrfl, rgw, iw, iwMac);
	for (iw = iwMac; iw >= 2; --iw)
		{
		int w;
		SortSiftUpIcp(ptrfl, rgw, 1, iw);
		w = rgw[0];
		rgw[0] = rgw[iw - 1];
		rgw[iw - 1] = w;
		}
}


/* %%Function:SortSiftUpIcp %%Owner:krishnam */
SortSiftUpIcp(ptrfl, rgw, iwI, iwN)
TRFL *ptrfl;
int *rgw, iwI, iwN;
{
	int iwJ;
	int wCopy;

	wCopy = rgw[iwI - 1];
Loop:
	iwJ = 2 * iwI;
	if (iwJ <= iwN)
		{
		if (iwJ < iwN)
			{
			if (ptrfl->rgCpFirst[rgw[iwJ]] > 
					ptrfl->rgCpFirst[rgw[iwJ - 1]])
				iwJ++;
			}
		if (ptrfl->rgCpFirst[rgw[iwJ - 1]] > ptrfl->rgCpFirst[wCopy])
			{
			rgw[iwI - 1] = rgw[iwJ - 1];
			iwI = iwJ;
			goto Loop;
			}
		}
	rgw[iwI - 1] = wCopy;
}


/* F  C O M P  C H  C H */
/* do a case insensitive comparison of chars. */

/* %%Function:FCompChCh %%Owner:krishnam */
FCompChCh(ch1, ch2)
char ch1, ch2;
{
	if (FAlpha(ch1) && FAlpha(ch2))
		return (ch1 == ChUpper(ch2) || ch1 == ChLower(ch2));
	else
		return (ch1 == ch2);
}




/* F  T R A N S L  F N */
/* Translates the function name in an = field. Responsibility of caller to
	ensure that szOpt has size of at least cchArgumentFetch */

/* %%Function:FTranslFn %%Owner:krishnam */
BOOL FTranslFn(szOpt, ppchLead, pchFollow, pfnCchSz, pfnId)
char szOpt[], **ppchLead, *pchFollow;
int (*pfnCchSz)(), (*pfnId)();
{
	int cch, id;
	char szT[cchArgumentFetch];
	char *pchLead = *ppchLead;

	Assert(pchFollow <= pchLead);
	Assert(CchSz(pchLead) >= 0);
	if (pchFollow == pchLead)
		return fTrue;
	cch = min(pchLead - pchFollow, cchArgumentFetch - 1);
	bltb(pchFollow, szT, cch);
	cch = CchStripString(szT, cch);
	szT[cch]='\0';
	id = (*pfnId)(szgFltExpFunc, szT);
	cch = (*pfnCchSz)(szT, szgFltExpFunc, id, cchArgumentFetch);
	if (cch > 1)
		{
		--cch;
		if (pchFollow+cch+CchSz(pchLead)-szOpt >= cchArgumentFetch)
			return fFalse;
		bltb(pchLead, pchFollow+cch, CchSz(pchLead));
		bltb(szT, pchFollow, cch);
		*ppchLead = pchFollow + cch;
		}
	return fTrue;
}




/* F  I N F I X  C H */
/* Determine whether ch is an infix operator of the expression field. */

/* %%Function:FInfixCh %%Owner:krishnam */
BOOL FInfixCh(ch)
char ch;
{
	return (ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '%' ||
			ch == '^' || ch == '=' || ch == '<' || ch == '>');
}


