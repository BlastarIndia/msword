/*style sheet commands */

#ifdef MAC
#define EVENTS
#define CONTROLS
#define TEXTEDIT
#define WINDOWS
#define FONTS
#define DIALOGS

#include "resource.h"
#include "toolbox.h"
#include "style.h"
#endif

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "props.h"
#include "disp.h"
#include "sel.h"
#include "doc.h"
#include "file.h"
#include "prm.h"
#include "inter.h"

#ifdef MAC
#include "mac.h"
#include "dlg.h"
#endif

#include "debug.h"
#include "ch.h"
#include "ruler.h"
#include "cmd.h"
#include "error.h"

#ifdef WIN
#include "heap.h"
#include "idle.h"
#include "style.h"
#include "fontwin.h"
#endif


#ifdef PROTOTYPE
#include "stysubs.cpt"
#endif /* PROTOTYPE */

#ifdef MAC
extern int		vfStyleDirty;
extern struct ITR	vitr;
extern struct SPOP	vspop;
extern int		cwHeapFree;
extern int 		vfStyleNameDirty;
extern int		vfStyleDefnDirty;
#else
extern IDF		vidf;
#endif


/* E X T E R N A L S */
extern int docMac;
extern int vdocStsh;
extern int vdocFetch;
extern int vstcStyle;
extern int vstcBackup;
extern int vfShowStyles;
extern int vccpFetch;
extern CP vcpFetch;
extern struct CA caPara;
extern struct CA caHdt;
extern struct MERR vmerr;
extern struct SEL selCur;
extern struct RULSS vrulss;
extern struct CHP vchpFetch;
extern struct PAP vpapFetch;
extern char (**vhmpiststcp)[];
extern struct DOD **mpdochdod[];
extern int  vdocPapLast;
#ifdef MAC
extern int CbGenPapxFromPap(), CbGenChpxFromChp();
#endif /* MAC */

csconst CHAR stNormal[] = StKey("Normal",StyleNormal);

#ifdef MAC /* Not used by WIN */
/* %%Function:FChkNamesMatchOneStyle %%Owner:NOTUSED */
NATIVE FChkNamesMatchOneStyle(stNew, pstsh, pstcMatch, fDoAlert, mt, pcnk) /* WINIGNORE - MAC only */
char stNew[];
struct STSH *pstsh;
int *pstcMatch;
int fDoAlert;
int mt;
struct CNK *pcnk;
{
	int cchNew;
	int ichNew;
	int stcMatch;
	int stc;
	int fHaveName = fFalse;
	int fMatchStd;
	int fMultipleNames;
	struct STTB **hsttb;
	char stName[256];

	*(int *)pcnk = 0;
	cchNew = stNew[0];
	ichNew = 1;
	hsttb = pstsh->hsttbName;
	stcMatch = stcNil;
	while (FCopySubname(stNew, stName, &cchNew, &ichNew))
		{
		if (FMatchDefinedAndStdStyles(hsttb,
				pstsh->cstcStd, stName, mt, &stc, &fMatchStd,
				&fMultipleNames))
			{
			if (fMatchStd)
				pcnk->fMatchStd = fTrue;
			/* stc of matching name was returned */
			if (fMultipleNames)
				goto LMultipleNames;
			if (stcMatch == stcNil)
				{
				stcMatch = stc;
				pcnk->fMatchedOne = fTrue;
				}
			else  if (stcMatch != stc)
				{
LMultipleNames:
				pcnk->fMultipleNames = fTrue;
				if (fDoAlert)
					ErrorEid(eidMultiRefStyle,"FChkNamesMatchOneStyle");
				return (fFalse);
				}
			}
		else
			pcnk->fMismatch = fTrue;
		fHaveName = fTrue;
		}
	if (pcnk->fMismatch)
		{
		if (fDoAlert)
			ErrorEid(eidNotStyleName,"FChkNamesMatchOneStyle");
		return (fFalse);
		}
	if (!fHaveName)
		{
		pcnk->fInvalid = fTrue;
		if (fDoAlert)
			ErrorEid(eidInvalStyleName,"FChkNamesMatchOneStyle");
		return(fFalse);
		}

	*pstcMatch = stcMatch;
	return(fTrue);
}


#endif /* MAC */

#ifdef MAC /* Not used by WIN */
/* %%Function:FCopySubname %%Owner:NOTUSED */
NATIVE FCopySubname(stSrc, stName, pcchSrc, pichSrc) /* WINIGNORE - MAC only */
char stSrc[];
char stName[];
int *pcchSrc;
int *pichSrc;
{
	int cchSrc = *pcchSrc;
	int ichSrc = *pichSrc;

	int ichName = 0;
	int fFound = fTrue;

	while (ichName == 0)
		{
		if (cchSrc <= 0)
			{
			fFound = fFalse;
			break;
			}
		while (cchSrc > 0 && (stSrc[ichSrc] == vitr.chList || stSrc[ichSrc] == ' '))
			{
			ichSrc++;
			cchSrc--;
			}
		while  (cchSrc > 0 && stSrc[ichSrc] != vitr.chList)
			{
			stName[++ichName] = stSrc[ichSrc++];
			--cchSrc;
			}
		}
	stName[0] = ichName;
	*pichSrc = ichSrc;
	*pcchSrc = cchSrc;
	return(fFound);
}


#endif /* MAC */


#ifdef MAC /* Not used by WIN */
/* %%Function:FMatchDefinedAndStdStyles %%Owner:NOTUSED */
NATIVE FMatchDefinedAndStdStyles(hsttb, cstcStd, stName, mt, pstc, pfMatchStd, pfMultipleNames) /* WINIGNORE - MAC only */
struct STTB **hsttb;
int cstcStd;
char stName[];
int mt;
int *pstc;
int *pfMatchStd;
int *pfMultipleNames;
{
	int stc;
	int stcp;
	int stcpRet;
	int ibstMac;
	int fPartialMatch;
	int fHaveName;
	int fExact;
	int i;
	int iLim;
	int fStd;
	int isz;
	char st[256];

	Assert(!vfShowStyles || vstcBackup == stcNil);

	ibstMac = (*hsttb)->ibstMac;

/* we will make only one pass if we don't need case insensitive match */
	iLim = ((mt & mtUpperLower) == 0) ? 1 : 2;

/* set whether we allow matching by prefix or not. */
	fPartialMatch = ((mt & mtPartialMatch) != 0);

	*pfMultipleNames = *pfMatchStd = fHaveName = fFalse;

	for (i = 0; i < iLim && !fHaveName; i++)
		{
		if (i == 1)
			TranslateStUpper(stName);
		for (stcp = stcStdMin - 254 + cstcStd; stcp < ibstMac; stcp++)
			{
			if (stcp >= 0 && !FStcpEntryIsNull(hsttb, stcp) &&
					(vfShowStyles || vstcBackup == WinMac(stcNil, -1) ||
					stcp != StcpFromStc(vstcBackup, cstcStd)))
				{
				fStd = fFalse;
				GetStFromSttb(hsttb, stcp, st);
				if (i == 1)
					TranslateStUpper(st);
				if (FMatchSubnamesInSt(st, stName, fPartialMatch, &fExact))
					goto LFoundName;
				}
			if (stcp <= cstcStd)
				{
				GetStdStyleSt(cstcStd - stcp, st);
				if (i == 1)
					TranslateStUpper(st);
				if (FMatchSubnamesInSt(st, stName, fPartialMatch, &fExact))
					{
					fStd = fTrue;
LFoundName:
					if (fExact)
						{
						stcpRet = stcp;
						*pfMatchStd = fStd;
						if (i == 0)
							{
							fHaveName = fTrue;
							*pfMultipleNames = fFalse;
							break;
							}
						}
					if (!fHaveName)
						{
						stcpRet = stcp;
						*pfMatchStd = fStd;
						fHaveName = fTrue;
						}
					else
						*pfMultipleNames = fTrue;
					}
				}
			}
		}
	*pstc = StcFromStcp(stcpRet, cstcStd);
	return (fHaveName);
}


#endif /* MAC */

#ifdef MAC /* Not used by WIN */
/* %%Function:FMatchSubnamesInStExact %%Owner:NOTUSED */
NATIVE FMatchSubnamesInStExact(st, stName) /* WINIGNORE - MAC only */
char st[];
char stName[];
{
	int fExact;
	return (FMatchSubnamesInSt(st, stName, fFalse, &fExact));
}


#endif /* MAC */

#ifdef MAC /* Not used by WIN */
/* %%Function:FMatchSubnamesInSt %%Owner:NOTUSED */
NATIVE FMatchSubnamesInSt(st, stName, fPartial, pfExact) /* WINIGNORE - MAC only */
char st[];
char stName[];
int fPartial;
int *pfExact;
{
	char *pst, *pch;
	int cchName, cchSt;
	int ichName;
	int fPartialMatch;


	if ((cchName = stName[0]) > (cchSt = st[0]))
		return (fFalse);
	*pfExact = fFalse;
	fPartialMatch = fFalse;
	pch = &st[1];

	while (cchSt > 0)
		{
		ichName = 1;
		while (cchSt > 0 && ichName <= cchName &&
				stName[ichName] == *pch)
			{
			ichName++;
			cchSt--;
			pch++;
			}
		if (ichName > cchName)
			{
			if (cchSt == 0 || *pch == vitr.chList)
				{
				*pfExact = fTrue;
				return (fTrue);
				}
			if (fPartial)
				fPartialMatch = fTrue;
			}
		while (cchSt > 0 && *pch != vitr.chList)
			{
			cchSt--;
			pch++;
			}
		cchSt--;
		pch++;
		}
	return (fPartialMatch);
}


#endif /* MAC */


#ifdef MAC /* Not used by WIN */
/* %%Function:FAppendToStyleName %%Owner:NOTUSED */
NATIVE FAppendToStyleName(stActual, stName) /* WINIGNORE - MAC only */
char stActual[];
char stName[];
{
	char stComma[2];
	int cchActual = stActual[0];

	if (stName[0] == 0)
		return (fTrue);
	if (cchActual + stName[0] + ((cchActual == 0) ? 0 : 1) > 255)
		return(fFalse);

	if (cchActual)
		{
		stComma[0] = 1;
		stComma[1] = vitr.chList;
		StStAppend(stActual, stComma);
		}
	StStAppend(stActual, stName);
	return (fTrue);
}


#endif /* MAC */


