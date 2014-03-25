/* F E T C H 2 . C */
#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "doc.h"
#include "file.h"
#include "props.h"
#include "border.h"
#include "fkp.h"
#include "ch.h"
#include "format.h"
#include "layout.h"
#include "prm.h"
#include "sel.h"
#include "disp.h"
#include "inter.h"
#include "cmd.h"
#include "debug.h"
#include "screen.h"
#include "ruler.h"
#include "pic.h"
#include "error.h"
#ifdef MAC
#include "mac.h"
#include "toolbox.h"
#endif

#ifdef WIN
#include "sdmdefs.h"
#include "sdmver.h"
#include "sdm.h"
#include "sdmtmpl.h"
#include "sdmparse.h"

#include "char.hs"

#ifdef PROTOTYPE
#include "fetch1.cpt"
#endif /* PROTOTYPE */
#endif  /* WIN */

/* E X T E R N A L S */

extern BOOL             vfEndFetch;
extern struct DOD       **mpdochdod[];
extern struct STTB      **vhsttbFont;
extern struct CHP       vchpFetch;
extern struct CHP       vchpStc;
extern struct FKPD      vfkpdChp;
extern struct CA        caPara;
extern char HUGE        *vhpchFetch;
extern int              vdocFetch;
extern CP               vcpFetch;
extern int              vccpFetch;
extern int              vdocTemp;

extern char             rgchInsert[];
extern int              ichInsert;
extern struct SELS      rgselsInsert[];

extern struct FCB       **mpfnhfcb[];

extern struct CA        caParaL;
extern struct CA        caTap;

extern struct FKPD      vfkpdPap;

extern struct CA        caSect;
extern int              vised;
extern struct CA        caPage;
extern int              vipgd;
extern CP               vcpFirstLayout;
extern int              vifnd;
extern struct CHP       vchpStc;
extern struct PAP       vpapFetch;
extern struct SEP       vsepFetch;
extern int              prmFetch;
extern struct LLC       vllc;
extern struct TAP       vtapStc;
extern struct TAP       vtapFetch;
extern struct TCC       vtcc;
extern CP               vmpitccp[];
extern int              vitcMic;

extern char             (**vhgrpchr)[];
extern int              vbchrMax;
extern int              vbchrMac;

extern struct FLI       vfli;
extern struct FTI       vfti;
extern struct PREF      vpref;
extern struct SEL       selCur;

extern struct FLS       vfls;
extern struct MERR      vmerr;
extern struct DBS       vdbs;

extern int              wwCur;
extern struct WWD       **hwwdCur;

extern struct ITR       vitr;

extern BOOL             vfShowAllStd;
extern struct SCI       vsci;

extern int              vjnRare;
extern int              vjcRare;
extern int              vlm;
extern BOOL		vfFliMunged; /* returned from CachePage */

extern struct TCXS		vtcxs;
#ifdef WIN
extern int              vflm;
extern struct CHP	vchpGraySelCur;
extern struct PAP	vpapSelCur;
extern struct PAP	vpapGraySelCur;
#endif

#ifdef MAC
extern int		vftcHelvetica;
#endif

#ifdef JR
extern int              vfJRProps;
#endif /* JR */


/* standard style names */
csconst char rgstStd[][] =
{
	StKey("Normal",NormalFETCH1C),
#ifdef WIN
			StKey("Normal Indent",NormalIndent),
#endif
			StKey("heading 1",Head1),
			StKey("heading 2",Head2),
			StKey("heading 3",Head3),
			StKey("heading 4",Head4),
			StKey("heading 5",Head5),
			StKey("heading 6",Head6),
			StKey("heading 7",Head7),
			StKey("heading 8",Head8),
			StKey("heading 9",Head9),
			StKey("footnote text",FnText),
			StKey("footnote reference",FnRef),
			StKey("header",Header),
			StKey("footer",Footer),
#ifdef MAC
			StKey("page number", PageNumFETCH1_C),
#else /* WIN */
			StKey("index heading",IndexHeading),
#endif
			StKey("line number",LineNum),
			StKey("index 1",Ind1),
			StKey("index 2",Ind2),
			StKey("index 3",Ind3),
			StKey("index 4",Ind4),
			StKey("index 5",Ind5),
			StKey("index 6",Ind6),
			StKey("index 7",Ind7),
			StKey("toc 1",TOC1),
			StKey("toc 2",TOC2),
			StKey("toc 3",TOC3),
			StKey("toc 4",TOC4),
			StKey("toc 5",TOC5),
			StKey("toc 6",TOC6),
			StKey("toc 7",TOC7),
			StKey("toc 8",TOC8),
#ifdef MAC
			StKey("toc 9",TOC9),
			StKey("PostScript",PS),
			StKey("Normal Indent", NormIndent)
#else /* WIN */
			StKey("annotation text",AnnotationText),
			StKey("annotation reference",AnnotationReference)
#endif
};


