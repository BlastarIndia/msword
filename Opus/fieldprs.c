/* F I E L D P R S . C */
/*  Field parsing functions */

#ifdef DEBUG
#ifdef PCJ
/* #define DBGFIELDS */
#endif /* PCJ */
#endif /* DEBUG */

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "props.h"
#include "doc.h"
#include "disp.h"
#include "format.h"
#include "field.h"
#include "sel.h"
#include "prm.h"
#include "ch.h"
#include "strtbl.h"
#include "inter.h"
#include "debug.h"
#include "dde.h"
#include "border.h"
#include "idle.h"



#ifdef PROTOTYPE
#include "fieldprs.cpt"
#endif /* PROTOTYPE */

/* E X T E R N A L S */

extern struct TAP   vtapFetch;
extern struct TCC   vtcc;
extern struct CA    caTap;
extern struct CA    caPara;
extern struct SEL   selCur;
extern CP           vcpFetch;
extern CHAR HUGE *  vhpchFetch;
extern int          vdocFetch;
extern int          vccpFetch;
extern struct CHP   vchpFetch;
extern BOOL         vfEndFetch;
extern BOOL         vfInsertMode;
extern int          vifldInsert;
extern struct FLI   vfli;
extern CP           cpInsert;
extern struct PREF  vpref;
extern struct MERR  vmerr;
extern struct DDES  vddes;
extern BOOL         vfDdeIdle;
extern struct ITR   vitr;
extern struct PAP   vpapFetch;
extern IDF	    vidf;






/* F I E L D  P A R S I N G  F U N C T I O N S */





/* F L T  P A R S E  D O C  C P */
/*  Parse the field which starts at doc, cp.  
	If fModify then the field is killed or resurrected as appropriate.
*/

/* %%Function:FltParseDocCp %%Owner:peterj */
EXPORT FltParseDocCp (doc, cp, ifld, fChgView, fEnglish)
int doc;
CP cp;
int ifld;
BOOL fChgView, fEnglish;
{
	int wwFli = vfli.ww;
	int docFli = vfli.doc;
	int ich;
	int flt;
	struct FFB ffb;
	struct EFLT eflt;
	CHAR rgch [cchMaxFieldKeyword + 1];
	struct FLD fld;

	Assert (doc != docNil);

	Scribble (ispFieldParse, 'P');

#ifdef DBGFIELD
	CommSzLong (SzShared("FltParseDocCp: parsing "), cp);
#endif /* DBGFIELD */

	/* set up field fetch block */
	InitFvb (&ffb.fvb);
	InitFvbBufs (&ffb.fvb, rgch, cchMaxFieldKeyword, NULL, 0);

	/* fetch instructions for field at doc, cp */
	if (ifld == ifldNil)
		SetFfbDeadFlt (&ffb, doc, cp, fltUnknownKwd);
	else
		SetFfbIfld (&ffb, doc, ifld);

	/*  fetch the first argument */
	ffb.foc = focNone; /* don't treat '\' special, don't allow nested fields */
	FetchFromField (&ffb, fTrue /* fArgument */, fFalse /* fRTF */);

	/*  search for the keyword */
	rgch [ffb.cch] = '\0';
	flt = FltParseSz (rgch, fEnglish);

	eflt = EfltFromFlt (flt);

	/* bring to life or kill ? */
	if (ifld == ifldNil)
		{
		/*  previously dead but should be alive: resurrect */
		if (eflt.fLive)
			ResurrectField (doc, cp, flt, fChgView);
		ifld = IfldFromDocCp(doc, cp, fTrue);
		}

	else  if /* ifld != ifldNil */
	(eflt.fDead)
		/*  previously live but should be dead: kill */
		KillField (doc, cp, ifld);

	else  /* assure field correct & visible */			
		{
		struct PLC **hplcfld = PdodDoc(doc)->hplcfld;

		GetPlc( hplcfld, ifld, &fld );
		fld.flt = flt;
		fld.fDirty = fFalse;
		PutPlcLast( hplcfld, ifld, &fld );
		if (fChgView)
			AssureInstVisible (doc, ifld);
		}

	/*  Killing & resurrecting may invalidate fli, revalidate */
	vfli.ww = wwFli;
	vfli.doc = docFli;

	if ((uns)(flt - fltSeqLevOut) <= (fltSeqLevNum - fltSeqLevOut))
		/* autonum field - have to see that all others get updated */
		vidf.fInvalSeqLev = PdodDoc(doc)->fInvalSeqLev = fTrue;

	Scribble (ispFieldParse, ' ');
	return flt;

}