#ifdef MAC /* Not used by WIN */
/* %%Function:IbstFromStyleName %%Owner:NOTUSED */
IbstFromStyleName(doc, stName)
int doc;
char stName[];
{
	struct STTB **hsttbName;
	int cstcStd, cstcStdT;
	struct DOD *pdod;
	int i;
	int ich;
	int bst;
	char *pst;
	int ibstMac;
	struct CNK cnk;
	int stc;
	int stcp;
	int stcpRet;
	char st[256];

	if (stName[0] == 0)
		return(-1);
	/* match stName with names recorded in
		hsttbName and standard style name table */

	pdod = PdodDoc(doc = DocMother(doc));
	hsttbName = pdod->stsh.hsttbName;
	cstcStd = pdod->stsh.cstcStd;
	if (FChkNamesMatchOneStyle(stName, &pdod->stsh, &stc, fFalse, 
			mtUpperLower+mtPartialMatch, &cnk))
		{
		stcpRet = StcpFromStc(stc, cstcStd);

/* make sure std styles are defined before we return. */

		if (stc > stcStdMin)
			{
			if (!FEnsureStcDefined(doc, stc, &cstcStdT))
				return(-1);
/* we know that when stc is an undefined std style FEnsureStcDefined will
	define it and that it will be the style with stcp == 0.    */
			if ((256 - stc) > cstcStd)
				stcpRet = 0;
			}
		}
	else
		{
		if (cnk.fMultipleNames)
			stcpRet = -2;
		else
			stcpRet = stcpNil;
		}
	return(stcpRet);
}


#endif /* MAC */


#ifdef MAC /* Not used by WIN */
/* %%Function:TranslateStUpper %%Owner:NOTUSED */
NATIVE TranslateStUpper(st) /* WINIGNORE - MAC only */
char st[];
{

	int ich;
	int cchLim = st[0] + 1;
	for (ich = 1; ich < cchLim; ich++)
		st[ich] = ChUpper(st[ich]);
}


#endif /* MAC */


/* %%Function:FRetargetStcBaseNext %%Owner:davidbo */
FRetargetStcBaseNext(doc, stcDel, stcMapTo, phplestcp)
int doc;
int stcDel;
int stcMapTo;
struct PLESTCP **phplestcp;
{
	int stcpMac;
	int cstcStd;
	int stcp;
	int stcpMapTo;
	int stcpDel;
	int stcBase;
	int stcNext;
	int fNextChanged;
	int fBaseChanged;
	struct STSH *pstsh;
	struct STTB **hsttbName;

	*phplestcp = hNil;
	pstsh = &(PdodDoc(doc))->stsh;
	hsttbName = pstsh->hsttbName;
	stcpMac = (*pstsh->hplestcp)->stcpMac;
	cstcStd = pstsh->cstcStd;
	stcpMapTo = StcpFromStc(stcMapTo, cstcStd);
	stcpDel = StcpFromStc(stcDel, cstcStd);
	for (stcp = 0; stcp < stcpMac; stcp++)
		{
		/* for each defined style in the stsh, compare the style's
			stcBase and stcNext with stcDel. Remap stcBase and stcNext to
			stcMapTo on any match. */
		if (FStcpEntryIsNull(hsttbName, stcp) || stcp == stcpDel)
			continue;
		GetStcBaseNextForStcp(pstsh, stcp, &stcBase, &stcNext);
		fNextChanged = fFalse;
		if (stcNext == stcDel)
			{
			stcNext = stcMapTo;
			fNextChanged = fTrue;
			}
		fBaseChanged = fFalse;
		if (stcBase == stcDel)
			{
			/* if the stcBase for stcpMapTo points to stcDel, we
				want to avoid mapping back to stcMapTo and looping
				stcBase, so remap to stcNormal. If we would be
				basing Normal on itself, use stcStdMin instead.
				Else just use stcMapTo.
				*/
			stcBase = (stcp == stcpMapTo) ?
					((stcp == cstcStd) ? stcStdMin : stcNormal) :
			stcMapTo;
			fBaseChanged = fTrue;
			}
		if (fNextChanged || fBaseChanged)
			{
			if (*phplestcp == hNil &&
					(*phplestcp = HCopyHeapBlock(PdodDoc(doc)->stsh.hplestcp, strcPL)) == hNil)
				return (fFalse);
			/* regenerate because of Heap Movement */
			pstsh = &(PdodDoc(doc))->stsh;
			SetStcBaseNextForStcp(pstsh, stcp, stcBase, stcNext);
			if (fBaseChanged && !FGenChpxPapxNewBase(doc, stcp))
				return fFalse;
			/* regenerate because of Heap Movement */
			pstsh = &(PdodDoc(doc))->stsh;
			}
		}
	FreePhpl(phplestcp);
	return fTrue;
}


/*
* returns stcp of empty slot in stsh.  returns stcpNil when stsh full or
* out of memory.
*/
/* %%Function:StcpCreateNewStyle %%Owner:davidbo */
StcpCreateNewStyle(doc, fUseReserve)
int doc;
int fUseReserve;
{
	char stNull[1];
	int stcp;
	int fExpand, fChngEstcp = fFalse;
	int fSkipFirst;
	int ibstMac;
	struct ESTCP estcp;
	struct STSH stsh;
	struct STTB **hsttbChpe, **hsttbPape;

	RecordStshForDoc(doc, &stsh, &hsttbChpe, &hsttbPape);

/* when fUseReserve == fFalse and a backup stcp is not allocated, we need to
	retain 1 entry in stsh for backup.
	when fTrue, we are allocating the backup. */
	fSkipFirst = !fUseReserve && (vstcBackup == WinMac(stcNil, -1)
			|| vstcBackup == stcStdMin);
	ibstMac = (*stsh.hsttbName)->ibstMac;
	for (stcp = stsh.cstcStd; stcp < ibstMac; stcp++)
		{
		if (FStcpEntryIsNull(stsh.hsttbName, stcp) && (vstcBackup == stcNil ||
				stcp != StcpFromStc(vstcBackup, stsh.cstcStd)))
			{
			if (!fSkipFirst)
				break;
			fSkipFirst = fFalse;
			}
		}
	if (StcFromStcp(stcp, stsh.cstcStd) >= stcStdMin - fSkipFirst)
		return(stcpNil);

	fExpand = (stcp == ibstMac);

	estcp.stcNext = StcFromStcp(stcp, stsh.cstcStd);
	estcp.stcBase = stcNormal;
	if (fExpand)
		{
		if (!FInsertInPl(stsh.hplestcp, stcp, &estcp))
			goto LErrRet0;
		if (!FStretchSttb(stsh.hsttbName, 1, 1) ||
				!FStretchSttb(hsttbChpe, 1, 1) ||
				!FStretchSttb(hsttbPape, 1, 1) ||
				!FStretchSttb(stsh.hsttbChpx, 1, 1) ||
				!FStretchSttb(stsh.hsttbPapx, 1, 1))
				{
				((struct PL *)*stsh.hplestcp)->iMac--;
				goto LErrRet0;
				}
			
		}
	else
		bltb(&estcp, PInPl(stsh.hplestcp, stcp), sizeof(struct ESTCP));

	stNull[0] = 0377;
	/* can't fail since st in sttb is already >= 1 byte in length */
	AssertDo(FChangeStInSttb(stsh.hsttbName, stcp, stNull));

	if (!FCopyStInSttb(hsttbChpe, stsh.cstcStd, stcp))
		goto LErrRet2;

	if (!FGenPapeFromOldPape(stcp, stsh.cstcStd, stsh.cstcStd, hsttbPape))
		goto LErrRet2;

	if (!FGenChpxPapxNewBase(doc, stcp))
		goto LErrRet2;
	return(stcp);

LErrRet2:
	/* can't fail since st in sttb is already >= 1 byte in length */
	AssertDo(FChangeStInSttb(stsh.hsttbName, stcp, stNull));
	AssertDo(FChangeStInSttb(stsh.hsttbChpx, stcp, stNull));
	AssertDo(FChangeStInSttb(stsh.hsttbPapx, stcp, stNull));
	AssertDo(FChangeStInSttb(hsttbChpe, stcp, stNull));
	AssertDo(FChangeStInSttb(hsttbPape, stcp, stNull));
LErrRet0:
	Assert((*stsh.hsttbName)->ibstMac == (*stsh.hsttbChpx)->ibstMac &&
		(*stsh.hsttbName)->ibstMac == (*stsh.hsttbPapx)->ibstMac &&
		(*stsh.hsttbName)->ibstMac == (*hsttbChpe)->ibstMac &&
		(*stsh.hsttbName)->ibstMac == (*hsttbPape)->ibstMac &&
		(*stsh.hsttbName)->ibstMac == ((struct PL *)*stsh.hplestcp)->iMac); 
	return stcpNil;
}



#ifndef JR
/* %%Function:CopyStyleWithinStsh %%Owner:davidbo */
CopyStyleWithinStsh(doc, stcFrom, stcTo)
int doc;
int stcFrom;
int stcTo;
{
	AssertDo(FCopyStyleWithinStsh(doc, stcFrom, stcTo));
}


/* %%Function:FCopyStyleWithinStsh %%Owner:davidbo */
FCopyStyleWithinStsh(doc, stcFrom, stcTo)
int doc;
int stcFrom;
int stcTo;
{
	struct STSH stsh;
	struct STTB **hsttbChpe, **hsttbPape;
	int stcpFrom;
	int stcpTo;
	int stcBase, stcNext;
	char stDest[cchMaxSz];

	RecordStshForDoc(doc, &stsh, &hsttbChpe, &hsttbPape);
	stcpFrom = StcpFromStc(stcFrom, stsh.cstcStd);
	stcpTo = StcpFromStc(stcTo, stsh.cstcStd);

	GetStcBaseNextForStcp(&stsh, stcpFrom, &stcBase, &stcNext);
	SetStcBaseNextForStcp(&stsh, stcpTo, stcBase, stcNext);

	GetStFromSttb(stsh.hsttbName, stcpFrom, stDest);
	if (!FChangeStInSttb(stsh.hsttbName, stcpTo, stDest))
		return (fFalse);

	GetStFromSttb(stsh.hsttbChpx, stcpFrom, stDest);
	if (!FChangeStInSttb(stsh.hsttbChpx, stcpTo, stDest))
		return (fFalse);

	GetStFromSttb(stsh.hsttbPapx, stcpFrom, stDest);
	if (!FChangeStInSttb(stsh.hsttbPapx, stcpTo, stDest))
		return (fFalse);

	GetStFromSttb(hsttbChpe, stcpFrom, stDest);
	if (!FChangeStInSttb(hsttbChpe, stcpTo, stDest))
		return (fFalse);

	GetStFromSttb(hsttbPape, stcpFrom, stDest);
	if (!FChangeStInSttb(hsttbPape, stcpTo, stDest))
		return (fFalse);

	return fTrue;
}


