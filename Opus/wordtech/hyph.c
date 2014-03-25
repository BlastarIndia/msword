#ifdef MAC
#define EVENTS
#define CONTROLS
#define DIALOGS
#define WINDOWS
#define FONTS

#include "toolbox.h"
#endif /* MAC */

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "props.h"
#include "sel.h"
#include "doc.h"
#include "disp.h"
#include "format.h"
#include "debug.h"
#include "file.h"
#include "ch.h"
#include "cmd.h"
#include "hyph.h"
#include "border.h"
#ifdef WIN
#include "prompt.h"
#include "message.h"
#include "rareflag.h"
#else
#include "mac.h"
#endif

#ifndef JR


/* E X T E R N A L S */
extern struct SEL       selCur;
extern struct MERR      vmerr;
extern int              wwCur;
extern struct FLI       vfli;
extern struct CHP       vchpFetch;
extern struct TCC       vtcc;
extern struct FTI       vfti;
Mac( extern			vfNoLlcPercent );
Win( extern struct PAP		vpapFetch );

/* G L O B A L S */

CP CpBestHypd();
struct HYPFD vhypfd;    /* hyphenation file state */

struct HYPB **vhhypb;   /* heap pointer to the parameter block used by
the hyphenation routines */


/* H Y P H  C H A N G E  S E L E C T I O N */
/* %%Function:HyphChangeSelection %%Owner:bryanl */
HyphChangeSelection(dxaHotZ, fCaps, pcpr)
uns dxaHotZ;
BOOL fCaps;
CPR *pcpr;
{
	if (pcpr != NULL)
		InitHyphCancelPoll(pcpr);
	StartLongOp();
	Mac( vfNoLlcPercent = fFalse );
	while (FHyphFind(dxaHotZ, fTrue, fCaps, pcpr) &&
			!vmerr.fDiskFail && !vmerr.fMemFail)
		{
		CP	cpBest;
		Win(struct CA	caInval);

		cpBest = CpBestHypd(&(*vhhypb)->hypd);
#ifdef SHOWBATCH
		TurnOffSel(&selCur);
		/* Since we don't have a dialog up in this context,
			we can get by as long as cp is visible. */
		AssureCpAboveDyp(cpBest, PwwdWw(wwCur)->ywMac, fTrue);
		Select(&selCur, cpBest, cpBest + 1);
		TurnOnSelCur();
#endif
		HyphInsertCp(cpBest);
#ifdef SHOWBATCH
		InvalCp(PcaSet( &caInval, selCur.doc, cpBest, cpBest + 1));
#endif
		}
}


/* H Y P H  I N S E R T  C P */
/* inserts a chNonReqHyphen at cp unless theres is already one there */
/* %%Function:HyphInsertCp %%Owner:bryanl */
HyphInsertCp(cp)
CP cp;
{
	if (ChFetch(selCur.doc, cp, fcmChars) != chNonReqHyphen)
		{
		char rgchNRH[2];
		rgchNRH[0] = chNonReqHyphen;
		if (!FInsertRgch(selCur.doc, cp, rgchNRH, 1, 0, 0))
			return;

		InvalFli(); /* since hyphenation mode is changed */

/* need to do FormatLine for previous line in case the user picked a
	hyphenation point that won't cause a line break to occur */
		if (PwwdWw(selCur.ww)->fPageView && (*vhhypb)->fForceWidth)
			FormatLineDxa(selCur.ww, selCur.doc, (*vhhypb)->cpLinePrev,
					(*vhhypb)->dxa);
		else
			FormatLine(selCur.ww, selCur.doc, (*vhhypb)->cpLinePrev);

		(*vhhypb)->cpLine = vfli.cpMac;

		(*vhhypb)->fDirty = fTrue;
		Win((*vhhypb)->cHyph++);
		(*vhhypb)->cpLim++;
		}
}


/* C P  B E S T  H Y P D */
/* %%Function:CpBestHypd %%Owner:bryanl */
CP CpBestHypd(phypd)
struct HYPD *phypd;
{
	Assert(phypd->iichBest >= 0);
	phypd->ichSel = phypd->rgich[phypd->iichBest] + 1;
	return (phypd->rgdcp[phypd->ichSel - 1] + 1
			+ phypd->cpFirst);
}