/* F L T  P A R S E  S Z */
/*  Parse sz and return the field whose keyword it is.
*/

/* %%Function:FltParseSz %%Owner:peterj */
FltParseSz (sz, fEnglish)
CHAR *sz;
BOOL fEnglish;
{
	BOOL fEscaped = *sz == chFieldEscape;
	int flt, cch;
	CHAR szSingle [2];

	/*  ignore '\' if any */
	if (fEscaped)
		sz++;

	/*  empty string? bag out early */
	if (!*sz)
		return fltUnknownKwd;

	/*  see if it is a special "single character" keyword */
	szSingle [0] = *sz;
	szSingle [1] = 0;

	if ((flt = (fEnglish ? IdEngFromSzgSz(szgFltSingle, szSingle) :
			IdFromSzgSz (szgFltSingle, szSingle))) != idNil)
		return flt;

	/* try for a normal keyword */
	if ((flt = (fEnglish ? IdEngFromSzgSz(szgFltNormal, sz) :
			IdFromSzgSz (szgFltNormal, sz))) != idNil)
		return flt;

	/*  could it possibly be a bookmark name? */
	if (!fEscaped && (cch = CchSz (sz)) <= cchBkmkMax)
		{
		CHAR st [cchBkmkMax];
		SzToSt (sz, st);
		if (FLegalBkmkName (st))
			return fltPosBkmk;
		}

	/*  don't know anything about this field */
	return fltUnknownKwd;
}






/* A S S U R E  I N S T  V I S I B L E */
/*  Make field ifld be in show instruction mode.
*/

/* %%Function:AssureInstVisible %%Owner:peterj */
AssureInstVisible (doc, ifld)
int doc;
int ifld;
{
	struct FLCD flcd;
	GetIfldFlcd (doc, ifld, &flcd);
	if (FShowResultPflcdFvc (&flcd, selCur.ww /*fvcScreen*/))
		{
		flcd.fDiffer ^= 1;
		SetFlcdCh (doc, &flcd, chFieldEnd);
		}
}





/* F I E L D  I N S T  F E T C H I N G */



/* S E T  F F B  I F L D */
/*  Set up pffb to fetch the instructions of field ifld.
*/

/* %%Function:SetFfbIfld %%Owner:peterj */
SetFfbIfld (pffb, doc, ifld)
struct FFB *pffb;
int doc, ifld;

{
	struct FLCD flcd;

	GetIfldFlcd (doc, ifld, &flcd);

	pffb->doc = doc;
	pffb->cpFirst = flcd.cpFirst + 1; /* skip chFieldBegin */
	pffb->cpLim = flcd.cpFirst + flcd.dcpInst - 1; /* skip chFieldEnd */
	pffb->fOverflow = fFalse;

	SetBytes (&pffb->fsfb, 0, sizeof (struct FSFB));
	pffb->flt = flcd.flt;
	pffb->cpField = flcd.cpFirst;

	CachePara (doc, flcd.cpFirst);
	FetchCp (doc, flcd.cpFirst, fcmProps);
	Assert (vccpFetch > 0 && vchpFetch.fSpec);
	pffb->fFetchVanished = vchpFetch.fVanish || vchpFetch.fStrike
			|| vchpFetch.fFldVanish;
	pffb->fFetchTable = FInTableDocCp(doc, flcd.cpFirst);
}





/* F I E L D  S W I T C H E S */
/*  Each switch available globally (system) or for a specific field type
	must be recorded here. */