#endif /* NOT JR */


/* %%Function:FCopyStInSttb %%Owner:davidbo */
FCopyStInSttb(hsttb, stcpOld, stcpNew)
struct STTB **hsttb;
int stcpOld;
int stcpNew;
{
	char stProp[cchMaxSz];

	GetStFromSttb(hsttb, stcpOld, stProp);
	return (FChangeStInSttb(hsttb, stcpNew, stProp));
}



/* %%Function:FGenPapeFromOldPape %%Owner:davidbo */
FGenPapeFromOldPape(stcp, stcpOld, cstcStd, hsttbPape)
int stcp;
int stcpOld;
int cstcStd;
struct STTB **hsttbPape;
{
	char pap[cbPAP + 1];

	SetBytes(&pap, 0, cbPAP + 1);
	GetStFromSttb(hsttbPape, stcpOld, &pap);
	bltb(((char *)&pap) + 1, &pap, cbPAP);
	((struct PAP *)&pap)->stc = StcFromStcp(stcp, cstcStd);
	((struct PAP *)&pap)->phe.fStyleDirty = fFalse;
	return (FStorePropeForStcp(&pap, stcp, hsttbPape, fFalse));
}


/* %%Function:GenFilePropxFromProp %%Owner:davidbo */
GenFilePropxFromProp(rgbResult, pprop, ppropBase, fPara)
char rgbResult[];
char *pprop;
char *ppropBase;
int fPara;
{
	int cchResult;

	cchResult = (fPara ?
			CbGenPapxFromPap(rgbResult, pprop, ppropBase, fFalse) :
			CbGenChpxFromChp(rgbResult, pprop, ppropBase, fFalse));
#ifdef DEBUG
	if (fPara)
		{
		int cbPrl = cchResult - cbPHE - 1; 
		char *pprl = rgbResult + cbPHE + 1; 
		int cch; 
		if (cbPrl > 0)
			{
			char *pprlLim = pprl + cbPrl;
			int cbPrlActual = 0; 
			while (pprl < pprlLim)
				{
				pprl += (cch = CchPrl(pprl)); 
				cbPrlActual += cch; 
				}
			Assert(cbPrl == cbPrlActual); 
			}
		}
#endif

	if (fPara && cchResult > 255)
		cchResult = cbPHE + 1; 
	bltb(rgbResult, rgbResult+1, cchResult);
	rgbResult[0] = cchResult;
}



/* %%Function:FGenChpxPapxNewBase %%Owner:davidbo */
FGenChpxPapxNewBase(doc, stcp)
int doc;
int stcp;
{
	int stc;
	int stcBase;
	struct STSH stsh;
	struct DOD *pdod;
	struct PAP papBase, pap;
	struct CHP chpBase, chp;
	struct STTB **hsttbChpe, **hsttbPape;
	char stNil[1];
	char rgbResult[cchPapxMax+1];

	RecordStshForDoc(doc, &stsh, &hsttbChpe, &hsttbPape);
	stcBase = ((struct ESTCP *)PInPl(stsh.hplestcp, stcp))->stcBase;
	pdod = PdodDoc(doc);

	stc = StcFromStcp(stcp, stsh.cstcStd);
	MapStc(pdod, stc, &chp, &pap);

	if (stcp < stsh.cstcStd && stcBase == stcNormal)
		{
/*
*  if a standard style is based on stcNormal and the chp and pap reported by
*  MapStc are equal to those reported by MapStcStandard, then we can code
*  the chpx, papx, chpe, and pape as stNil in the STSH.
*/
		MapStc(pdod, stcNormal, &chpBase, &papBase);
		MapStcStandard(stc, &chpBase, &papBase);
		if (!FNeRgch(&chp, &chpBase, cbCHP) &&
				!FNeRgw(&pap, &papBase, cwPAPBase) &&
				!FNeRgw(pap.rgdxaTab, papBase.rgdxaTab, pap.itbdMac) &&
				!FNeRgch(pap.rgtbd, papBase.rgtbd, pap.itbdMac))
			{
			stNil[0] = 255;
			/* cannot fail because sizeof(stNil) <= entry in sttb */
			AssertDo(FChangeStInSttb(stsh.hsttbChpx, stcp, stNil));
			AssertDo(FChangeStInSttb(stsh.hsttbPapx, stcp, stNil));
			AssertDo(FChangeStInSttb(hsttbChpe, stcp, stNil));
			AssertDo(FChangeStInSttb(hsttbPape, stcp, stNil));
			return fTrue;
			}
		}
	MapStc(pdod, stcBase, &chpBase, &papBase);
	GenFilePropxFromProp(rgbResult, &chp, &chpBase, fFalse /* fPara */);
	if (!FChangeStInSttb(stsh.hsttbChpx, stcp, rgbResult))
		return fFalse;

	GenFilePropxFromProp(rgbResult, &pap, &papBase, fTrue /* fPara */);
	if (!FChangeStInSttb(stsh.hsttbPapx, stcp, rgbResult))
		return fFalse;

	return fTrue;
}


/* %%Function:RetrievePropeForStcp %%Owner:davidbo */
RetrievePropeForStcp(ppropeResult, stcp, hsttb, fChp)
char *ppropeResult;
int stcp;
struct STTB **hsttb;
int fChp;
{

	char HUGE *hpprope;
	int stc;
	int cb, cch;
	struct CHP *pchp;
	struct PAP *ppap;
	struct CHP chp;
	struct PAP pap;

	hpprope = HpstFromSttb(hsttb, stcp);
	if ((cch = *hpprope++) == 255)
		{
		struct DOD *pdod;
		pchp = fChp ? ppropeResult : &chp;
		ppap = fChp ? &pap : ppropeResult;
		if (!fChp)
			SetBytes(ppropeResult, 0, cbPAP);
		pdod = PdodMother(selCur.doc);
		MapStc(pdod, 0, pchp, ppap);
		stc = (stcp - pdod->stsh.cstcStd) & 255;
		Assert(stcStdMin < stc && stc <= 255);
		MapStcStandard(stc, pchp, ppap);
		return;
		}
	bltbx(LpFromHp(hpprope), (char far *)ppropeResult, cch);
	cb = fChp ? cbCHP : cbPAP;
	SetBytes(((char *)ppropeResult) + cch, 0, cb - cch);
}



/* %%Function:FStorePropeForStcp %%Owner:davidbo */
FStorePropeForStcp(pprope, stcp, hsttb, fChp)
char *pprope;
int stcp;
struct STTB **hsttb;
int fChp;
{
	int cb;
	int cch;
	char rgbProp[cbPAP+1];

	cb = fChp ? cbCHP :
			cwPAPBase * sizeof(int) + itbdMax * sizeof(int) +
			((struct PAP *)pprope)->itbdMac;
	Assert(cb <= cbPAP);
	bltb(pprope, rgbProp+1, cb);

	cch = CchNonZeroPrefix(rgbProp+1, cb);

	rgbProp[0] = cch;

	return (FChangeStInSttb(hsttb, stcp, rgbProp));
}


#ifndef JR
/* %%Function:FRecalcAllDependentChpePape %%Owner:davidbo */
FRecalcAllDependentChpePape(pstsh, hsttbChpe, hsttbPape, stcpChanged)
struct STSH *pstsh;
struct STTB **hsttbChpe;
struct STTB **hsttbPape;
int stcpChanged;
{
/* operates on vdocStsh */
	int stcp;
	int stcpMac;
	int stc;
	int stcBase;
	int stcpBase;
	char *ppape;
	int fStyDirtyBroken;
	int fChanged;
	struct STSH stsh;
	struct PLESTCP *pplestcp;
	struct CHP chp, chpBase;
	struct PAP pap, papBase;
	char rgbProps[cbPAP + 1];
#ifdef WIN
	char rgBuffer[cchMaxSt]; /* for MakeChp/PapFromChpx/Papx... */
#endif

	fStyDirtyBroken = PdodDoc(vdocStsh)->fStyDirtyBroken;
	fChanged = fFalse;
	bltb(pstsh, &stsh, cbSTSH);
	stcpMac = (*stsh.hsttbChpx)->ibstMac;
	for (stcp = 0; stcp < stcpMac; stcp++)
		/* examine each valid style in stsh */
		{
		if (FStcpStyleBaseChanged(&stsh, stcp, stcpChanged))
			{
			fChanged = fTrue;
			if (stcp < stsh.cstcStd)
				{
				stcBase = ((struct ESTCP *)PInPl(stsh.hplestcp, stcp))->stcBase;
				/*
				*  when a standard style is based on stcNormal
				*  and its properties are unchanged from the
				*  standard,  MapStcStandard will generate
				*  the correct property and does not require
				*  expanded properties, so continue.
				*/
				if (stcBase == stcNormal && FStcpEntryIsNull(hsttbPape, stcp))
					continue;
				}
			/* the stcp under consideration does depend on
				the stcp that was changed, stcpChanged */
			stc = StcFromStcp(stcp, stsh.cstcStd);
			SetBytes(rgbProps, 0, cbCHP);
#ifdef WIN
			MakeChpFromChpxBaseChp(rgbProps, stc, stsh.hsttbChpx,
					*stsh.hplestcp, stsh.cstcStd, 0, rgBuffer);
#else
			MakeChpFromChpxBaseChp(rgbProps, stc, stsh.hsttbChpx,
					*stsh.hplestcp, stsh.cstcStd, 0);
#endif
			if (!FStorePropeForStcp(rgbProps, stcp, hsttbChpe, fTrue))
				return fFalse;
			SetBytes(rgbProps, 0, cbPAP);
#ifdef WIN
			MakePapFromPapxBasePap(rgbProps, stc, stsh.hsttbPapx,
					*stsh.hplestcp, stsh.cstcStd, 0, rgBuffer);
#else
			MakePapFromPapxBasePap(rgbProps, stc, stsh.hsttbPapx,
					*stsh.hplestcp, stsh.cstcStd, 0);
#endif
			/* this throws away paph's on the file */
			if (!fStyDirtyBroken)
				((struct PAP *)rgbProps)->phe.fStyleDirty = fTrue;
			if (!FStorePropeForStcp(rgbProps, stcp, hsttbPape, fFalse))
				return fFalse;

			/* now we have recalculated and stored a new Chpe
				and Pape for the style that was based on
				stcpChanged. */
			}
		}

	if (fChanged)
		InvalDoc(vdocStsh);
	return fTrue;
}