#ifdef MAC	/* No good for Opus; assumes printer and screen fonts are
realized exactly as requested */		   		
/* D X T  L E F T  O F  T A B  S T O P */
/* return sum of xt's of chars in the interval (ichTab, ichLim) that are
to be the left of a decimal aligned tab stop.
Align with first non digit non comma character after the first digit.
*/
/* %%Function:DxtLeftOfTabStop %%Owner:NOTUSED */
EXPORT int DxtLeftOfTabStop(ichTab, ichLim, pftiDxt, pdxp)
int ichTab, ichLim; 
struct FTI *pftiDxt; 
int *pdxp;
{
	int ich = ichTab + 1;
	int dxp = 0;
	int ch;
	BOOL fDigitSeen = fFalse;

	/* decimal point terminates immediately */
	for (; ich < ichLim && (ch = vfli.rgch[ich]) != vitr.chDecimal; ich++)
		{
		if (FDigit(ch))
			fDigitSeen = fTrue;
		else  if (ch != vitr.chThousand && fDigitSeen)
			break;
		dxp += vfli.rgdxp[ich];
		}
	*pdxp = dxp;
	return((vfli.fPrint) ? dxp : NMultDiv(dxp, vfli.dxuInch, vfli.dxsInch));
}


#endif	/* MAC */

#ifdef WIN
/* %%Function:DxtLeftOfTabStop %%Owner:davidlu */
NATIVE int DxtLeftOfTabStop(ichTab, ich, pftiDxt, pdxp)
int ichTab, ich; 
struct FTI *pftiDxt; 
int *pdxp;
{
	int ichT = ichTab + 1;
	int dxtT = 0;
	int dxpT = 0;
	int chT;
	BOOL fDigitSeen = fFalse;

	/* decimal point terminates immediately */
	if (vfli.rgch[ichT] != vitr.chDecimal)
		{
		for (; ichT < ich; ichT++)
			{
			chT = vfli.rgch[ichT];
			if (FDigit(chT))
				fDigitSeen = fTrue;
			else  if (chT != vitr.chThousand && fDigitSeen) break;
			dxtT += DxpFromCh(chT, pftiDxt);
			dxpT += DxpFromCh(chT, &vfti);
			}
		}
	*pdxp = dxpT;
	return dxtT;
}


#endif	/* WIN */

#ifdef MAC /* WIN - moved to RES.C */
/* D I  D I R T Y  D O C */
/* checks if doc is dirty, including complex cases */
/* %%Function:DiDirtyDoc %%Owner:NOTUSED */
int DiDirtyDoc(doc)
{
	struct DOD *pdod = PdodDoc(doc);
	int docHdr;
	int di = 0;

	if (pdod->fDirty || ((pdod->fFtn || pdod->fAtn) && PdodDoc(pdod->doc)->fDirty))
		di += dimDoc;
	if (pdod->fStshDirty)
		di += dimStsh;
	if (!pdod->fShort && pdod->docFtn && PdodDoc(pdod->docFtn)->fDirty)
		di += dimFtn;
	if (!pdod->fShort && (docHdr = pdod->docHdr) != docNil)
		{
		if (PdodDoc(docHdr)->fDirty)
			{
			di += dimHdr;
			}
		else  if (PdodDoc(docHdr)->docHdr != docNil)
			/* check docHdrDisps also - works for mac too */
			{
			int docHdrDisp = docNil;
			int wwHdr;
			while ((wwHdr = WwHdrFromDocMom(doc, &docHdrDisp)) != wwNil)
				{
				if (PdodDoc(docHdrDisp)->fDirty)
					{
					di += dimHdr;
					break;
					}
				}
			}
		}
#ifdef WIN
	if (pdod->fDot && pdod->docGlsy && PdodDoc(pdod->docGlsy)->fDirty)
		di += dimGlsy;
	if (!pdod->fShort && pdod->docAtn && PdodDoc(pdod->docAtn)->fDirty)
		di += dimAtn;
#endif /* WIN */

/*      if (pdod->fRepag)
		di += dimRepag;*/
	return di;
}