csconst struct FSI rgfsi [] =
	{  /* ch,                   fArg,       flt             */
		{ chFldSwSysNumeric,    fTrue,      fltSys          },
		{ chFldSwSysDateTime,   fTrue,      fltSys          },
		{ chFldSwSysFormat,     fTrue,      fltSys          },
		{ chFldSwSysLock,       fFalse,     fltSys          },
		{ chFldSwStyleRefLast,  fFalse,     fltStyleRef     },
		{ chFldSwAskOnce,       fFalse,     fltAsk          },
		{ chFldSwAskOnce,       fFalse,     fltFillIn       },
		{ chFldSwDefault,       fTrue,      fltAsk          },
		{ chFldSwDefault,       fTrue,      fltFillIn       },
		{ chFldSwConverter,     fTrue,      fltInclude      },
		{ chFldXeText,          fTrue,      fltXe           },
		{ chFldXeRange,         fTrue,      fltXe           },
		{ chFldXeBold,          fFalse,     fltXe           },
		{ chFldXeItalic,        fFalse,     fltXe           },
		{ chFldIndexRunin,      fFalse,     fltIndex        },
		{ chFldIndexBookmark,   fTrue,      fltIndex        },
		{ chFldIndexSeqname,    fTrue,      fltIndex        },
		{ chFldIndexSeqSep,     fTrue,      fltIndex        },
		{ chFldIndexListSep,    fTrue,      fltIndex        },
		{ chFldIndexRangeSep,   fTrue,      fltIndex        },
		{ chFldIndexHeading,    fTrue,      fltIndex        },
		{ chFldIndexPartial,    fTrue,      fltIndex        },
		{ chFldIndexEntrySep,   fTrue,      fltIndex        },
#ifdef ENABLE
		{ chFldIndexConcord,    fTrue,      fltIndex        },
#endif /* ENABLE */
		{ chFldSeqReset,        fTrue,      fltSequence     },
		{ chFldSeqNext,         fFalse,     fltSequence     },
		{ chFldSeqCurrent,      fFalse,     fltSequence     },
		{ chFldSeqHidden,       fFalse,     fltSequence     },
		{ chFldTocOutline,      fTrue,      fltToc          },
		{ chFldTocFields,       fTrue,      fltToc          },
		{ chFldTocBookmark,     fTrue,      fltToc          },
		{ chFldTocSeqname,      fTrue,      fltToc          },
		{ chFldTocSeqSep,       fTrue,      fltToc          },
		{ chFldTceTableId,      fTrue,      fltTce          },
		{ chFldTceLevel,        fTrue,      fltTce          },
		{ chFldPrintPs,         fTrue,      fltPrint        },
		{ chFldImportBogus,     fTrue,      fltImport       },

		/* this must be last entry */
		{ 0,                    fFalse,     fltSys          }
	};	



/* I F S I  P F S F  L O O K U P  S W I T C H */
/*  Search the switch table for a switch for type flt, character ch.
	Return the ifsi and a pointer to the fsf in pffb.
*/

/* %%Function:IfsiPfsfLookupSwitch %%Owner:peterj */
IfsiPfsfLookupSwitch (ch, pffb, ppfsf)
CHAR ch;
struct FFB *pffb;
struct FSF **ppfsf;

{
	int ifsi;
	int flt = pffb->flt;
	int fltSw;
	CHAR chSw;

	if (pffb->flt == fltNil)
		return iNil;

	ch = ChLower (ch);

	for (ifsi = 0; (chSw = rgfsi [ifsi].ch) != 0; ifsi++)
		if (chSw == ch)
			if ((fltSw = (int)rgfsi [ifsi].flt) == fltSys || fltSw == flt)
				{
				*ppfsf = fltSw == fltSys ? &pffb->fsfSys : &pffb->fsfFlt;
				return ifsi;
				}

	return iNil;
}




/* F E T C H  F R O M  F I E L D */
/*  Fetch cps and crs starting at cpFirst.  If fArgument fetch one
	argument, otherwise fetch to cpLim or an unmatched chFieldEnd. Stop
	fetching if cr or ch buffer are filled, set fOverflow and be prepared
	to resume with sufficient state.  If !fArgument then first level nested
	fields are (optionally) surrounded by chGroupInternal.
*/

/* %%Function:FetchFromField %%Owner:peterj */
NATIVE FetchFromField (pffb, fArgument, fRTF)
struct FFB * pffb;
BOOL fArgument, fRTF;