/* I I C H  B E S T  H Y P D */
/* returns iich of last hyphenation point which "fits" vfli line */
/* Scan back from ichLimEffective, because the sequential scan   */
/* either forward or backward using FFitHypdIch() is dependent   */
/* on the width of a character following the hyphenation point,  */
/* and misses the best one.                                      */
int
/* %%Function:IichBestHypd %%Owner:bryanl */
IichBestHypd()
{
	struct HYPD *phypd;
	int	iich, ich;

	Debug(phypd = &(*vhhypb)->hypd);
	Assert(phypd->iichMac > 0);
	ich = IchLimEffectiveHypd(); /* HM */
	phypd = &(*vhhypb)->hypd;
	if (ich >= 0)
		{
		for (iich = phypd->iichMac - 1; iich >= 0; iich--)
			{
			if (phypd->rgich[iich] < phypd->ichLimEffective)
				{
				break;
				}
			}
		}
	else
		{
		iich = -1;
		}
	return (phypd->iichBest = iich);
}


/* I C H  L I M  E F F E C T I V E  H Y P D */
/* Returns ich to the last position where inserting an hyphen will
	break a line. */
/* %%Function:IchLimEffectiveHypd %%Owner:bryanl */
int IchLimEffectiveHypd()
{
	struct HYPD *phypd;
	int	ich, fFit;

	phypd = &(*vhhypb)->hypd;
	phypd->ichLimEffective = -1;
	for (ich = phypd->ichMac - 1; ich > 0; ich--)
		{
		fFit = FFitHypdIch(ich); /* HM */
		phypd = &(*vhhypb)->hypd;
		if (fFit)
			{
			phypd->ichLimEffective = ich;
			break;
			}
		}
	return (phypd->ichLimEffective);
}


/* F  H Y P D  W I D O W */
/* returns true iff best iich would be a "widow".
widow is 1 char before, 1,2 char after
*/
/* %%Function:FHypdWidow %%Owner:bryanl */
FHypdWidow(phypd)
struct HYPD *phypd;
{
	int iich = phypd->iichBest;
	if (iich < 0)
		return fTrue;
	if (iich == phypd->iichMac - 1 &&
			phypd->ichMac - phypd->rgich[iich] - 1 <= 2)
		{
/* can we pick a lesser hyphen? */
		if (iich == 0) return fTrue;
		iich = phypd->iichBest = iich - 1;
		}
	if (iich == 0 && phypd->rgich[0] + 1 <= 1) return fTrue;
	return fFalse;
}