/* %%Function:FStcpStyleBaseChanged %%Owner:davidbo */
FStcpStyleBaseChanged(pstsh, stcpCheck, stcpChanged)
struct STSH *pstsh;
int stcpCheck;
int stcpChanged;
{
	int stcpBase;
	int stcBase;
	int cstcStd;
	struct PLESTCP *pplestcp;

	cstcStd = pstsh->cstcStd;

	if (FStcpEntryIsNull(pstsh->hsttbName, stcpCheck) ||
			(vstcBackup != stcNil && stcpCheck == StcpFromStc(vstcBackup, cstcStd)))
		return (fFalse);
	pplestcp = *pstsh->hplestcp;
	stcpBase = stcpCheck;
	while ((stcBase = (pplestcp->dnstcp[stcpBase].stcBase)) != stcStdMin)
		{
		stcpBase = StcpFromStc(stcBase, cstcStd);
		if (stcpBase == stcpChanged)
			return (fTrue);
		}
	return (fFalse);
}


#endif /* NOT JR */

/* %%Function:GetStcBaseNextForStcp %%Owner:davidbo */
GetStcBaseNextForStcp(pstsh, stcp, pstcBase, pstcNext)
struct STSH *pstsh;
int stcp;
int *pstcBase;
int *pstcNext;
{
	struct ESTCP *pestcp;
	int stc;

	if (stcp < (*pstsh->hsttbName)->ibstMac)
		{
		pestcp = PInPl(pstsh->hplestcp, stcp);
		*pstcBase = pestcp->stcBase;
		*pstcNext = pestcp->stcNext;
		}
	else
		{
		*pstcBase = *pstcNext = stcNormal;
		stc = StcFromStcp(stcp, pstsh->cstcStd);
		switch (stc)
			{ /* standard styles that are not defined yet */
#ifdef WIN
		default:
			if (stc <= stcLev3 && stc >= stcLev9)
				*pstcNext = stcNormIndent;
			break;
		case stcIndexHeading:
			*pstcNext = stcIndex1;
			break;
		case stcAtnText:
#endif
		case stcFtnText:
		case stcHeader:
		case stcFooter:
			*pstcNext = stc;
			break;
			}
		}
}


/* %%Function:SetStcBaseNextForStcp %%Owner:davidbo */
SetStcBaseNextForStcp(pstsh, stcp, stcBase, stcNext)
struct STSH *pstsh;
int stcp;
int stcBase;
int stcNext;
{
	struct ESTCP *pestcp;

	pestcp = PInPl(pstsh->hplestcp, stcp);
	pestcp->stcBase = stcBase;
	pestcp->stcNext = stcNext;
}



#ifndef JR
/* %%Function:GenApplyStcPermuteToDoc %%Owner:davidbo */
GenApplyStcPermuteToDoc(doc, stcFrom, stcTo)
int doc;
int stcFrom;
int stcTo;
{
	int stc;
	struct CA ca;
	char rgb[258];

	Assert(stcTo >= 0 && stcTo < 256 && stcFrom > 0 && stcFrom < 256);
	Assert(stcFrom != stcStdMin);
	for (stc = 1; stc < stcFrom; stc++)
		rgb[1 + stc] = stc;
	rgb[1 + stcFrom] = stcTo;
	rgb[0] = sprmPStcPermute;
	/* fill in size of permutation */
	rgb[1] = stcFrom;
	ApplyGrpprlDocSubdocs(doc, rgb, stcFrom + 2);
}


/* %%Function:ApplyGrpprlDocsSubdocs %%Owner:davidbo */
ApplyGrpprlDocSubdocs(doc, rgb, cb)
int doc;
char rgb[];
int cb;
{
	int docT;
	struct DOD *pdod;

#ifdef MAC
	if (vrulss.caRulerSprm.doc != docNil)
		FlushRulerSprms();
#endif
	pdod = PdodDoc(doc = DocMother(doc));
	ApplyGrpprlWholeDoc(doc, rgb, cb);
	/* apply to all associated docs */
	for (docT = docMinNormal; docT < docMac; docT++)
		if (mpdochdod [docT] != hNil && PdodDoc(docT)->doc == doc)
			ApplyGrpprlWholeDoc(docT, rgb, cb);
}


/* A P P L Y  G R P P R L  W H O L E  D O C */
/* %%Function:ApplyGrpprlWholeDoc %%Owner:davidbo */
ApplyGrpprlWholeDoc(doc, rgb, cb)
int doc;
char rgb[];
int cb;
{
	struct CA ca;

#ifdef WIN
/* flush any pending sprms in ruler sprm cache */
	if (vrulss.caRulerSprm.doc != docNil)
		FlushRulerSprms();
#endif

	ApplyGrpprlCa(rgb, cb, PcaSet(&ca, doc, cp0, CpMacDoc(doc)));
	InvalCp(&ca);

#ifdef WIN
	InvalText (&ca, fFalse /* fEdit */);
	if (PdodDoc(ca.doc)->hplcfld != hNil && !PdodDoc(ca.doc)->fHasNoSeqLev)
		PdodDoc(ca.doc)->fInvalSeqLev = vidf.fInvalSeqLev = fTrue;
#endif
}


/* %%Function:InvalStyleEntry %%Owner:davidbo */
InvalStyleEntry(pstsh, hsttbChpe, hsttbPape, stcp)
struct STSH *pstsh;
int stcp;
{
	struct ESTCP *pestcp;
	char stNil[1];

	/* make name nil */
	stNil[0] = 255;
	/* these should not fail since stNil is same or smaller than what is
		already there */
	AssertDo(FChangeStInSttb(pstsh->hsttbName, stcp, stNil));
	AssertDo(FChangeStInSttb(pstsh->hsttbChpx, stcp, stNil));
	AssertDo(FChangeStInSttb(pstsh->hsttbPapx, stcp, stNil));
	AssertDo(FChangeStInSttb(hsttbChpe, stcp, stNil));
	AssertDo(FChangeStInSttb(hsttbPape, stcp, stNil));
	pestcp = PInPl(pstsh->hplestcp, stcp);
	pestcp->stcBase = stcNormal;
	pestcp->stcNext = stcNormal;
}


#endif /* NOT JR */


/* I N V A L  A L L  D E P E N D E N T  P A D  P G D  */
/* %%Function:InvalAllDependentPadPgd %%Owner:davidbo */
InvalAllDependentPadPgd(pstsh, hsttbChpe, hsttbPape, stcpChanged)
struct STSH *pstsh;
struct STTB **hsttbChpe;
struct STTB **hsttbPape;
int stcpChanged;
{
/* operates on vdocStsh */
	int stcp;
	int stcpMac;
	int stc;
	int stcBase;
	int stcpBase;
	char *ppape;
	struct STSH stsh;


	bltb(pstsh, &stsh, cbSTSH);
	stcpMac = (*stsh.hsttbChpx)->ibstMac;
	for (stcp = 0; stcp < stcpMac; stcp++)
		/* examine each valid style in stsh */
		{
		if (FStcpStyleBaseChanged(&stsh, stcp, stcpChanged))
			{
			/* the stcp under consideration does depend on
				the stcp that was changed, stcpChanged */
			stc = StcFromStcp(stcp, stsh.cstcStd);
			/* dirty all pad entries for paras with this style */
			InvalPadPgdForStc(vdocStsh, StcFromStcp(stcp, stsh.cstcStd));
			}
		}
}


/* I N V A L  P A D  P G D  F O R  S T C  */
/* %%Function:InvalPadPgdForStc %%Owner:davidbo */
InvalPadPgdForStc(doc, stc)
int doc;
int stc;
{
	int ipad, ipadMac;
	int iphe, ipheMac;
	CP cpLast;
	int fCachePara;
	int docFtn, docHdr;
	struct DOD *pdod = PdodDoc(doc);
	struct PAD pad;
	struct PHE phe;
	struct PLCPAD **hplcpad;
	struct PLCPGD **hplcpgd;
	struct PLCPHE **hplcphe;
/* limit how many paragraphs we will examine when we try to validate
	phe entries. */
#define ipheFixMac 1000

	caHdt.doc = docNil;	/* headers may be affected, get rid of LRs */
	hplcpgd = pdod->hplcpgd;
	if ((hplcphe = pdod->hplcphe) != hNil)
		{
		ipheMac = IMacPlc(hplcphe);
		cpLast = cp0;
		for (iphe = 0; iphe < ipheMac; iphe++)
			{
/* we will try to bless entries in the hplcphe as long as we have not
	reached ipheFixMac and have not passed beyond selCur.cpLim. */
			if (fCachePara = (iphe < ipheFixMac || (selCur.doc == doc &&
					cpLast < selCur.cpLim)))
				CachePara(doc, cpLast = CpPlc(hplcphe, iphe));
			if (!fCachePara || vpapFetch.stc == stc)
				{
				GetPlc(hplcphe, iphe, &phe);
				phe.fUnk = fTrue;
				PutPlcLast(hplcphe, iphe, &phe);
				}
			}
		}
	if ((docFtn = pdod->docFtn) != docNil &&
			(hplcphe = (PdodDoc(docFtn))->hplcphe) != hNil)
		InvalAllPheEntries(hplcphe);
	if ((docHdr = pdod->docHdr) != docNil &&
			(hplcphe = (PdodDoc(docHdr))->hplcphe) != hNil)
		InvalAllPheEntries(hplcphe);
	/* the only paras we know have been invalidated are
		those just identified from the pad, which may be
		incomplete; we're therefore uncertain about what
		pages are dirty, so we invalidate the entire pgd */
	if (hplcpgd != hNil)
		SetPlcUnk(hplcpgd, cp0, CpMacDocEdit(vdocStsh));
}


/* %%Function:InvalAllPheEntries %%Owner:davidbo */
InvalAllPheEntries(hplcphe)
struct PLC **hplcphe;
{
	int iphe;
	struct PHE phe;
	int ipheMac = IMacPlc(hplcphe);

	for (iphe = 0; iphe < ipheMac; iphe++)
		{
		GetPlc(hplcphe, iphe, &phe);
		phe.fUnk = fTrue;
		PutPlcLast(hplcphe, iphe, &phe);
		}
}


/* Majority CHP gunk */

#ifdef WIN
#define imaskMac        (4)
#else
#define imaskMac        (8)
#endif
#define ifooMac         (5)
/* ftc, hps, hpsPos, kul, qpsSpace, ico */
#define cfoo            (6)