{

/* WARNING: this is a *VERY* fragile routine!  It fully implements the
	"argument" rules of fields, be sure you understand all of those rules
	before making changes.

	Notes/Assumptions:
		fLeading is undefined if !fArgument
		fPMergeRules means we are fetching from the print merge data file
		chTermPreceeding is used only if fPMergeRules	

*/

	CHAR HUGE *hpch = NULL, HUGE *hpchLim = NULL;
	CHAR ch;
	CP cp = pffb->cpFirst - 1;
	CP cpPreceeding;
	int doc = pffb->doc;

	BOOL fLeading = pffb->fOverflow ? fFalse : fTrue;
	BOOL fTrailing = fFalse;
	BOOL fChFieldPreceeding = fFalse;
	BOOL fChEscapePreceeding = fFalse;
	BOOL fSkipEscape = fTrue;
	CHAR chTermPreceeding = 0;

	/*  Take care of initialization */
	pffb->cch = pffb->ccr = 0;

	if (!pffb->fOverflow)
		{
		pffb->cchFieldBegin = 0;
		pffb->fGroupPending = fFalse;
		pffb->fGrouped = fFalse;
		pffb->fNested = fFalse;
		pffb->fTableRules = !pffb->fPMergeRules ? fFalse :
				(FInTableDocCp(doc, cp+1) && cp+1 < pffb->cpLim);
		}
	else
		pffb->fOverflow = fFalse;

	Assert (pffb->cpLim <= CpMacDoc (doc));
	Assert (pffb->cpLim <= CpMacDocEdit (doc) || pffb->fPMergeRules);
	Assert ((pffb->rgch != NULL) ^ (pffb->cchMax == 0));
	Assert ((pffb->rgcr != NULL) ^ (pffb->ccrMax == 0));

	if (pffb->fTableRules)
		/* Reading arguments from a table (for print merge).  Rules change
			somewhat.  We ignore all the normal Argument separators (tabs,
			paragraph marks, commas) and all grouping (quotes).  Instead we
			pay full attention to the table structure.
		*/
		{
		CpFirstTap(doc, cp+1);
		CacheTc(wwNil, doc, cp+1, fFalse, fFalse);
		pffb->cpLim = fArgument ? vtcc.cpLim-ccpEop : caTap.cpLim;
		fLeading = fFalse;
		}

	while (++cp < pffb->cpLim)
		{

		if (hpch >= hpchLim)
			{
			if (FInTableDocCp(doc, cp) && !pffb->fFetchTable && !pffb->fTableRules)
				/* a table nested within field codes is IGNORED */
				{
				CachePara (doc, cp);
				cp = caPara.cpLim - 1;
				continue;
				}
			FetchCpAndPara(doc, cp, fcmChars+fcmProps);
			Assert (vccpFetch > 0);

			/*  ignore vanished text unless fFetchVanished */
			if ((vchpFetch.fVanish||vchpFetch.fStrike||vchpFetch.fFldVanish)
					&& (!pffb->fFetchVanished || vchpFetch.fSysVanish))
				{
				cp += vccpFetch - 1;
				/* hpch still >= hpchLim */
				continue;
				}

			hpch = vhpchFetch;
			hpchLim = hpch + vccpFetch;
			Assert (vcpFetch == cp);
			}

		ch = *hpch++;

		Assert ((fLeading || fTrailing) + (pffb->fGroupPending ||
				pffb->cchFieldBegin != 0) < 2);
		Assert (fArgument || !fTrailing);


		/*  preceeding character was a field begin character which is
			being resolved.  Current character is:
			chFieldSeparate - there is a result in the field so we
			are nesting. Continue and process the subsequent
			characters (of the result).
			chFieldEnd - the field had no result. Fall through to the
			chFieldEnd character processing below to terminate
			this nest.
		*/
		if (fChFieldPreceeding)
			{{ /* !NATIVE - FetchFromField */
			fChFieldPreceeding = fFalse;

			Assert ((ch == chFieldSeparate || ch == chFieldEnd) &&
					vchpFetch.fSpec);

			/*  if we are nesting another level, increment count */
			if (!pffb->fTableRules && pffb->cchFieldBegin++ == 0 && 
					!pffb->fGroupPending)
				{
				/* field starts new argument with fArgument
					if !fArgument, add chGroupInternal
				*/
				if ((fArgument && !fLeading)
						|| (!fArgument && pffb->fGroupInternal &&
						!FAddChCpToPfvb (&pffb->fvb, chGroupInternal, 
						cpNil)))
					{
					/*  restore the state and finish up */
					cp = cpPreceeding;
					pffb->cchFieldBegin--;
					{{goto LDone;}}  /* !NATIVE - FetchFromField */
					}
				/* indicate type of argument */
				pffb->fNested = fTrue;
				pffb->fGrouped = fTrue;
				fLeading = fFalse;
				}

			if (ch != chFieldEnd)
				{{continue;}} /* !NATIVE - FetchFromField */
			}}


		/*  We are not nested in a field and hit an escape character.
			If the next character is a known switch, cache it (skip argument
			if necessary).  If it is unknown, pass it on.
		*/
		if (fChEscapePreceeding)
			{{ /* !NATIVE - FetchFromField */
			int ifsi, ich;
			struct FSF *pfsf;

			fChEscapePreceeding = fFalse;

			/* special case for index entries */
			if (ch == chColon && pffb->flt == fltXe)
				{
				/* put the \ back */
				if (!FAddChCpToPfvb (&pffb->fvb, chFieldEscape, cpPreceeding))
					{
					cp = cpPreceeding;
						{{goto LDone;}} /* !NATIVE - FetchFromField */
					}
				}

			/*  see if it is a switch we know about (inside quotes, we don't
				know any except ':' for xe). */
			if (!pffb->fGroupPending && 
					(ifsi = IfsiPfsfLookupSwitch (ch, pffb, &pfsf)) != iNil
					&& pfsf->c < cSwitchMax)
				{  /* this is a real switch (not pass through character) */

				/*  if we are not caching (i.e. fetching switches), stop */
				if (pffb->foc != focNormal)
					{
					cp = cpPreceeding;
					/* switch found, stop! */
					{{goto LDone;}} /* !NATIVE - FetchFromField */
					}

				/*  else: cache the switch */
				ich = pfsf->c++;
				Assert (pfsf->rgch [ich] == 0 && pfsf->rgcp [ich] == 0);
				pfsf->rgch [ich] = ch;
				pfsf->rgcpSw [ich] = cp;
				pfsf->rgcp [ich] = cpNil; /* no arg until we fetch one */
				if (rgfsi [ifsi].fArg)
					{    /* recursively skip over the argument */
					struct FFB ffbT;
					CP cpT1, cpT2;
					ffbT = *pffb;
					ffbT.foc = focStopOnSw;
					ffbT.cpFirst = cp + 1; /* start with next char */
					if (FFetchArgExtents (&ffbT, &cpT1, &cpT2, fRTF))
						{
						pfsf->rgcp [ich] = cp+1;
						cp = ffbT.cpFirst - 1; /* continue where left off */
						}
					hpch = hpchLim;   /* invalidate FetchCp */
					}
				/* sw is cached, continue processing */
				{{continue;}} /* !NATIVE - FetchFromField */
				}

			/* switch not understood, add ch as text (with or w/o the leading 
				'\' as the case may be). Note that \", \\, etc fall through 
				to here. */
			else
				{
				if (!fSkipEscape && !FAddChCpToPfvb(&pffb->fvb, chFieldEscape, 
						cpPreceeding))
					{
					cp = cpPreceeding;
					{{goto LDone;}} /* !NATIVE - FetchFromField */
					}
				if (fTrailing || 
					(fRTF && pffb->flt == fltFormula && !FAddChCpToPfvb(&pffb->fvb, chFieldEscape, cpPreceeding))
					|| !FAddChCpToPfvb (&pffb->fvb, ch, cp))
					/* Don't or can't put in result (fTrailing implies 
						fArgument) */
					{
					cp = cpPreceeding;
					{{goto LDone;}} /* !NATIVE - FetchFromField */
					}
				else
					{
					fLeading = fFalse;
					{{continue;}} /* !NATIVE - FetchFromField */
					}
				}
			}}


		/*  escape character while not nested, get next ch and process
			(ignore escape while in quotes if !fArgument). */
		if (ch == chFieldEscape)
			{{ /* !NATIVE - FetchFromField */
			if (pffb->cchFieldBegin == 0 && pffb->foc != focNone
					&& (fArgument || !pffb->fGroupPending))
				{
				cpPreceeding = cp;
				fChEscapePreceeding = fTrue;
				{{continue;}} /* !NATIVE - FetchFromField */
				}
			}}


		/* Check for Print Merge End-of-Record */
		/* TableRules end of record handles already by setting cpLim. */
		if (pffb->fPMergeRules && 
#ifdef CRLF
				(ch == chReturn || ch == chEop))
#else
				ch == chEop)
#endif /* CRLF */
				/* end of a record during Print Merge is an EOP */
				{{ /* !NATIVE - FetchFromField */
				if (!pffb->fTableRules && !pffb->fGroupPending
						&& pffb->cchFieldBegin == 0)
					{
					if (!chTermPreceeding)
						/* skip over the CR so that next fetch is in new record.
							if chTermPreceeding then wait until next argument fetch
							to reset so that proper notification of an empty item
							is made.
						*/
						{
#ifdef CRLF
						cp = pffb->cpLim = cp + (ch == chReturn ? 2 : 1);
#else
						cp = pffb->cpLim = cp + 1;
#endif /* CRLF */
						}
					fLeading = fFalse;
					{{goto LDone;}} /* !NATIVE - FetchFromField */
					}
				}}


		/*  Fields Processing */

		if (vchpFetch.fSpec)
			{

			/* If chFieldBegin is hit, skip over the hidden part and cause
				next character to be fetched to be either the chFieldEnd (for
				dead field or field w/o result) or the chFieldSeparate (for
				field with result).
			*/
			if (ch == chFieldBegin)
				{{ /* !NATIVE - FetchFromField */
				int ifld = IfldFromDocCp (doc, cp, fTrue);

				if (pffb->foc == focNone)
					/* nested fields not allowed */
					{{goto LDone;}} /* !NATIVE - FetchFromField */

				if (ifld == ifldNil)
					cp = CpLimDeadField (doc, cp) - 1;

				else
					{
					struct FLCD flcd;
					GetIfldFlcd (doc, ifld, &flcd);
					cpPreceeding = cp;  /* to be able to restore state */
					cp += DcpSkipFieldChPflcd (ch, &flcd, 
							fTrue /* fShowResults */, fFalse/*fFetch*/) - 2;
					fChFieldPreceeding = fTrue;
					}

				hpch = hpchLim;
				{{continue;}} /* !NATIVE - FetchFromField */
				}}

			/* chFieldEnd indicates we are done nesting in a field.  If it
				was a top-level nested field and fArgument, it is the end
				of the argument, just eat the white space left.  if !fArgument
				add internal quotes.
			*/
			else  if (ch == chFieldEnd)
				{{ /* !NATIVE - FetchFromField */

				if (!pffb->fTableRules && --pffb->cchFieldBegin == 0 && 
						!pffb->fGroupPending)
					{
					if (fArgument)
						fTrailing = fTrue;
					else
						{
						if (pffb->fGroupInternal &&
								!FAddChCpToPfvb (&pffb->fvb, chGroupInternal, 
								cpNil))
							{
							pffb->cchFieldBegin++;
							{{goto LDone;}} /* !NATIVE - FetchFromField */
							}
						}
					{{continue;}} /* !NATIVE - FetchFromField */
					}

				else  if (pffb->cchFieldBegin < 0)
					{
					Assert (fFalse); /* overflowed the field! */
					{{goto LDone;}} /* !NATIVE - FetchFromField */
					}

				else  /* cchFieldBegin > 0 || pffb->fGroupPending */			
					{{continue;}} /* !NATIVE - FetchFromField */

				}} /* ch == chFieldEnd */


			Assert (ch != chFieldSeparate);

			} /* vchpFetch.fSpec */


		if (pffb->cchFieldBegin == 0 && !pffb->fTableRules)
			{
			/*  Group Processing */

			/* if we hit a quote character indicate that the argument is
				grouped.  If we are currently processing a group, end that
				processing.
			*/
			if (ch == chGroupExternal)
				{{ /* !NATIVE - FetchFromField */

				/* add the quote to result if !fArgument */
				if (!fArgument && !FAddChCpToPfvb (&pffb->fvb, ch, cp))
					{{goto LDone;}} /* !NATIVE - FetchFromField */

				if (pffb->fGroupPending)
					/* finish off a group */
					{
					if (fArgument)
						fTrailing = fTrue;
					pffb->fGroupPending = fFalse;
					if (fRTF)
						{
						Assert(fArgument);
						fSkipEscape = fTrue;
						}
					{{continue;}} /* !NATIVE - FetchFromField */
					}

				else  if (fLeading || !fArgument)
					/* begin a group */
					{
					pffb->fGroupPending = fTrue;
					pffb->fGrouped = fTrue;
					fLeading = fFalse;
					if (fRTF)
						{
						Assert(fArgument);
						fSkipEscape = fFalse;
						}
					{{continue;}} /* !NATIVE - FetchFromField */
					}

				else
					/* quote follows fetched argument */
					{
					Assert (fArgument);
					{{goto LDone;}} /* !NATIVE - FetchFromField */
					}
				}}


			/*  Argument terminator processing */

			/* Check for white space (normal rules) or for a print merge
				argument termination */

			if (fArgument && !pffb->fGroupPending)
				{
				if (pffb->fPMergeRules)
					{
					if (FPMergeTerm (ch))
						{{ /* !NATIVE - FetchFromField */
						if (ch == chTermPreceeding)
							{{goto LDone;}} /* !NATIVE - FetchFromField */
						else  if (!chTermPreceeding)
							chTermPreceeding = ch;
						fLeading = fFalse;
						fTrailing = fTrue;
						{{continue;}} /* !NATIVE - FetchFromField */
						}}
					else  if (FWhite (ch))
						{
						if (fLeading || fTrailing)
							continue;
						/* else insert it */
						}
					else  /* ! white space */						
						{
						if (fLeading)
							fLeading = fFalse;
						if (fTrailing)
							goto LDone; /* done */
						/* else insert non-white space */
						}
					}

				else  /* !fPMergeRules */					
					{
					if (FWhite (ch))
						{
						if (!fLeading && !fTrailing)
							fTrailing = fTrue;
						continue;
						}
					else  /* ! white space */						
						{
						if (fTrailing)
							goto LDone;
						if (fLeading)
							fLeading = fFalse;
						/* else insert non-white space */
						}
					}
				}

			}  /* if (pffb->cchFieldBegin == 0) */


		if (! FAddChCpToPfvb (&pffb->fvb, ch, cp))
			goto LDone;  /* done */

		}  /* while */