#endif /* MAC */

/* G E N  S T Y L E  N A M E ... */
/* %%Function:GenStyleNameForStcp %%Owner:davidlu */
NATIVE GenStyleNameForStcp(st, hsttbName, cstcStd, stcp)
char st[];
struct STTB **hsttbName;
int cstcStd;
int stcp;
{
	int stc = StcFromStcp(stcp, cstcStd);
	int ist;
	int cchDiff;
	int cbStName;
	int cbSt;
	int cbAdjust;
	int ibstMac;
	int fDefined;
	int fNullName;
	char HUGE *hpstName;

	st[0] = 0;
	if (stc == stcStdMin)
		return;
	fDefined = (stcp >= 0 && stcp <  (ibstMac = (*hsttbName)->ibstMac));
	/* map bogus styles to Normal...just like MapStc */
	if (stcp >= ibstMac && stc < stcStdMin)
		{
		stc = stcNormal;
		stcp = cstcStd;
		fDefined = fTrue;
		}
	fNullName = (!fDefined || (*(hpstName=HpstFromSttb(hsttbName,stcp)) == 255));

	if (fDefined &&
#ifdef WIN
			(stc != stcNormIndent) &&
			!(stc >= stcLev3 && stc <= stcLev1) &&
#endif
			!(vfShowAllStd && stcp <= cstcStd) && fNullName)
		return;
	if (stcp <= cstcStd || stcp >= ibstMac)
		{
		/* convert stcp to stc, stc to ist */
		ist = (256 - ((stcp - cstcStd) & 255)) & 255;
		GetStdStyleSt(ist, st);
		}
#ifdef MAC	/* DAVIDBO sez: Not needed for WIN, no synonym feature */
	if (!fNullName)
		{
		cbStName = hpstName[0];
		cbSt = st[0];
/* wish to ensure that there is room for exactly one 255 byte name or else
	two or more names whose total length is 254 plus one comma. */
		cbAdjust = (cbSt != 0) ? 1 : 0;
		if ((cchDiff = cbSt + cbStName - 255 + cbAdjust) > 0)
			cbStName -= cchDiff;
		if (cbStName > 0)
			{
			if (cbAdjust)
				st[cbSt+1] = ',';
			bltbh(hpstName+1,(char HUGE *)st + cbSt + 1 + cbAdjust, cbStName);
			st[0] += cbStName + cbAdjust;
			}
		}
#else
	else  if (!fNullName)
		bltbh(hpstName, st, hpstName[0] + 1);
#endif
}


/* G E T  S T D  S T Y L E  S T  - gets std style name into st */
/* %%Function:GetStdStyleSt %%Owner:davidlu */
GetStdStyleSt(ist, st)
int ist;
char *st;
{
	char far *qst = rgstStd[ist];
	CopyCsSt(qst, st);
}


#ifdef WIN