/* G E T  M A J O R I T Y  C H P  S E L S  */
/* %%Function:GetMajoritySelsChp %%Owner:davidbo */
GetMajoritySelsChp(psels, pchp)
struct SELS *psels;
struct CHP *pchp;
{
	struct CA ca;

	FirstCaForSels(psels, &ca);
	GetMajorityChp(&ca, pchp);
}


/* G E T  M A J O R I T Y  C H P */
/* %%Function:GetMajorityChp %%Owner:davidbo */
GetMajorityChp(pca, pchp)
struct CA *pca;
struct CHP *pchp;
{
	int imask, ifoo, ccp, w0, voteMaj;
	CP cpFirst, cpLim, dcp;
	int *pmask, *pfoo, *pgrpfoo;
	CHAR *pvoteMask, *pgrpvote;
	int rgmask[imaskMac];
	CHAR rgvoteMask[imaskMac];
	int rgfoo[cfoo], rggrpfoo[cfoo*ifooMac];
	CHAR rggrpvote[cfoo*ifooMac];

	SetBytes(rgvoteMask, 0, imaskMac);
	SetWords(rggrpfoo, 0, cfoo*ifooMac);
	SetBytes(rggrpvote, 0, cfoo*ifooMac);
	SetBytes(pchp, 0, cbCHPBase);

	pmask = rgmask;
#ifdef WIN
	*pmask++ = maskfBold;
	*pmask++ = maskfItalic;
	*pmask++ = maskfSmallCaps;
	*pmask++ = maskfVanish;
#else
	*pmask++ = maskfBold;
	*pmask++ = maskfItalic;
	*pmask++ = maskfStrike;
	*pmask++ = maskfOutline;
	*pmask++ = maskfShadow;
	*pmask++ = maskfSmallCaps;
	*pmask++ = maskfCaps;
	*pmask++ = maskfVanish;
#endif

	CacheParaCa(pca);
	/* insertion point? scan whole para */
	if (DcpCa(pca) == cp0)
		{
		cpFirst = caPara.cpFirst;
		dcp = DcpCa(&caPara);
		}
	/* if not, use current selection limited to end of current para */
	else
		{
		cpFirst = pca->cpFirst;
		dcp = CpMin(DcpCa(pca), caPara.cpLim - pca->cpFirst);
		}
	/* also limit run to 255 chars */
	dcp = CpMin((CP) 255, dcp);
	cpLim = cpFirst + dcp;

	FetchCp(pca->doc, cpFirst, fcmProps);
	for (;;)
		{
		ccp = CpMin((CP) vccpFetch, cpLim - vcpFetch);
		w0 = *((int *) &vchpFetch);

		/* deal first with the toggle properties in word0 of the chp */
		pmask = rgmask;
		pvoteMask = rgvoteMask;
		for (imask = 0; imask < imaskMac; imask++)
			{
			if (w0 & *pmask)
				{
				Assert(*pvoteMask + ccp < 256);
				*pvoteMask += ccp;
				}
			pmask++;
			pvoteMask++;
			}

		/* now deal with multi-value properties of the chp */
		pfoo = rgfoo;
		*pfoo++ = vchpFetch.ftc;
		*pfoo++ = vchpFetch.hps;
		*pfoo++ = vchpFetch.hpsPos;
		*pfoo++ = vchpFetch.kul;
		*pfoo++ = vchpFetch.qpsSpace;
		*pfoo++ = vchpFetch.ico;
		pgrpfoo = rggrpfoo;
		pgrpvote = rggrpvote;
		pfoo = rgfoo;
		for (ifoo = 0; ifoo < cfoo; ifoo++)
			{
			MultiValueVote(pgrpfoo, pgrpvote, *pfoo, ccp);
			pgrpfoo += ifooMac;
			pgrpvote += ifooMac;
			pfoo++;
			}

		if (vcpFetch + vccpFetch >= cpLim)
			break;
		FetchCp(docNil, cpNil, fcmProps);
		}

	/* generate chp */
	voteMaj = (dcp + 1) / 2;
	pmask = rgmask;
	pvoteMask = rgvoteMask;
	for (imask = 0; imask < imaskMac; imask++)
		{
		if (*pvoteMask >= voteMaj)
			*((int *) pchp) |= *pmask;
		pmask++;
		pvoteMask++;
		}

	pgrpfoo = rggrpfoo;
	pgrpvote = rggrpvote;
	pfoo = rgfoo;
	for (ifoo = 0; ifoo < cfoo; ifoo++)
		{
		*pfoo = GetMultiValueMaj(pgrpfoo, pgrpvote);
		pgrpfoo += ifooMac;
		pgrpvote += ifooMac;
		pfoo++;
		}
	pfoo = rgfoo;
	pchp->ftc = *pfoo++;
	pchp->hps = *pfoo++;
	pchp->hpsPos = *pfoo++;
	pchp->kul = *pfoo++;
	pchp->qpsSpace = *pfoo++;
	pchp->ico = *pfoo++;
}


/* M U L T I  V A L U E  V O T E  */
/* %%Function:MultValueVote %%Owner:davidbo */
MultiValueVote(rgfoo, rgvoteFoo, foo, vote)
int rgfoo[];
CHAR rgvoteFoo[];
int foo, vote;
{
	int ifoo, voteMin, ivoteMin;
	int *pfoo;
	CHAR  *pvoteFoo;

	voteMin = 256;
	Assert(vote < 256);
	pfoo = rgfoo;
	pvoteFoo = rgvoteFoo;
	for (ifoo = 0; ifoo < ifooMac; ifoo++)
		{
		/* foo's in the list so increase its vote */
		if (*pfoo == foo)
			{
			Assert(*pvoteFoo + vote < 256);
			*pvoteFoo += vote;
			return;
			}
		/* foo's not in the list but there's room, so put it in */
		else  if (*pvoteFoo == 0)
			{
			*pfoo = foo;
			*pvoteFoo = vote;
			return;
			}
		/* no match */
		else  if (*pvoteFoo < voteMin)
			{
			Assert(*pvoteFoo > 0);
			voteMin = *pvoteFoo;
			ivoteMin = ifoo;
			}
		pfoo++;
		pvoteFoo++;
		}

	/* foo didn't match and there's no room...replace the least voted
		for if foo has more votes! */
	if (vote > voteMin)
		{
		rgfoo[ivoteMin] = foo;
		rgvoteFoo[ivoteMin] = vote;
		}
}


/* G E T  M U L T I  V A L U E  V O T E  */
/* Given an rgFoo and an rgvoteFoo, return the foo that got the most votes */
/* %%Function:GetMultiValueMaj %%Owner:davidbo */
int GetMultiValueMaj(pfoo, pvoteFoo)
int *pfoo;
CHAR *pvoteFoo;
{
	int ifoo, fooMaj, voteMaj;

	fooMaj = *pfoo++;
	voteMaj = *pvoteFoo++;
	for (ifoo = 1; ifoo < ifooMac; ifoo++);
		{
		if (*pvoteFoo == 0)
			goto LRet;
		if (*pvoteFoo > voteMaj)
			{
			fooMaj = *pfoo;
			voteMaj = *pvoteFoo;
			}
		*pfoo++;
		*pvoteFoo++;
		}
LRet:
	return fooMaj;
}


/* F I R S T  C A  F O R  S E L S */
/* for non-block non-column selections, just return psels->ca. For block 
	selections, return ca describing first line of block. For column selections
	return ca that covers text of first selected cell of the first table row.*/
/* %%Function:FirstCaForSels %%Owner:davidbo */
FirstCaForSels(psels, pca)
struct SELS *psels;
struct CA *pca;
{
	struct CA caT;
	struct BKS bks;
	CP dcp;

	*pca = psels->ca;

	if (psels->fBlock && (!psels->fTable || psels->fColumn))
		{
		caT = *pca;
		InitBlockStat(&bks);
		while (FGetBlockLine(&caT.cpFirst, &dcp, &bks))
			{
			if (dcp)
				{
				pca->cpFirst = pca->cpLim = caT.cpFirst;
				pca->cpLim += dcp;
				break;
				}
			}
		}
}


/* %%Function:ResetDocStsh %%Owner:davidbo */
ResetDocStsh(doc, fMinimal)
int doc;
{
	struct STSH stsh;
	struct STTB **hsttbChpe, **hsttbPape;
	struct DOD *pdod;


	RecordStshForDoc(doc, &stsh, &hsttbChpe, &hsttbPape);
/* Asserts to demonstrate that heap guarantee made below is valid. */
	Assert(stsh.hsttbName != hNil);
	FreePhsttb(&stsh.hsttbName);
	Assert(stsh.hsttbChpx != hNil);
	FreePhsttb(&stsh.hsttbChpx);
	Assert(stsh.hsttbPapx != hNil);
	FreePhsttb(&stsh.hsttbPapx);
	Assert(stsh.hplestcp != hNil);
	FreePhpl(&stsh.hplestcp);
	Assert(hsttbChpe != hNil);
	FreePhsttb(&hsttbChpe);
	Assert(hsttbPape != hNil);
	FreePhsttb(&hsttbPape);

	if (!fMinimal)
		{
		stsh.cstcStd = PdodDoc(docNew)->stsh.cstcStd;
		if ((stsh.hsttbName
				= HCopyHeapBlock(PdodDoc(docNew)->stsh.hsttbName,strcSTTB)) == hNil)
			goto ErrRet;
		if ((stsh.hsttbChpx
				= HCopyHeapBlock(PdodDoc(docNew)->stsh.hsttbChpx,strcSTTB)) == hNil)
			goto ErrRet;
		if ((stsh.hsttbPapx
				= HCopyHeapBlock(PdodDoc(docNew)->stsh.hsttbPapx,strcSTTB)) == hNil)
			goto ErrRet;
		if ((stsh.hplestcp
				= HCopyHeapBlock(PdodDoc(docNew)->stsh.hplestcp, strcPL)) == hNil)
			goto ErrRet;
		if ((hsttbChpe = HsttbPropeFromStsh(&stsh, fTrue)) == hNil)
			goto ErrRet;
		if ((hsttbPape = HsttbPropeFromStsh(&stsh, fFalse)) == hNil)
			{
ErrRet:
/* rationale: even if copying the docNew style sheet fails because it is
	larger than the STSH we just freed, we should still be able to allocate
	the absolutely minimal STSH we create for docNew when the settings file
	doesn't exist. */
			FreePhsttb(&stsh.hsttbName);
			FreePhsttb(&stsh.hsttbChpx);
			FreePhsttb(&stsh.hsttbPapx);
			FreePhpl(&stsh.hplestcp);
			FreePhsttb(&hsttbChpe);
			FreePhsttb(&hsttbPape);
			fMinimal = fTrue;
			}
		}
	if (fMinimal)
		{
		StartGuaranteedHeap();
		AssertDo(FCreateStshNormalStc(&stsh, &hsttbChpe, &hsttbPape));
		EndGuarantee();
		}

	pdod = PdodDoc(doc);
	pdod->stsh = stsh;
	pdod->hsttbChpe = hsttbChpe;
	pdod->hsttbPape = hsttbPape;
	vdocPapLast = vdocFetch = docNil;
	InvalDoc(doc);
}