/* F  H Y P H  F I N D */
/* with *vhhypb:
Scans selCur.doc starting at cpLine, formatting.
Gives message and returns false if cpLim is reached.
Returns false is aborted.
(Wraps around end of doc if fWholeDocScan is set).
If Line can be shortened by hyphenation, returns true and cpLine is not
changed. Otherwise advances cpLine and loops.
*/
/* %%Function:FHyphFind %%Owner:bryanl */
FHyphFind(dxaHotZ, fBatch, fCaps, pcpr)
uns dxaHotZ;
BOOL fBatch, fCaps;
CPR *pcpr;
{
	int tmc;
	struct HYPB *phypb;
	BOOL fPageView = PwwdWw(selCur.ww)->fPageView;
	CP cpWordLim;
	CP cpWordStart;
	CP cp, cpLine;
	Mac( struct WDS wds );

	InvalFli(); /* since hyphenation mode is changed */
	SaveWds(&wds);
	PwwdSetWw(wwCur, cptDoc);       /* must have port for FormatLine */
LRestart:
	while ((cpLine = (*vhhypb)->cpLine) < (*vhhypb)->cpLim)
		{
		if (FHyphCancelPoll(pcpr, fBatch))
			{
			goto LDone;
			}
		if (fPageView && cpLine >= (*vhhypb)->cpLimDr)
/* end of current DR, find nex one */
			{
			struct PLDR **hpldr;
			int idr;
			BOOL fChangedPage;
			struct DR *pdr;
			struct DRF drf;
			CP cpFirst;

			UpdateWw(selCur.ww, fFalse);
			if (DlWhereDocCp(selCur.ww, selCur.doc, cpLine, fFalse,
					&hpldr, &idr, &cpFirst, &fChangedPage, fTrue)
					!= dlNil)
				{
				if (fChangedPage)
					{
					if (pcpr != NULL)
						InitHyphCancelPoll(pcpr);
					SetPageDisp(selCur.ww, IpgdCurrentWw(selCur.ww),
							YeTopPage(selCur.ww),
							fTrue, fTrue);
					}
				cpLine = (*vhhypb)->cpLine = cpFirst;
				pdr = PdrFetch(hpldr, idr, &drf);
				(*vhhypb)->fForceWidth = pdr->fForceWidth;
				if (!pdr->fInTable)
					(*vhhypb)->cpLimDr = pdr->cpLim;
				else
					{
					CacheTc(wwNil, selCur.doc, cpFirst, fFalse, fFalse);
					(*vhhypb)->cpLimDr = vtcc.cpLim;
					}
				(*vhhypb)->dxa = DxaFromDxp(
						PwwdWw(selCur.ww), pdr->dxl);
				FreePdrf(&drf);
				}
			}
#ifdef WIN
			{
			int	jcSave;
			int	dxaLine;

		/* Force it to format it as left justified to get a correct
			vfli.dxa for our purpose.  If this hack does not work
			for all the cases, because of potential CachePara calls
			from FormatLine, we should do this hack in CahcePara
			or FormatLine. */
			CachePara(selCur.doc, (*vhhypb)->cpLine);
			jcSave = vpapFetch.jc;
			vpapFetch.jc = jcLeft;
#endif /* WIN */

			if (fPageView && (*vhhypb)->fForceWidth)
				FormatLineDxa(selCur.ww, selCur.doc, cpLine,
						(*vhhypb)->dxa);
			else
				FormatLine(selCur.ww, selCur.doc, cpLine);

			if (vfli.chBreak != -1 && vfli.dcpDepend < 3 /* && vfli.chBreak != chNonReqHyphen*/)
				goto NoHyph;  /* can't fit any more on line */

#ifdef WIN
		/* Quickly restore the old jc to avoid further side
			effects. */
			vpapFetch.jc = jcSave;
			dxaLine = NMultDiv(vfli.xpRight - vfli.xpLeft, czaInch,
					vfti.dxpInch);
/* Bug fix 1/7/89 (BL): ignore hot zone & always hyphenate if we are dealing
	with a forced wrap in the middle of a word */
			if (vfli.chBreak != chNil && dxaHotZ > vfli.dxa - dxaLine)
				{
			/* Less than the specified hot zone */
				goto NoHyph;
				}
			}
#endif

		/* determine word at edge of line */
		cpWordLim = CpLimSty(selCur.ww, selCur.doc, vfli.cpMac, styWord, fFalse);
		cpWordStart = CpFirstSty(selCur.ww, selCur.doc, cpWordLim - 1, styWord, fFalse);

		if (cpWordStart < (*vhhypb)->cpLine)
			goto NoHyph;

		if (cpWordLim >= (*vhhypb)->cpLim)
			break;

		/* get word to be hyphenated */
		FreezeHp();
		FetchToHypd(selCur.doc, cpWordStart, cpWordLim, &(*vhhypb)->hypd);
		MeltHp();

		HyphenateWord(&(*vhhypb)->hypd, fCaps);
		if ((*vhhypb)->hypd.iichMac == 0) goto NoHyph;

/* determine if there is sufficient room to accept the first proposed
hyphenation point */

		if (!FFitHypdIch((*vhhypb)->hypd.rgich[0]))
			goto NoHyph;
		IichBestHypd();
		if (FHypdWidow(&(*vhhypb)->hypd)) goto NoHyph;
/* if there is already a hyphen at the selected place, do not stop */
		cp = CpBestHypd(&(*vhhypb)->hypd);
		if (ChFetch(selCur.doc, cp, fcmChars) == chNonReqHyphen)
			goto NoHyph;


		(*vhhypb)->cpLinePrev = vfli.cpMin;
		(*vhhypb)->cpLine = vfli.cpMac;
		Mac( RestoreWds(&wds) );
		return(fTrue);
NoHyph:
		(*vhhypb)->cpLine = vfli.cpMac;
		}

	Mac( BlankLlc() );
	phypb = *vhhypb;
	if (phypb->fWholeDocScan)
		{
		if (phypb->cpStart == cp0)
			{
			Debug(phypb = 0); /* can't assume no HM */
#ifdef WIN
			/* Since we use the prompt line for the "Hyphenation
				complete message." take the progress report down.*/
			if (pcpr != NULL && !(*vhhypb)->fHyphWord)
				{
				ChangeProgressReport(pcpr->hppr, 100);
				StopProgressReport(pcpr->hppr, pdcRestoreImmed);
				}
#endif
			IAlertHyphAn(anHyphEOD, fFalse);
			}
		else
			{
			Debug(phypb = 0); /* can't assume no HM */
#ifdef WIN
			if (pcpr != NULL && !(*vhhypb)->fHyphWord)
				{
				/* Make sure it says 100%. */
				ChangeProgressReport(pcpr->hppr, 100);
				}
#endif

			if (IAlertHyphAn(anHyphContinue, fTrue))
				{
				phypb = *vhhypb;
				/* Don't go through the places where you have
					already gone through. */
				phypb->cpLim = phypb->cpStart;
				phypb->cpStart = phypb->cpLine = cp0;
#ifdef WIN
				/* to avoid a bogus progress report. */
				phypb->hypd.cpFirst = cp0;
				/* Restart the progress report. */
				if (pcpr != NULL && !(phypb)->fHyphWord)
					{
					StopProgressReport(pcpr->hppr, pdcRestoreImmed);
					InitHyphCancelPoll(pcpr);
					}
#endif
				Debug(phypb = 0);
				goto LRestart;
				}
			}
		}
	else  if (!phypb->fHyphWord)
		{
#ifdef WIN
		/* Since we use the prompt line for the "Hyphenation
			complete message.", take the progress report down. */
		if (pcpr != NULL && !(phypb)->fHyphWord)
			{
			ChangeProgressReport(pcpr->hppr, 100);
			StopProgressReport(pcpr->hppr, pdcRestoreImmed);
			}
#endif
		Debug(phypb = 0); /* can't assume no HM */
		IAlertHyphAn(anHyphFini, fFalse);
		}

LDone:
	Mac( RestoreWds(&wds) );
	return(fFalse);
}