LDone:
	Assert (cp <= pffb->cpLim);
	Assert (cp == pffb->cpLim || !fLeading || pffb->foc != focNormal
			|| !fArgument);
	Assert (cp < pffb->cpLim || pffb->cchFieldBegin == 0
			|| pffb->flt == fltIf || pffb->flt == fltPMNextIf
			|| pffb->flt == fltPMSkipIf /*special cases*/);
	Assert (!fChFieldPreceeding);
	/* if fChEscapePreceeding, don't really care. */

	if (pffb->fTableRules && !pffb->fOverflow)
		{
		CpFirstTap(doc, pffb->cpFirst);
		CacheTc(wwNil, doc, pffb->cpFirst, fFalse, fFalse);

		if (vtcc.itc >= vtapFetch.itcMac-1 || !fArgument)
			/* indicate end of fetch for this record */
			{
			pffb->cpFirst = caTap.cpLim;
			pffb->fNoArg = vtcc.itc == vtapFetch.itcMac;
			}
		else
			/* set up to fetch next item */
			{
			pffb->cpFirst = vtcc.cpLim;
			pffb->fNoArg = fFalse;
			}
		pffb->cpLim = caTap.cpLim;
		}
	else
		{
		pffb->cpFirst = cp;
		pffb->fNoArg = fArgument ? fLeading : fFalse;
		}
}