/* %%Function:FGetCharState %%Owner:bobz */
FGetCharState(fAll, fAbortOk)
BOOL fAll;
BOOL fAbortOk;   /* if true, this routine is interruptible */
{
#define cparaMax 50

	CP cpLim;
	CP cpStartRun;
	CP cpParaPrev = cpNil;
	int cpara = 0;
	struct CHP chp, chpGray, chpT;
	int doc;
	CP dcp;
	struct BKS bks;
	BOOL fFirstRun = fTrue;

	/* notes on what is happening here:
		Whenever a selection is made, selCur.chp is set to the properties of
		the first run in the selection, and selCur.fUpdateChpGray is set to true,
		indicating that chpGray does not necessarily reflect the state of the
		entire selection. This routine scans the selection, and if any fields
		differ, the corresponding field in vchpGraySelCur is set to non-zero.
		At the end, selCur.fUpdateChpGray is set false, indicating that the chp is
		up to date. FSetRgwDiff uses a fairly clever mechanism to set the
		appropriate different field, but can't tell when all fields are gray, so
		it always continues to the limit of the selection or cparaMax paras.
	
		Changed to do a fixed # of paragraphs, not runs
	
		As in Write, if we stop before the entire selection is checked, we set
		everything to gray.
	
	*/

	   /* bag out if we got here in a state we can't handle bz */
	if (wwCur != selCur.ww    /* cpLimNoSpaces assumes this */
	   || PselActive() != &selCur)  /* probably in trouble otherwise bz */
		   {
		   Assert (fFalse);	/* if we got here, warn so we can stopp it */
		   return fFalse;
		   }
	/* here we assume that a chp is an even # of bytes */
	Assert (!(sizeof(struct CHP) % sizeof(int)));

	/* gray out everything if no child window up */

	/* set values besides selCur.chp into selCur */

	if (hwwdCur == hNil)
		{
		/* init gray chp to all gray */
		SetWords(&vchpGraySelCur, 0xFFFF, cwCHP);
		selCur.fUpdateChpGray = fFalse;
		StandardChp(&selCur.chp);
		return(fTrue);  /* leave fUpdate bit on */
		}

	doc = selCur.doc;

	/*  assure that selCur.chp reflects the current selection */
	/* note that selCur.chp is only valid now if !selCur.fBlock */
	GetSelCurChp (fFalse /* fForce */);
	blt (&selCur.chp, &chp, cwCHP);

	/* init local gray chp to all non gray */
	SetWords(&chpGray, 0, cwCHP);

	if (!selCur.fIns && !selCur.fGraphics)
		{
		if (selCur.fBlock)
			{

			/* selection is a block */

			InitBlockStat(&bks);
			while (FGetBlockLine(&cpStartRun, &dcp, &bks))
				{
				/* fFirstRun test forces us to do this at least once so chp will always be
				set up
				*/
				for (cpLim = cpStartRun + dcp; (cpStartRun < cpLim || fFirstRun);
						cpStartRun += vccpFetch)
					{
					/* interrupted? */
					if ( fAbortOk && FMsgPresent(mtyIdle))
						/* take heed: FMsgPresent call can call FetchCp,
							move the heap, etc. */
						{
						return (fFalse);
						}

					/* If any props are different, come up gray */
					FetchCpAndPara(doc, cpStartRun, fcmProps);
					/* if we ran out, say everything is gray */
					if (cpParaPrev != caPara.cpFirst)
						{
						cpara++;
						cpParaPrev = caPara.cpFirst;
						}
					if (!fAll && cpara > cparaMax)
						{
						SetWords(&chpGray, 0xFFFF, cwCHP);
						goto EndGetProps;
						}

					chpT = vchpFetch;

					chpT.fSpec = fFalse;  /* never inherit these */
					chpT.fRMark = fFalse;
					chpT.fStrike = fFalse;

					/* compare new props to previous ones */
					if (fFirstRun)
						{
						fFirstRun = fFalse;
						blt (&chpT, &chp, cwCHP);
						}
					else
						FSetRgwDiff (&chp, &chpT, &chpGray, cwCHP);
					}
				}
			/* replace the value gotten from GetSelCurChp which
				would always be the props at the start of the line for
				a block/column select
			*/
			if (cpara)
				blt(&chp, &selCur.chp, cwCHP);
#ifdef DEBUG
			else
				{
				/* in the case of selecting the last ftn/atn para, so FGetBlockLine would
					return false immediately, we could get here. In that case. the value
					from GetSelCurChp is ok, since there was only the para mark on the line,
					or selCur.cpFirst would not be >= cpMacDocEdit, so we can leave
					that value. However, in case there are other cases I have not considered,
					I am leaving this assert behind. bz
				*/
				struct DOD *pdod = PdodDoc(bks.doc);
				Assert (bks.cpFirst >= CpMacDocEdit(bks.doc));
				}
#endif /* DEBUG */

			}   /* block */
		else
			{
			/* normal selection, not an insertion point. Set up
			selCur.chp */

			cpLim = CpLimNoSpaces(&selCur.ca);
			for (cpStartRun = selCur.cpFirst; cpStartRun < cpLim;
					cpStartRun += vccpFetch)
				{
				/* interrupted? */
				if ( fAbortOk &&
		/* take heed: FMsgPresent call can call FetchCp, move the heap, etc. */
				FMsgPresent(mtyIdle))
					{
					return (fFalse);
					}

				/* If any props are different, come up gray */
				FetchCpAndPara( doc, cpStartRun, fcmProps);
				if (cpParaPrev != caPara.cpFirst)
					{
					cpara++;
					cpParaPrev = caPara.cpFirst;
					}

				/* if we ran out, say everything is gray */
				if (!fAll && cpara > cparaMax)
					{
					SetWords(&chpGray, 0xFFFF, cwCHP);
					break;
					}

				chpT = vchpFetch;

				chpT.fSpec = fFalse;  /* never inherit these */
				chpT.fRMark = fFalse;
				chpT.fStrike = fFalse;

				if (fFirstRun)
					{
					fFirstRun = fFalse;
					blt (&chpT, &chp, cwCHP);
					}
				else
					/* compare new props to previous ones */
					FSetRgwDiff (&chp, &chpT, &chpGray, cwCHP);
				}  /* while */

			}  /* else normal */
		}      /*  if (!selCur.fIns)   */

EndGetProps:
	/* indicates vchpGraySelCur reflects state of entire current
			selection. Note we set this even if hwwdCur == hNil and
			we are just graying out the chpGray. This should be ok,
			since when we open the first file, selCur.fUpdateChpGray
			will be set to true and FGetCharState will be called
			again.    */

	selCur.fUpdateChpGray = fFalse;

	/* in case someone besides the ribbon called me, set this to
			ensure ribbon update at idle  */

	selCur.fUpdateRibbon = fTrue;
	/* selCur.chp was set up either by GetSelCurChp for non-block/col
		sels or by bltting in chp in the block code above
	*/
	blt(&chpGray, &vchpGraySelCur, cwCHP);  /* zeroed for ins point, pic */

#ifdef DEBUG  /* save selection range and ww for CkLHRUpd debug test */
		{
		extern struct CA  vcaLHRUpd;
		extern int   vwwLHRUpd; /* window where selection is shown */

		vcaLHRUpd = selCur.ca;
		vwwLHRUpd = selCur.ww;
		}

	Assert(!selCur.chp.fSpec);
	Assert(!selCur.chp.fRMark);
	Assert(!selCur.chp.fStrike);
#endif

	return (fTrue);
}