/* F  F I T  H Y P D  I C H */
/* returns true iff the hyphenation point after ich, if taken, would allow
the characters before the point to fit on the same line.
We assume that:
0. vfli is still valid.
1. characters before cpDepend fit.
2. the chNRH is inserted, would not be wider than 4 characters of any kind.
3. otherwise we compare the width of the - with the width of the character
it "displaces".
*/
/* %%Function:FFitHypdIch %%Owner:bryanl */
FFitHypdIch(ich)
int ich;
{
	struct HYPD *phypd;
	unsigned dxp, dxpHyphen;
	CP cp, cpDepend;

	phypd = &(*vhhypb)->hypd;
/* points where - is proposed to go after */
	cp = (phypd->rgdcp[ich]) + 1 + phypd->cpFirst;
	Debug (phypd = 0); /* can't assume no HM */

/* points to last acceptable char + 1 (last char assumed to have 1 bit width) */
	cpDepend = vfli.chBreak == -1 ? vfli.cpMac : vfli.cpMac + vfli.dcpDepend;

	if ((*vhhypb)->fHyphWord)
		{
		/* If hyphenating just a word, anything fits in. */
		return fTrue;
		}

	if (cp >= cpDepend)
		return fFalse; /* can't possibly fit */
	if (cp + 4 < cpDepend)
		return fTrue; /* Assumes hyphen can fit in space for any 4 chars. */

	/* else need to look at widths */
	CachePara(selCur.doc, cp);
/* ParseCaps is not used here for speed, but we check for smallcaps
conservatively. Caps has had an effect on rgch already. */
	FetchCp(selCur.doc, cp, fcmProps);
	PwwdSetWw(wwCur, cptSame);      /* need correct port */
	LoadFont(&vchpFetch, fTrue);
	phypd = &(*vhhypb)->hypd; /* HM */
	if (!vchpFetch.fSmallCaps)
		{
		FreezeHp();
		dxpHyphen = DxpFromCh(chHyphen, &vfti);
		dxp = DxpFromCh(phypd->rgch[++ich], &vfti) + 1;
		MeltHp();
		cpDepend--;
/* assume 1 pixel seen in last dependet char. This should be fixed by
making Format return the exact space left when the line ran out. */
		for (; ++cp < cpDepend && dxpHyphen >= dxp;)
			{
			CachePara(selCur.doc, cp);
			FetchCp(selCur.doc, cp, fcmProps);
			LoadFont(&vchpFetch, fTrue);
			phypd = &(*vhhypb)->hypd; /* HM */
			dxp += DxpFromCh(phypd->rgch[++ich], &vfti);
			}
		return dxpHyphen < dxp;
		}
	else
		{
		return fFalse;
		}
}