/* F  A D D  C H  C P  T O  P F V B */
/*  Add ch to rgch and update rgcr to include cp.  Does nothing and
	returns fFalse if either rgch or rgcr run out of space.
*/

/* %%Function:FAddChCpToPfvb %%Owner:peterj */
NATIVE FAddChCpToPfvb (pfvb, ch, cp)
struct FVB * pfvb;
CHAR ch;
CP cp;

{
	if (pfvb->rgch != NULL && pfvb->cch >= pfvb->cchMax)
		{
		pfvb->fOverflow = fTrue;
		return fFalse;
		}

	if (pfvb->rgcr != NULL)
		{
		int icr = pfvb->ccr - 1;
		struct CR *pcr = &pfvb->rgcr[icr];
		CP cpT;

		if (icr < 0)
			goto LNewCr;

		cpT = pcr->cp;
		if (cpT + pcr->ccp != cp)
			goto LNewCr;

		if (pcr->ccp >= ccpMaxRun)
			{
LNewCr:
			if (icr + 1 >= pfvb->ccrMax)
				{
				pfvb->fOverflow = fTrue;
				return fFalse;
				}

			else
				{
				pfvb->ccr++;
				(++pcr)->cp = cp;
				pcr->ccp = 1;
				}
			}

		else
			pcr->ccp++;
		}

	if (pfvb->rgch != NULL)
		pfvb->rgch [pfvb->cch++] = ch;

	return fTrue;

}