/* WARNING: a copy of this routine with hard-coded chpe, chpx, pape and
	papx values is in init2.c.  any changes to this function should be 
	reflected there.
*/

/* %%Function:FCreateStshNormalStc %%Owner:davidbo */
FCreateStshNormalStc(pstsh, phsttbChpe, phsttbPape)
struct STSH *pstsh;
struct STTB ***phsttbChpe, ***phsttbPape;
{
	struct STSH stsh;
	char st[10];
	struct ESTCP estcp;
	int rgwProp[cwPAP + 1];
	char pap[cbPAP];
	char papBase[cbPAP];
	int cch;

	/* clear so frees at failure will work */
	SetBytes (&stsh, 0, sizeof (struct STSH));
	*phsttbChpe =  *phsttbPape = hNil;

	st[0] = 0;
	if ((stsh.hsttbName = HsttbInit1(0, 1, 0, fTrue, fTrue)) == hNil ||
			IbstAddStToSttb(stsh.hsttbName, &st) == ibstNil)
		goto ErrRet;

	StandardChp(rgwProp);

	((struct CHP *) rgwProp)->fsFtc = fTrue;
	((struct CHP *) rgwProp)->fsHps = fTrue;
	cch = CchNonZeroPrefix(rgwProp, cbCHP);
	bltb(rgwProp, (char *) rgwProp + 1, cch);
	*(char *) rgwProp = cch;
	if ((stsh.hsttbChpx = HsttbInit1(0, 1, 0, fTrue, fTrue)) == hNil ||
			IbstAddStToSttb(stsh.hsttbChpx, rgwProp) == ibstNil)
		goto ErrRet;
	if ((*phsttbChpe = HsttbInit1(0, 1, 0, fTrue, fTrue)) == hNil ||
			IbstAddStToSttb(*phsttbChpe, rgwProp) == ibstNil)
		goto ErrRet;

	SetBytes(&papBase, 0, cbPAP);
	SetBytes(&pap, 0, cbPAP);
	/*  StandardPap(&pap); */
	cch = CbGenPapxFromPap(rgwProp, &pap, &papBase, fFalse);
	bltb(rgwProp, (char *) rgwProp + 1, cch);
	*(char *) rgwProp = cch;
	if ((stsh.hsttbPapx = HsttbInit1(0, 1, 0, fTrue, fTrue)) == hNil ||
			IbstAddStToSttb(stsh.hsttbPapx, rgwProp) == ibstNil)
		goto ErrRet;

	cch = CchNonZeroPrefix(&pap, cbPAP);
	bltb(&pap, (char *) rgwProp + 1, cch);
	*(char *) rgwProp = cch;
	if ((*phsttbPape = HsttbInit1(0, 1, 0, fTrue, fTrue)) == hNil ||
			IbstAddStToSttb(*phsttbPape, rgwProp) == ibstNil)
		goto ErrRet;

	estcp.stcBase = stcStdMin;
	estcp.stcNext = stcNormal;
	if ((stsh.hplestcp = HplInit(cbESTCP, 1)) == hNil)
		goto ErrRet;

	if (!FInsertInPl(stsh.hplestcp, 0, (char *) &estcp))
		goto ErrRet;


	bltb(&stsh, pstsh, sizeof (struct STSH));
	return fTrue;  /* OK */

ErrRet:
	/* FreePh/FreePhsttb tests for hNil before trying to free */
	FreePhsttb(&stsh.hsttbName);
	FreePhsttb(&stsh.hsttbChpx);
	FreePhsttb(phsttbChpe);
	FreePhsttb(&stsh.hsttbPapx);
	FreePhsttb(phsttbPape);
	FreePhpl(&stsh.hplestcp);
	return fFalse;
}


/* %%Function:MakeStshNonShrink %%Owner:davidbo */
MakeStshNonShrink(doc)
int doc;
{
	struct STSH stsh;
	struct STTB **hsttbChpe;
	struct STTB **hsttbPape;

	RecordStshForDoc(doc, &stsh, &hsttbChpe, &hsttbPape);
	(*stsh.hsttbName)->fNoShrink = fTrue;
	(*stsh.hsttbChpx)->fNoShrink = fTrue;
	(*stsh.hsttbPapx)->fNoShrink = fTrue;
	(*hsttbChpe)->fNoShrink = fTrue;
	(*hsttbPape)->fNoShrink = fTrue;
}


/* %%Function:CloseUpStsh %%Owner:davidbo */
CloseUpStsh(doc)
int doc;
{
	struct STSH stsh;
	struct STTB **hsttbChpe;
	struct STTB **hsttbPape;

	RecordStshForDoc(doc, &stsh, &hsttbChpe, &hsttbPape);

	CloseUpSttb(stsh.hsttbName);
	CloseUpSttb(stsh.hsttbChpx);
	CloseUpSttb(stsh.hsttbPapx);
	CloseUpSttb(hsttbChpe);
	CloseUpSttb(hsttbPape);
}


/* %%Function:RestoreStcpDefn %%Owner:davidbo */
RestoreStcpDefn(doc, stcp, pchp, ppap)
int doc;
int stcp;
struct CHP *pchp;
struct PAP *ppap;
{
	char stNil[1];
	struct STSH stsh;
	struct STTB **hsttbChpe;
	struct STTB **hsttbPape;

	RecordStshForDoc(doc, &stsh, &hsttbChpe, &hsttbPape);

	stNil[0] = 0;
	AssertDo(FChangeStInSttb(stsh.hsttbChpx, stcp, stNil));
	AssertDo(FChangeStInSttb(stsh.hsttbChpx, stcp, stNil));
	AssertDo(FChangeStInSttb(hsttbChpe, stcp, stNil));
	AssertDo(FChangeStInSttb(hsttbPape, stcp, stNil));

	AssertDo(FStorePropeForStcp(pchp, stcp, hsttbChpe, fTrue /* fChp */));
	AssertDo(FStorePropeForStcp(ppap, stcp, hsttbPape, fFalse /* PAP */));

	AssertDo(FGenChpxPapxNewBase(doc, stcp));
}


/* %%Function:RestoreStcpAndDependentDefn %%Owner:davidbo */
RestoreStcpAndDependentsDefn(doc, stcpRestore, pchp, ppap)
int doc;
int stcpRestore;
struct CHP *pchp;
struct PAP *ppap;
{
	char stNil[1];
	int stcp;
	int stcpMac;
	int stcBase;
	int docFtn, docHdr;
	struct DOD *pdod = PdodDoc(doc);
	struct STSH stsh;
	struct STTB **hsttbChpe;
	struct STTB **hsttbPape;
	struct PLC **hplcphe;

	RecordStshForDoc(doc, &stsh, &hsttbChpe, &hsttbPape);
/* declare that fStyleDirty in stylesheet entries is no longer trustworthy
	and invalidate all PHE entries. */
	pdod->fStyDirtyBroken = fTrue;
	if ((hplcphe = pdod->hplcphe) != hNil)
		InvalAllPheEntries(hplcphe);
	if ((docFtn = pdod->docFtn) != docNil &&
			(hplcphe = (PdodDoc(docFtn))->hplcphe) != hNil)
		InvalAllPheEntries(hplcphe);
	if ((docHdr = pdod->docHdr) != docNil &&
			(hplcphe = (PdodDoc(docHdr))->hplcphe) != hNil)
		InvalAllPheEntries(hplcphe);
	stcpMac = (*stsh.hsttbChpx)->ibstMac;
	for (stcp = 0; stcp < stcpMac; stcp++)
		/* examine each valid style in stsh */
		{
		if (FStcpStyleBaseChanged(&stsh, stcp, stcpRestore))
			{
			if (stcp < stsh.cstcStd)
				{
				stcBase = ((struct ESTCP *)PInPl(stsh.hplestcp, stcp))->stcBase;
				/* when a standard style is based on stcNormal
					and its properties are unchanged from the
					standard,  MapStcStandard will generate
					the correct property and does not require
					expanded properties, so continue. */
				if (stcBase == stcNormal && FStcpEntryIsNull(hsttbPape, stcp))
					continue;
				}

			stNil[0] = 0;
			AssertDo(FChangeStInSttb(hsttbChpe, stcp, stNil));
			AssertDo(FChangeStInSttb(hsttbPape, stcp, stNil));
			}
		}
/* by this point we have set to stNil any sttb entries that could have been
	created since we started the operation. Since we set sttb.fNoShrink we
	should be able to restore the stsh to its original state. */
	StartGuaranteedHeap();
	RestoreStcpDefn(doc, stcpRestore, pchp, ppap);
	AssertDo(FRecalcAllDependentChpePape(&stsh, hsttbChpe, hsttbPape, stcpRestore));
	EndGuarantee();
}


extern struct SAB vsab;

/* C H E C K  S C R A P  S T S H */
/* to be called when stsh of doc is redefined */
/* %%Function:CheckScrapStsh %%Owner:davidbo */
CheckScrapStsh(doc)
{
	Win(Assert(vsab.docStsh == doc)); /* avoid swapping in */
	if (vsab.docStsh != doc) return;
	ResetDocStsh(docScrap, fTrue /* allocate minimal STSH */);
	Win( ResetDocFonts(docScrap) );
	CopyStylesFonts(doc, docScrap, cp0, CpMacDoc(docScrap));
	vsab.docStsh = docNil;
}