/* ****
*  Description:  Return properties for the paragraph menu.
*  The current selection nongray para props are left in vpapSelCur and the
*  paragraph attributes in vpapGraySelCur are set to non-zero if that attribute
*  differs from that in the previous paragraph. Up to cparaMax paragraphs
*  will be checked
*
*    returns fFalse if interrupted, fTrue if complete. If interrupted,
*      selCur paps remain unchanged.
** **** */
#ifdef DEBUG
/* %%Function:C_FGetParaState %%Owner:davidlu */
HANDNATIVE C_FGetParaState(fAll, fAbortOk)
BOOL fAll;
BOOL fAbortOk;   /* if true, this routine is interruptible */
{
/* Note: for Opus, fAll is always false ?
*/

	/* max number of calls to CachePara */
#define cparaMax 50

	int cpara = 0;
	struct PAP pap, papGray;
	CP cpNext, cpT, cpLimT;
	BOOL fCol;

	/* we do some  word operations, so be sure sizes are good */
	Assert (!(cbPAPBase % sizeof(int)));
	Assert (!(cbPAP % sizeof(int)));

	/* gray out everything if no child window up */

	if (hwwdCur == hNil)
		{
		/* init gray pap to all gray */
		SetWords(&vpapGraySelCur, 0xFFFF, cwPAP);
		return (fTrue);  /* leave fUpdate bit on */
		}

	fCol = (selCur.sk == skColumn);

#ifdef DEBUG
	if (fCol)
		Assert (selCur.itcFirst >= 0 && selCur.itcLim >= 0);
#endif /* DEBUG */



	/* init entire local gray pap to all non gray */
	SetWords(&papGray, 0, cwPAP);
	cpNext = selCur.cpFirst;

	/* Need to initialize pap for insertion points and one line block
		selections because they don't get inside the following while loop. */
	CachePara (selCur.doc, cpNext);
	blt( &vpapFetch, &pap, cwPAP );

	while (cpNext < selCur.cpLim &&
			(fAll || (cpara <= cparaMax)))
		{

		/* interrupted? */
		if ( fAbortOk &&
		/* take heed: FMsgPresent can call CachePara, move the heap, etc */
		FMsgPresent(mtyIdle) )
			{
			return (fFalse);
			}

		if (fCol)
			CpFirstTap(selCur.doc, cpNext);

		cpT = (!fCol) ? cpNext : vmpitccp[selCur.itcFirst];
		/* will make non col case go only once */
		cpLimT = (!fCol) ? cpNext + 1 : vmpitccp[selCur.itcLim];
		while (cpT < cpLimT)
			{
			CachePara (selCur.doc, cpT);
			cpT = caPara.cpLim;
			if (++cpara == 1)
				blt( &vpapFetch, &pap, cwPAP );     /* save 1st paragraph for compares */
			else  if (!vpapFetch.fTtp)
				{
				/* If any props are different, set appropriate flags */
				/* get base of pap */
				C_FSetRgwDiff (&pap, &vpapFetch, &papGray,
						cbPAPBase / sizeof (int));
				/* also check in tab tables */
				C_FSetRgwDiff (pap.rgdxaTab, vpapFetch.rgdxaTab,
						papGray.rgdxaTab, pap.itbdMac);
				/* note chars, not words here */
				C_FSetRgchDiff (pap.rgtbd, vpapFetch.rgtbd,
						papGray.rgtbd, pap.itbdMac);
				}
			}

		cpNext = (!fCol) ? caPara.cpLim : caTap.cpLim;
		}

	/* if we ran out, say everything is gray */
	if (!fAll && cpara > cparaMax)
		SetWords(&vpapGraySelCur, 0xFFFF, cwPAP);
	else  /* normal finish */
		blt(&papGray, &vpapGraySelCur, cwPAP);

	/* vpapSelCur set to pap at start of sel */
	blt(&pap, &vpapSelCur, cwPAP);

	selCur.fUpdatePap = fFalse;

	/* if someone besides the ruler called me, set this to
		ensure ruler update at idle  */

	selCur.fUpdateRuler = fTrue;

#ifdef DEBUG  /* save selection range and ww for CkLHRUpd debug test */
		{
		extern struct CA  vcaLHRUpd;
		extern int   vwwLHRUpd; /* window where selection is shown */

		vcaLHRUpd = selCur.ca;
		vwwLHRUpd = selCur.ww;
		}
#endif

	return (fTrue);
}