/* H Y P H E N A T E  W O R D */
/* given word in rgch in hypd, return in hypd the possible hyphenation points
in the word. The word must not contain non-req-hyphens.
fCaps: if true, do not hyphenate capitalized words.
*/
/* %%Function:HyphenateWord %%Owner:bryanl */
HyphenateWord(phypd, fCaps)
struct HYPD *phypd;
{
	int ch, ich, ichMac, iich;
	char *pch;
	char *pchTo;
	char rgch[cchMaxWord+1];
	char rgval[cchMaxWord+1];

/* first, check capitalization, strip non-alpha chars, check word length.
failure means no hyphenation points will be returned. */

	FreezeHp();
	phypd->iichMac = 0; /* prepare for returns */

	pch = &phypd->rgch[0];
	pchTo = &rgch[0];

	if (!fCaps && !FLower(*pch))
		{
		MeltHp();
		return;
		}
	*pchTo++ = chHyphBase; /* prepend boundary character */

	for (ich = 0; ich < phypd->ichMac; ich++, pch++, pchTo++)
		{
		ch = *pch;
		if (!(FLower(ch) || FUpper(ch)))
			break;
		*pchTo = ChLower(ch);
		}

	Assert(pchTo <= &rgch[cchMaxWord]);
	*pchTo = chHyphBase; /* append boundary character */

	ichMac = ich;
/* reject short words and check if the hyphenation database has been loaded */
	if (ichMac <= 4 || !FLoadHyph())
		{
		MeltHp();
		return;
		}

/* now find the hyphenation points */

	SetBytes(rgval, 0, ichMac+1);
	for (ich = ichMac; ich >= 0; ich--)
		{
		int t;
		int ival, op;
		struct TRIE HUGE *hptrie;
		struct OUTP *poutp;
		pch = &rgch[ich];
		ch = *pch;
		if (ch >= vhypfd.chHttFirst && ch <= vhypfd.chHttLast)
			ch = (**vhypfd.hrgbctt)[ch - vhypfd.chHttFirst]; /* translation table */
		t = ch;
		/* TrieEnsure(t) */
		hptrie = HpchFromFc(vhypfd.fn, (FC) (vhypfd.cbTrieBase + (t) * cchTRIE));
		while (hptrie->ch == ch)  /* ch already mapped */
			{
			op = hptrie->op;
			while (op != 0)
				{
				poutp = &(**vhypfd.hrgoutp)[op];
				ival = ich + poutp->dot - 1;
				if (poutp->val > rgval[ival])
					rgval[ival] = poutp->val;
				op = poutp->next;
				}
			if ((t = trie_yes(hptrie)) == 0)
				break;
			ch = *++pch;
			if (ch >= vhypfd.chHttFirst && ch <= vhypfd.chHttLast)
				ch = (**vhypfd.hrgbctt)[ch - vhypfd.chHttFirst]; /* translation table */
			t += ch;
			/* TrieEnsure(t) */
			hptrie = HpchFromFc(vhypfd.fn, (FC) (vhypfd.cbTrieBase + (t) * cchTRIE));
			}
		}
/* odd values in rgval[ich] mean hyphenation point after char ich */
	iich = 0;
/* exclude hyphenation points with 1 character separated? Other tuning? */
	for (ich = 2; ich < ichMac - 1; ich++)
		if (rgval[ich] & 1)
			phypd->rgich[iich++] = ich - 1;
	phypd->iichMac = iich;
	MeltHp();
}


#endif /* NOT JR */