/* R E S T O R E  S T C  F R O M  B A C K U P  */
/* %%Function:RestoreStcFromBackup %%Owner:davidbo */
RestoreStcFromBackup()
{
	int stcp, stcpBackup;
	struct STSH stsh;
	struct STTB **hsttbChpe, **hsttbPape;
	struct CHP chp, chpBefore;
	struct PAP pap, papBefore;
	char stName[WinMac(cchMaxStyle, 256)];

	if (vstcStyle == -1 || vstcBackup == -1)
		return;
	RecordStshForDoc(vdocStsh, &stsh, &hsttbChpe, &hsttbPape);

#ifdef WIN
	/* if no change, don't bother with the rest of this gunk! */
	if (FStylesEqual(vstcBackup, &stsh, hsttbChpe, hsttbPape))
		return;
#endif

	MapStc(PdodDoc(vdocStsh), vstcStyle, &chpBefore, &papBefore);
	stcpBackup = StcpFromStc(vstcBackup, stsh.cstcStd);
	if (*(HpstFromSttb(hsttbPape, stcpBackup)) == 255)
		{
		Assert(stcStdMin < vstcStyle && vstcStyle <= 255);
		MapStc(PdodDoc(vdocStsh), 0, &chp, &pap);
		MapStcStandard(vstcStyle, &chp, &pap);
		}
	else
		{
		RetrievePropeForStcp(&chp, stcpBackup, hsttbChpe, fTrue);
		RetrievePropeForStcp(&pap, stcpBackup, hsttbPape, fFalse);
		}

	stcp = StcpFromStc(vstcStyle, stsh.cstcStd);
	if (!FCopyStyleWithinStsh(vdocStsh, vstcBackup, vstcStyle))
		goto LDrastic;

#ifdef MAC
	vfStyleDirty = fFalse;
	SetEnableTmc(tmcDefine, fFalse);
#endif

	if (!FGenChpxPapxNewBase(vdocStsh, stcp))
		goto LDrastic;
	if (!FRecalcAllDependentChpePape(&stsh, hsttbChpe, hsttbPape, stcp))
		{
LDrastic:
		RestoreStcpAndDependentsDefn(vdocStsh, stcp, &chp, &pap);
		GetStFromSttb(stsh.hsttbName, stcpBackup, stName);
		FChangeStInSttb(stsh.hsttbName, stcp, stName);
		}
	if (!FMatchAbs(vdocStsh, &pap, &papBefore))
		InvalPageView(vdocStsh);
	InvalDoc(vdocStsh);
	vdocFetch = caPara.doc = docNil;
#ifdef MAC
	vfStyleNameDirty = fFalse;
	vfStyleDefnDirty = fFalse;
#endif
}


/* %%Function:MakeStcpEntryNull %%Owner:davidbo */
MakeStcpEntryNull(stcp, pstsh, hsttbChpe, hsttbPape)
int stcp;
struct STSH *pstsh;
struct STTB **hsttbChpe;
struct STTB **hsttbPape;
{
	char stNil[2];

	stNil[0] = 255;
/* since we are either shrinking the sttb or at worst leaving it the same
	size, the FChangeSt... calls should not fail. */
	AssertDo(FChangeStInSttb(pstsh->hsttbName, stcp, stNil));
	AssertDo(FChangeStInSttb(pstsh->hsttbChpx, stcp, stNil));
	AssertDo(FChangeStInSttb(pstsh->hsttbPapx, stcp, stNil));
	AssertDo(FChangeStInSttb(hsttbChpe, stcp, stNil));
	AssertDo(FChangeStInSttb(hsttbPape, stcp, stNil));
}


/* %%Function:EraseStcRetargetMappings %%Owner:davidbo */
EraseStcRetargetMappings(mpstcSrcstcDest, rgstcRetarget, pistcRetargetMac,
pstsh, hsttbChpe, hsttbPape)
char *mpstcSrcstcDest;
char *rgstcRetarget;
int *pistcRetargetMac;
struct STSH *pstsh;
struct STTB **hsttbChpe;
struct STTB **hsttbPape;
{
	int istc;
	char *pstc;
	int istcRetargetMac = *pistcRetargetMac;

	for (istc = 0; istc < istcRetargetMac; istc++)
		{
		pstc = &mpstcSrcstcDest[rgstcRetarget[istc]];
		if (*pstc != stcNormal)
			{
			MakeStcpEntryNull(StcpFromStc(*pstc, pstsh->cstcStd), 
					pstsh, hsttbChpe, hsttbPape);
			}
		*pstc = stcNormal;
		}
	*pistcRetargetMac = 0;
}


/* %%Function:FGtStyleNamStc %%Owner:davidbo */
FGtStyleNamStc(stc1, stc2)
int stc1, stc2;
{
	/* Assumes vdocStsh has been set -- see FCopyStyleToDestStsh */
	int cstcStd = PdodDoc(vdocStsh)->stsh.cstcStd;
	struct STTB **hsttbName = PdodDoc(vdocStsh)->stsh.hsttbName;
	char stName1[cchMaxStyle];
	char stName2[cchMaxStyle];

	GenStyleNameForStcp(stName1, hsttbName, cstcStd, StcpFromStc(stc1,cstcStd));
	GenStyleNameForStcp(stName2, hsttbName, cstcStd, StcpFromStc(stc2,cstcStd));
	return(WCompSt(stName1, stName2) > 0 ? TRUE : FALSE);
}


/*
*  Binary-search the sttb returning true if the string is found.
*  sttb isn't sorted but rgstc is an array of sorted indices.
*  If string not found, pistc is where it should go in rgstc.
*/
/* %%Function:FSearchStshSttbRgstc %%Owner:davidbo */
FSearchStshSttbRgstc(hsttb, cstcStd, st, rgstc, pistc, istcLim)
struct STTB **hsttb;
char *st, rgstc[];
int *pistc, istcLim, cstcStd;
{
	int istcMin = 0;
	int istc = istcMin;
	int wCompGuess;
	char stComp[cchMaxStyle];

	while (istcMin < istcLim)
		{
		istc = (istcMin + istcLim) >> 1;
		GenStyleNameForStcp(stComp, hsttb, cstcStd, StcpFromStc(rgstc[istc], cstcStd));
		if ((wCompGuess = WCompSt(st, stComp)) == 0)
			return *pistc = istc, fTrue; /* found: return index */
		else  if (wCompGuess < 0)
			istcLim = istc;
		else
			istcMin = ++istc;
		}
	return *pistc = istc, fFalse; /* not found: return insert point */
}