#endif	/* DEBUG */


#ifdef DEBUG
/* %%Function:C_FSetRgwDiff %%Owner:davidlu */
C_FSetRgwDiff(pwBase,pwNew,pwDiff,cw)
int *pwBase;
int *pwNew;
int *pwDiff;
uns cw;
{
	int     wT;
	BOOL    fDiff;

	while (cw--)
		{
		*pwDiff++ |=  (wT = *pwBase++ ^ *pwNew++);
		fDiff |= wT;
		}
	return (fDiff != 0);
}


#endif	/* DEBUG */


#ifdef DEBUG
/* %%Function:C_FSetRgchDiff %%Owner:davidlu */
C_FSetRgchDiff(pchBase,pchNew,pchDiff,cch)
char *pchBase;
char *pchNew;
char *pchDiff;
uns cch;
{
	char    chT;
	BOOL    fDiff;

	while (cch--)
		{
		*pchDiff++ |=  (chT = *pchBase++ ^ *pchNew++);
		fDiff |= chT;
		}
	return (fDiff != 0);
}


#endif	/* DEBUG */


/* %%Function:WParseFontSize %%Owner:davidlu */
EXPORT WORD WParseFontSize(tmm, sz, ppv, bArg, tmc, wParam)
TMM tmm;
char * sz;
void ** ppv;
WORD bArg;
TMC tmc;
WORD wParam;
{
	DPV dpv;
	int hps;
	char * pch;

	switch (tmm)
		{
#ifdef tmmCwVal 
	case tmmCwVal:
		return 1;
#endif

	case tmmFormat:
		hps = WFromPpvB(ppv, bArg);
		pch = sz;

		if (hps != wNinch)
			CchHpsToPpch(hps, &pch);

		*pch = '\0';

		return fTrue;

	case tmmParse:
		CchStripString(sz, CchSz(sz) - 1);
		dpv = DpvParseFdxa(&hps, tmc, sz, 4, 127,
				dpvBlank | dpvSpaces | dpvDouble, 
				eidDxaOutOfRange, fFalse, 0);

		if (dpv == dpvError)
			{
			SetPpvBToW(ppv, bArg, wError);
			return fFalse;
			}

		if (dpv != dpvNormal && dpv != dpvDouble)
			hps = wNinch;

		SetPpvBToW(ppv, bArg, hps);
		return fTrue;
		}

	return 0;
}