/* F  W H I T E */

/* %%Function:FWhite %%Owner:peterj */
NATIVE FWhite (ch)
CHAR ch;
{
	switch (ch)
		{
	case chSpace:
	case chEop:
	case chTable:
	case chCRJ:
	case chSect:
	case chTab:
	case chColumnBreak:
	case chGroupInternal:
#ifdef CRLF
	case chReturn:
#endif
		return fTrue;

	default:
		return fFalse;
		}
}


/* F  P  M E R G E  T E R M */
/* Return true if this is a field terminating character for Print Merge */

/* %%Function:FPMergeTerm %%Owner:peterj */
NATIVE FPMergeTerm (ch)
CHAR ch;

{
	switch (ch)
		{
	case chTab:
	case chSect:
		return fTrue;
	default:
		return (ch == vitr.chList);
		}
}






/* M I S C E L L A N E O U S */


/* F A S S U R E  H P L C F L D */
/* Assure that there is room in the plcfld for doc for cfld more entries.
*/
/* %%Function:FAssureHplcfld %%Owner:peterj */
FAssureHplcfld(doc, cfld)
int doc, cfld;
{
	extern BOOL vfInCommit;
	struct PLC **hplcfld = PdodDoc(doc)->hplcfld;

#ifdef DEBUG
	if (vfInCommit)
		AssertH(hplcfld);
#endif /* DEBUG */

	if (hplcfld == hNil)
		{
		hplcfld = HplcInit(cbFLD, 1, CpMac2Doc (doc), fTrue /* ext rgFoo */);
		if (hplcfld == hNil)
			return fFalse;
		PdodDoc (doc)->hplcfld = hplcfld;
		}

	return cfld > 0 ? FStretchPlc(hplcfld, cfld) : fTrue;
}