/*
*  returns true if completely successful.  If it returns false, pidError
*  is set to -1 for OOM, and -2 for StshFull.
*/
/* %%Function:FCopyStyleToDestStsh %%Owner:davidbo */
FCopyStyleToDestStsh(docSrc, pstshSrc, hsttbChpeSrc, hsttbPapeSrc,
docDest, pstshDest, hsttbChpeDest, hsttbPapeDest, rgstcCopy,
istcCopyMac, rgb, fCopyAllProps, fRTFMerge, fAbortIncomplete, pidError)
int docSrc;  /* for font mapping. No mapping if docNil */
struct STSH *pstshSrc;
struct STTB **hsttbChpeSrc;
struct STTB **hsttbPapeSrc;
int docDest;
struct STSH *pstshDest;
struct STTB **hsttbChpeDest;
struct STTB **hsttbPapeDest;
char rgstcCopy[];
int istcCopyMac;
char rgb[];
int fCopyAllProps;
int fRTFMerge;
int fAbortIncomplete;
int *pidError;
{
	struct STSH stshSrc;
	struct STSH stshDest;
	int stcMatchDest;
	int fMatchStd, fCreate, fNewSortEntry, fRet;
	int stcSrc;
	int stcpSrc;
	int stcpSrcMac;
	int stcpDest;
	int stcpDestMac;
	int stcp;
	int stc;
	int docBackup;
	int stcBackup;
	int i, iMac;
	int istcp;
	int istc;
	int istcSort, istcSortMac;
	int istcDestMac;
	int istcT;
	int istcRetargetMac;
	int stcPermuteLast;
	char *mpstcSrcstcDest;
	char *stDest, *pb;
	int  rgstc[2];
	char rgbProps[cchMaxSz];
	char stName[cchMaxStyle];
	char rgstcRetarget[cstcMax];
	char rgstcSort[cstcMax];

	*pidError = 0;
	fRet = fTrue;
	if (vmerr.fMemFail)
		{
		*pidError = -1;
		return fFalse;
		}

	stshSrc = *pstshSrc;
	stshDest = *pstshDest;

	istcRetargetMac = 0;
	mpstcSrcstcDest = rgb + 2;
	for (stc = 0; stc < cstcMax; stc++)
		mpstcSrcstcDest[stc] = stc;

	/* we will copy the entire stshSrc when istcCopyMac == 0 */
	stcpSrcMac = (*stshSrc.hplestcp)->stcpMac;
	iMac = (istcCopyMac == 0) ? stcpSrcMac : istcCopyMac;

	/*
	*  Calls to FMatchDefinedAndStdStyles reference vstcBackup.  This backup
	*  is in stshSrc, but we're matching in stshDest.  Make vstcBackup
	*  something innocuous and reset at the end of the routine.
	*/
	stcBackup = vstcBackup;
	vstcBackup = stcStdMin;

	/*  Build rgstcSort -- array of style codes sorted by style name */
	istcDestMac = (*stshDest.hplestcp)->stcpMac;
	istcSortMac = 0;
	pb = rgstcSort;
	stc = (cstcMax - stshDest.cstcStd) & 0xff;
	for (istcSort = 0; istcSort < istcDestMac; istcSort++)
		{
		Assert (stc != vstcBackup);
		if (!FStcpEntryIsNull(stshDest.hsttbName, StcpFromStc(stc, stshDest.cstcStd)))
			{
			*pb++ = stc;
			istcSortMac++;
			}
		stc++;
		}
	Assert(istcSortMac <= cstcMax);
	docBackup = vdocStsh;
	vdocStsh = docDest;
	SortRgb(rgstcSort, istcSortMac, FGtStyleNamStc);
	vdocStsh = docBackup;

	for (i = 0; i < iMac; i++)
		{
		if (istcCopyMac == 0)
			{
			stcpSrc = i;
			stcSrc = StcFromStcp(stcpSrc, stshSrc.cstcStd);
			if (FStcpEntryIsNull(stshSrc.hsttbName,stcpSrc))
				continue;
			}
		else
			{
			stcSrc = rgstcCopy[i];
			stcpSrc = StcpFromStc(stcSrc, stshSrc.cstcStd);
			if (stcSrc < stcStdMin && (stcpSrc >= stcpSrcMac ||
					FStcpEntryIsNull(stshSrc.hsttbName, stcpSrc)))
				{
				mpstcSrcstcDest[stcSrc] = stcNormal;
				continue;
				}
			}

#ifdef DSTYFTC
		CommSzNum(SzShared("stcSrc: "), stcSrc);
#endif

		fCreate = fNewSortEntry = fFalse;
		if (!fRTFMerge)
			{
			GenStyleNameForStcp(stName, stshSrc.hsttbName,
					stshSrc.cstcStd, stcpSrc);
			}
		else
			{
			GetStFromSttb(stshSrc.hsttbName, stcpSrc, stName);
			if (stName[0] == 0)
				CopyCsSt(stNormal, stName);
			Assert(FValidStyleName(stName));
			}

		if (vmerr.fMemFail)
			goto LRestartMemFail;

#ifdef DSTYFTC
		CommSzSt(SzShared("stName: "), stName);
#endif

		/* Binary search stshDest.hsttbName using rgstcSort */
		if (FSearchStshSttbRgstc(stshDest.hsttbName, stshDest.cstcStd, stName, rgstcSort, &istcSort, istcSortMac))
			{
			stcMatchDest = rgstcSort[istcSort];
			stcpDest = StcpFromStc(stcMatchDest, stshDest.cstcStd);
#ifdef DSTYFTC
			CommSzNum(SzShared("match, stcDest: "), stcMatchDest);
#endif
			if (fCopyAllProps)
				goto LCopyAllProps;
			}
		else
			{
			if (fRTFMerge)
				{
				/* standard stcs aren't the same # between OPUS and MAC or
					PCWORD, so do this to generate correct stc in docDest. */
				if (FMatchStdStyles(stName, &stcMatchDest))
					goto LEnsureStd;
				goto LCreateNew;
				}
			else  if (stcSrc > stcStdMin)
				{
				/* std style not defined in stshDest */
				stcMatchDest = stcSrc;

LEnsureStd:
				if (vmerr.fMemFail || !FEnsureStcDefined(docDest, stcMatchDest,
						&stshDest.cstcStd))
					goto LRestartMemFail;
				stcpDest = StcpFromStc(stcMatchDest, stshDest.cstcStd);
				}
			else
				{
LCreateNew:
				stcpDest = StcpCreateNewStyle(docDest, fFalse /*fUseReserve*/);
				fCreate = fTrue;
				if (stcpDest == stcpNil)
					{
LRestartMemFail:
					*pidError = vmerr.fMemFail ? -1 : -2;
					fRet = fFalse;
					if (fAbortIncomplete)
						{
						vstcBackup = stcBackup;
						goto LRet;
						}
					if (fCreate && stcpDest != stcpNil)
						{
						MakeStcpEntryNull(stcpDest, &stshDest,
								hsttbChpeDest, hsttbPapeDest);
						}
					if (vmerr.fMemFail && istcRetargetMac > 0)
						{
						EraseStcRetargetMappings(mpstcSrcstcDest,rgstcRetarget,
								&istcRetargetMac, &stshDest,
								hsttbChpeDest, hsttbPapeDest);
						}
					if (fNewSortEntry)
						{
						/* get rid of bogus entry in sort table */
						Assert(istcSort < istcSortMac);
						if (istcSort < istcSortMac - 1)
							{
							pb = &rgstcSort[istcSort];
							bltb(pb+1, pb, istcSortMac - istcSort - 1);
							}
						istcSortMac--;
						}
					stcpDest = stshDest.cstcStd;
					stcMatchDest = stcNormal;
					goto LSetPermuteMap;
					}
				stcMatchDest = StcFromStcp(stcpDest, stshDest.cstcStd);
				}

#ifdef DSTYFTC
			CommSzNum(SzShared("no match, stcDest: "), stcMatchDest);
#endif
			/* Maintain sortedness of rgstcSort! */
			pb = &rgstcSort[istcSort];
			if (istcSort == istcSortMac)
				*pb = stcMatchDest;
			else
				{
				bltb(pb, pb+1, istcSortMac - istcSort);
				rgstcSort[istcSort] = stcMatchDest;
				}
			istcSortMac++;
			Assert (istcSortMac <= cstcMax);
			fNewSortEntry = fTrue;

			/* no need to store name of standard style */
			if (stcMatchDest > stcStdMin)
				stName[0] = 0;
			if (vmerr.fMemFail ||
					!FChangeStInSttb(stshDest.hsttbName, stcpDest, stName))
				goto LRestartMemFail;

LCopyAllProps:
			/*
			*  Only std styles should have a length of 255.  If the style is
			*  std, we can copy it without interpretation.
			*/

			if (stcpSrc < stcpSrcMac && !FStcpEntryIsNull(hsttbChpeSrc,stcpSrc))
				{
				int ftcSrc, ftcDest;

				GetStFromSttb(hsttbChpeSrc, stcpSrc, rgbProps);

				/* Map ftc if required. Ftc, if any, is in word 2 */

				if (docSrc != docNil && rgbProps[0] >= 4)
					{
					ftcSrc = ((struct CHP *)(rgbProps + 1))->ftc;
					if ( ftcSrc != ftcDefault )
						{
						/* HM */
						ftcDest = FtcMapDocMotherFtc(DocMother(docSrc),
								DocMother(docDest), ftcSrc);
						if (ftcDest != ftcSrc)
							{
							((struct CHP *)(rgbProps + 1))->ftc = ftcDest;
#ifdef DSTYFTC
							CommSzNum(SzShared("fcopystsh... map ftc: ftcSrc: "), ftcSrc);
							CommSzNum(SzShared("fcopystsh... map ftc: ftcDest: "), ftcDest);
#endif
							}
						}
					}
				}
			else
				{
				Assert(stcSrc > stcStdMin);
				rgbProps[0] = 255;
				}
			if (vmerr.fMemFail ||
					!FChangeStInSttb(hsttbChpeDest, stcpDest, rgbProps))
				goto LRestartMemFail;

			if (stcpSrc < stcpSrcMac && !FStcpEntryIsNull(hsttbPapeSrc,stcpSrc))
				{
				RetrievePropeForStcp(rgbProps, stcpSrc, hsttbPapeSrc, FALSE);
				((struct PAP *) rgbProps)->stc = stcMatchDest;
				if (vmerr.fMemFail ||
						!FStorePropeForStcp(rgbProps,stcpDest,hsttbPapeDest, FALSE))
					goto LRestartMemFail;
				}
			else
				{
				Assert(stcSrc > stcStdMin);
				rgbProps[0] = 255;
				if (vmerr.fMemFail ||
						!FChangeStInSttb(hsttbPapeDest, stcpDest, rgbProps))
					goto LRestartMemFail;
				}
			rgstcRetarget[istcRetargetMac++] = stcSrc;
			}
LSetPermuteMap:
		mpstcSrcstcDest[stcSrc] = stcMatchDest;
		}

	for (istc = 0; istc < istcRetargetMac; istc++)
		{
		stcSrc = rgstcRetarget[istc];
		stcpSrc = StcpFromStc(stcSrc, stshSrc.cstcStd);

		GetStcBaseNextForStcp(&stshSrc, stcpSrc, &rgstc[0], &rgstc[1]);
		for (istcT = 0; istcT < 2; istcT++)
			{
			if (rgstc[istcT] != stcStdMin)
				{
				/* get full name of style at stcBase */
				stcp = StcpFromStc(rgstc[istcT], stshSrc.cstcStd);
				if (!fRTFMerge)
					GenStyleNameForStcp(stName, stshSrc.hsttbName, stshSrc.cstcStd, stcp);
				else
					{
					GetStFromSttb(stshSrc.hsttbName, stcp, stName);
					if (stName[0] == 0)
						CopyCsSt(stNormal, stName);
					}

				/*
				*  If there is an equivalent stc in the docDest stsh, store it.
				*  Otherwise, store stcNormal.
				*/
				if (!FSearchStshSttbRgstc(stshDest.hsttbName, stshDest.cstcStd, stName, rgstcSort, &istcSort,
						 istcSortMac))
					rgstc[istcT] = stcNormal;
				else
					rgstc[istcT] = rgstcSort[istcSort];
				if (!FEnsureStcDefined(docDest, rgstc[istcT],&stshDest.cstcStd))
					{
					if (fAbortIncomplete)
						{
						vstcBackup = stcBackup;
						goto LRet;
						}
					EraseStcRetargetMappings(mpstcSrcstcDest, rgstcRetarget, &istcRetargetMac, &stshDest,
							 hsttbChpeDest, hsttbPapeDest);
					goto LBreakBasedNext;
					}
				}
			}

		stcpDest = StcpFromStc(mpstcSrcstcDest[stcSrc], stshDest.cstcStd);
		SetStcBaseNextForStcp(&stshDest, stcpDest, rgstc[0], rgstc[1]);
		}

LBreakBasedNext:
	stcpDestMac = (*stshDest.hplestcp)->stcpMac;
	/* we should only have to generate CHPXs and PAPXs for the
		new styles that were added. */
	for (istc = 0; istc < istcRetargetMac; istc++)
		{
		if (vmerr.fMemFail || !FGenChpxPapxNewBase(docDest,
				StcpFromStc(mpstcSrcstcDest[rgstcRetarget[istc]],stshDest.cstcStd)))
			{
			if (fAbortIncomplete)
				{
				vstcBackup = stcBackup;
				goto LRet;
				}
			EraseStcRetargetMappings(mpstcSrcstcDest, rgstcRetarget, &istcRetargetMac, 
					&stshDest, hsttbChpeDest, hsttbPapeDest);
			break;
			}
		}


	if (!fRTFMerge)
		{
		for (stcPermuteLast = 255; stcPermuteLast > 0; stcPermuteLast--)
			{
			if (mpstcSrcstcDest[stcPermuteLast] != stcPermuteLast)
				break;
			}
		rgb[0] = 0;
		if (stcPermuteLast > 0)
			{
			rgb[0] = sprmPStcPermute;
			rgb[1] = stcPermuteLast;
			bltb(mpstcSrcstcDest + 1, rgb+2, stcPermuteLast);
			}
		}

	pstshDest->cstcStd = stshDest.cstcStd;
	vstcBackup = stcBackup;
	(PdodDoc(docDest))->fStshDirty = fTrue;

LRet:
	if (vmerr.fMemFail)
		{
		fRet = fFalse;
		*pidError = -1;
		SetErrorMat(matNil);
		}
	return fRet;
}


/*
*  Returns fTrue if stName can be used as a style name.
*/
/* %%Function:FValidStyleName %%Owner:davidbo */
FValidStyleName(stName)
char *stName;
{
	int cch, cchMac;
	char *pch;
	char st[cchMaxStyle + 1];

	/* certainly not valid if no name! */
	if (*stName == 0)
		return fFalse;

	/* not valid if name is too long */
	if (*stName >= cchMaxStyle)
		return fFalse;

	/* not valid if name is nothing but whitespace...check in temp so
		we don't nuke leading white space of valid style. */
	CopySt(stName, st);
	if ((cchMac = CchStripString(&st[1], st[0])) == 0)
		return fFalse;

	return fTrue;
}