/* %%Function:WParseFontName %%Owner:davidlu */
EXPORT WORD WParseFontName(tmm, sz, ppv, bArg, tmc, wParam)
TMM tmm;
char * sz;
void ** ppv;
WORD bArg;
TMC tmc;
WORD wParam;
{
	int ftc, cch;
	struct FFN * pffn;
	char rgch [cbFfnLast];

	switch (tmm)
		{
#ifdef tmmCwVal 
	case tmmCwVal:
		return 1;
#endif

	case tmmFormat:
		ftc = WFromPpvB(ppv, bArg);
		*sz = '\0';
		if (ftc != wNinch && selCur.doc != docNil)
			{
			int ibst;

			if ((ibst = IbstFontFromFtcDoc(ftc, selCur.doc))
					!= iNil)
				{
				CchCopySz((char *) ((struct FFN *)
						PstFromSttb(vhsttbFont, ibst))->szFfn,
						sz);
				}
			}
		return fTrue;

	case tmmParse:
		pffn = (struct FFN *) rgch;
		if (CchSz(sz) > LF_FACESIZE-1)
			{
			bltbyte( sz, pffn->szFfn, LF_FACESIZE-1 );
			pffn->szFfn [LF_FACESIZE-1] = '\0';
			cch = LF_FACESIZE;
			}
		else
			cch = CchCopySz( sz, pffn->szFfn );

		cch = CchStripString(pffn->szFfn, cch );
		Assert (cch >= 0);
		if (!cch)
			ftc = wNinch;
		else
			{
			ftc = FtcValidateFont(pffn);
			if (ftc == valNil)
				{
				SetPpvBToW(ppv, bArg, wError);
				SetTmcTxs(tmcCharName, TxsOfFirstLim(0, ichLimLast));
				return fFalse;
				}
			}
		SetPpvBToW(ppv, bArg, ftc);
		return fTrue;
		}

	return 0;
}


#endif /* WIN */

/* S O R T  R G B */
/* sorts the array rgb with respect to FGt */
/* copy of the sort routine from file.c adapted for sorting an rgb */
/* %%Function:SortRgb %%Owner:davidlu */
SortRgb(rgb, ibMac, FGt)
char *rgb;
int ibMac, (*FGt)();
{
	int ib;
	if (ibMac < 2) return;
	for (ib = ibMac>>1; ib >= 2; --ib)
		SortSiftUpB(rgb, ib, ibMac, FGt);
	for (ib = ibMac; ib >= 2; --ib)
		{
		char b;
		SortSiftUpB(rgb, 1, ib, FGt);
		b = rgb[0];
		rgb[0] = rgb[ib - 1];
		rgb[ib - 1] = b;
		}
}


/* S O R T  S I F T  U P  B*/
/* see Floyd, Robert W. Algorithm 245 TREESORT 3 [M1] CACM 7, December 1964. */
/* %%Function:SortSiftUpB %%Owner:davidlu */
SortSiftUpB(rgb, ibI, ibN, FGt)
char *rgb;
int ibI, ibN, (*FGt)();
{
	int ibJ;
	char bCopy;

	bCopy = rgb[ibI - 1];
Loop:
	ibJ = 2 * ibI;
	if (ibJ <= ibN)
		{
		if (ibJ < ibN)
			{
			if ((*FGt)(rgb[ibJ], rgb[ibJ - 1]))
				ibJ++;
			}
		if ((*FGt)(rgb[ibJ - 1], bCopy))
			{
			rgb[ibI - 1] = rgb[ibJ - 1];
			ibI = ibJ;
			goto Loop;
			}
		}
	rgb[ibI - 1] = bCopy;
}


#ifdef WIN
#ifdef PROFILE
/*  this is here so that appended native code does not appear in previous
	function in pcode profiles. */
Fetch2_Last(){}
#endif /* PROFILE */
#endif /* WIN */

/* ADD NEW CODE *ABOVE* Fetch2_Last() */