/* F  I N S E R T  F L T  D C P S */
/*  Insert an entry into the plcfld for a field of type flt at doc, cp with
	dcpInst and dcpResult as passed.  This routine only sets up the plc, it
	does not insert any characters into the document.  It is the callers
	responsibility to assure that the correct characters are in the correct
	locations (with fSpec set) BEFORE calling this function.
*/

/* %%Function:FInsertFltDcps %%Owner:peterj */
BOOL FInsertFltDcps (flt, doc, cp, dcpInst, dcpResult, pflcd)
int flt, doc;
CP cp, dcpInst, dcpResult;
struct FLCD *pflcd;

{
	extern struct DOD **mpdochdod[];
	int ifld;
	struct PLC **hplcfld;
	struct FLD fld;
	CP cpChSeparate = cp + dcpInst - 1;
	CP cpChEnd = cp + dcpInst + dcpResult - 1;

	Assert (doc != docNil);
	Assert (dcpInst >= 2);

	if (!FAssureHplcfld(doc, dcpResult == 0 ? 2 : 3))
		return fFalse;

	hplcfld = PdodDoc(doc)->hplcfld;

	/*  chFieldBegin */
	fld.ch = chFieldBegin;
	fld.fDirty = flt == fltUnknownKwd;
	fld.flt = flt;  /* flt in pflcd ignored! */
	ifld = IInPlcRef (hplcfld, cp);
	Assert (ifld >= 0 && ifld <= (*hplcfld)->iMac);
	AssertDo(FInsertInPlc (hplcfld, ifld, cp, &fld));  /* Assured above */

	/*  chFieldSeparate */
	if (dcpResult != 0)
		{
		fld.ch = chFieldSeparate;
		fld.bData = pflcd == NULL ? bDataInval : pflcd->bData;
		ifld = IInPlcRef (hplcfld, cpChSeparate);
		Assert (ifld >= 0 && ifld <= (*hplcfld)->iMac);
		AssertDo(FInsertInPlc (hplcfld, ifld, cpChSeparate, &fld));/* Assured above */
		}

	/* chFieldEnd */
	fld.ch = chFieldEnd;
	if (pflcd == NULL)
		{
		fld.fDiffer = fFalse;
		fld.fResultDirty = fFalse;
		fld.fResultEdited = fFalse;
		fld.fLocked = fFalse;
		fld.fPrivateResult = fFalse;
		fld.fNested = fFalse;
		}
	else
		fld.grpf = pflcd->grpf;
	ifld = IInPlcRef (hplcfld, cpChEnd);
	Assert (ifld >= 0 && ifld <= (*hplcfld)->iMac);
	AssertDo(FInsertInPlc (hplcfld, ifld, cpChEnd, &fld));  /* Assured above */

	PdodMother(doc)->fFormatted = fTrue;
	PdodDoc(doc)->fFldNestedValid = fFalse;
	return fTrue;
}


/* D E L E T E  F L D */
/*  Removes field ifld from the plc */

/* %%Function:DeleteFld %%Owner:peterj */
DeleteFld (doc, ifld)
int doc, ifld;

{
	struct PLC **hplcfld = PdodDoc (doc)->hplcfld;
	struct FLCD flcd;

	Assert (doc != docNil && hplcfld != hNil &&
			ifld > ifldNil && ifld < (*hplcfld)->iMac);
	FillIfldFlcd (hplcfld, ifld, &flcd);
	if ((uns)(flcd.flt - fltSeqLevOut) <= (fltSeqLevNum - fltSeqLevOut))
		vidf.fInvalSeqLev = PdodDoc(doc)->fInvalSeqLev = fTrue;
	FOpenPlc (hplcfld, flcd.ifldChEnd, -1);
	if (flcd.ifldChSeparate != ifldNil)
		FOpenPlc (hplcfld, flcd.ifldChSeparate, -1);
	FOpenPlc (hplcfld, flcd.ifldChBegin, -1);
	if (vddes.cDclClient != 0)
		/*  we may have deleted dde field, cause dde to check later */
		vfDdeIdle = fTrue;
	PdodDoc(doc)->fFldNestedValid = fFalse;
}


