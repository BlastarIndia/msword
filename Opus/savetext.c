/* S A V E T E X T . C */
/* Code for writting documents in text (unformatted) form
*/

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "doc.h"
#include "props.h"
#include "file.h"
#include "sel.h"
#include "disp.h"
#include "ch.h"
#include "format.h"
#include "screen.h"
#include "filecvt.h"
#include "message.h"
#include "prompt.h"
#include "debug.h"



extern int              vfScratchFile;
extern int              docMac;
extern int              fnFetch;
extern int              vdocTemp;
extern int              vfGlsyMerge;
extern int              docGlsy;
extern int              vibp;
extern int              wwCur;
extern struct SEL       selCur;
extern struct DOD       **mpdochdod[];
extern struct FCB       **mpfnhfcb[];
extern struct CA        caPara;
extern struct CHP       vchpFetch;
extern struct UAB       vuab;
extern struct MERR      vmerr;
extern struct FKPD      vfkpdChp;
extern struct FKPD      vfkpdPap;
extern struct FKPD      vfkpdText;
extern struct SEP       vsepFetch;
extern struct FLI       vfli;
extern struct PREF      vpref;
extern struct RULSS     vrulss;
extern CP               vcpFetch;
extern int              vccpFetch;
extern char HUGE        *vhpchFetch;
extern struct WWD       **mpwwhwwd[];
extern struct PAP       vpapFetch;
extern int              cwHeapFree;
extern int              wwMac;
extern int              fnMac;
extern struct BPTB      vbptbExt;
extern CHAR             rgchEop[];

#ifdef NOASM
extern FC               FcAppendRgchToFn();
#endif /* NOASM */
extern void             AppendRgchToFnRtf();
extern char             (**vhgrpchr)[];


extern struct FMTSS     vfmtss;
extern int              vdocFetch;
#ifdef DEBUG
extern struct DBS       vdbs;
#endif


/* C C P  W R I T E   T E X T */
/* %%Function:CcpWriteText  %%Owner:peterj */
CP CcpWriteText(fn, doc, ccpSoFar, ccpTotal, fTranslate)
int fn; 
int doc; 
CP ccpSoFar, ccpTotal; 
BOOL fTranslate;
{
/* Write out unformatted file */
	CP cp, cpMac = CpMacDoc(doc);
	char HUGE *hpch, HUGE *hpchFirst, HUGE *hpchLim;
	int chT;
	int iCount = 0;
	int cch;
	char rgch[ichChSpecMax];
	char rgchBuff[cbSector+1];

	Assert (ccpSoFar + cpMac <= ccpTotal);

	for (cp = cp0; cp < cpMac && !vmerr.fDiskAlert && 
			!vmerr.fDiskWriteErr; cp += cch)
		{
		FetchCp(doc, cp, fcmChars);

		/* show % in status line */
		ReportSavePercent(cp0, ccpTotal, ccpSoFar+cp, fFalse);
			{{ /* NATIVE - CcpWriteText */
			for (hpch = hpchFirst = vhpchFetch, hpchLim = hpch + vccpFetch;
					hpch < hpchLim; hpch++)
				{
				chT = *hpch;
				switch (chT)
					{
					/* these fSpec chars should not be in text */
				case chTFtn:
				case chTFtnCont:
				case chLnn:
				case chPicture:
				case chFootnote:
				case chAtn:
				case chFieldBegin:
				case chFieldEnd:
				case chFieldSeparate:
						/* possible fSpec characters */
						{{ /* !NATIVE - CcpWriteText */
						if ((cch = (int)(hpch - hpchFirst)) != 0)
							{
							if (fTranslate)
								{
								bltbh(hpchFirst, (CHAR HUGE *) rgchBuff, cch);
								rgchBuff[cch] = 0;
								AnsiToOem(rgchBuff, rgchBuff);
								hpchFirst = (CHAR HUGE *) rgchBuff;
								}
							FcAppendHprgchToFn(fn, hpchFirst, cch);
/* warning: hpchFirst now invalid */
							}
						cp += cch;
						CachePara(doc, cp);
						FetchCp(doc, cp, fcmProps);
						FetchCp(doc, cp, fcmChars);
						hpch = hpchFirst = vhpchFetch;
						hpchLim = hpch + vccpFetch;

						if (vchpFetch.fSpec)
							/* fSpec characters, replace with useful
								form */
							{
							Assert (chT != chTFtn && chT != chLnn &&
									chT != chTFtnCont);

							/*  get character representation */
							vfmtss.cpRef = cp;
							vfli.doc = vdocFetch;
							vfmtss.pgn = 1;

							if ((cch = CchChSpec(chT, &rgch)) != 0)
								FcAppendRgchToFn(fn, &rgch, cch);

							InvalFli();

							cp += 1;
							hpchFirst++;

							}
						/* else put in result */
						}}
					break;

				case chSect:
				case chCRJ:
				case chColumnBreak:
				case chNonReqHyphen:
				case chNonBreakSpace:
				case chNonBreakHyphen:
				case chPubLDblQuote:
				case chPubRDblQuote:
				case chPubBullet:
				case chPubEmDash:
				case chPubEnDash:
				case chLQuote:
				case chRQuote:
				case chTable:
						/* these must be replaced with other forms */
						{{ /* !NATIVE - CcpWriteText */
						if ((cch = (int)(hpch - hpchFirst)) != 0)
							{
							if (fTranslate)
								{
								bltbh(hpchFirst, (CHAR HUGE *) rgchBuff, cch);
								rgchBuff[cch] = 0;
								AnsiToOem(rgchBuff, rgchBuff);
								hpchFirst = (CHAR HUGE *) rgchBuff;
								}
							FcAppendHprgchToFn(fn, hpchFirst, cch);
/* warning: hpchFirst now invalid */
							}

						/* point at the next character along as
							begining of next run */
						cp += cch + 1;
						hpchFirst = hpch + 1;

						/* do process according to ch being replaced */
						switch (chT)
							{
						case chTable:
							/* replace with chEop */
							*rgch = chEop;
							goto LTranslateCh;
						case chSect:
						case chCRJ:
						case chColumnBreak:
							FcAppendRgchToFn(fn, rgchEop, cchEop);
							break;
						case chNonReqHyphen:
							/* this results in nothing */
							break;
						case chNonBreakSpace:
							/* replace with space */
							*rgch = chSpace;
							goto LTranslateCh;
						case chPubEmDash:
						case chPubEnDash:
						case chNonBreakHyphen:
							/* replace with hyphen */
							*rgch = chHyphen;
LTranslateCh:	                        
							FcAppendRgchToFn(fn, &rgch, 1);
							break;
						case chPubLDblQuote:
						case chPubRDblQuote:
							*rgch = chDQuote;
							goto LTranslateCh;
						case chPubBullet:
							*rgch = 'o';
							goto LTranslateCh;
						case chLQuote:
						case chRQuote:
							*rgch = '\'';
							goto LTranslateCh;
							}
						}}
					break;
					}
				}
			}}
		if ((cch = (int)(hpch - hpchFirst)) != 0)
			{
			if (fTranslate)
				{
				bltbh(hpchFirst, (CHAR HUGE *) rgchBuff, cch);
				rgchBuff[cch] = 0;
				AnsiToOem(rgchBuff, rgchBuff);
				hpchFirst = (CHAR HUGE *) rgchBuff;
				}
			FcAppendHprgchToFn(fn, hpchFirst, cch);
/* warning: hpchFirst now invalid */
			}
		}
	return cp;
}


/* C C P  W R I T E   T E X T  W I T H ... */
/* %%Function:CcpWriteTextWithLineBreaks  %%Owner:peterj */
CP CcpWriteTextWithLineBreaks(fn, doc, ccpSoFar, ccpTotal, fTranslate)
int fn; 
int doc; 
CP ccpSoFar, ccpTotal; 
BOOL fTranslate;
{
	int ichMac;
	int iCount = 0;
	CP cp, cpMac = CpMacDoc(doc);
	GRPFVISI grpfvisi;
	int dchLine;

	/* FUTURE: should really be combined with clipboard writing routines */

	/* Set up view preferences for rendered text in wwTemp */
	grpfvisi.w = 0;
	grpfvisi.grpfShowResults = ~0;
	grpfvisi.fNoShowPictures = fTrue;
	grpfvisi.fForceField = fTrue;
	grpfvisi.flm = FlmDisplayFromFlags( vpref.fDisplayAsPrint, vpref.fPageView, vpref.fDraftView );
	PwwdWw(wwTemp)->grpfvisi = grpfvisi;
	LinkDocToWw( doc, wwTemp, wwNil );

	Assert (ccpSoFar + cpMac <= ccpTotal);

	cp = cp0;
	while (cp < cpMac && !vmerr.fDiskAlert && !vmerr.fDiskWriteErr)
		{
		/* show % in status line */
		ReportSavePercent(cp0, ccpTotal, ccpSoFar+cp, fFalse);
		FormatLine(wwTemp, doc, cp);
		ichMac = min(vfli.ichMac, ichMaxLine-(1+ccpEop));
/* if break was eop or section mark, backup so we can replace the displayed
	equivalent of that character in vfli.rgch with a real chEop. */
		if (vfli.chBreak == chEop || vfli.chBreak == chSect ||
				vfli.chBreak == chCRJ || vfli.chBreak == chTable)
			ichMac--;
		bltbyte( rgchEop, &vfli.rgch [ichMac], cchEop );
		ichMac += cchEop;
		dchLine = CchSanitizeRgch(vfli.rgch, ichMac, ichMaxLine /* sizeof vfli.rgch */, fTrue /*fSpecToCRLF*/);
		if (fTranslate)
			{
			vfli.rgch[dchLine] = 0;
			AnsiToOem(vfli.rgch, vfli.rgch);
			}
		FcAppendRgchToFn(fn, &vfli.rgch, dchLine);
		cp = vfli.cpMac;
		}
	InvalFli();

	return cpMac;
}


/* W R I T E  T E X T  C O M M O N */
/*  note: save percentage report will appear iff vpstSaving is non NULL. */
/* %%Function:WriteTextCommon  %%Owner:peterj */
WriteTextCommon(dff, fn, doc)
int dff; 
int fn, doc;
{
	int docT;
	BOOL fTranslate = (dff == dffSaveText8 || dff == dffSaveText8CR);
	CP (*CcpWrite)() = (dff == dffSaveText || dff == dffSaveText8) ? 
			CcpWriteText : CcpWriteTextWithLineBreaks;
	CP cpTotal, cpSoFar = 0;
	struct FCB *pfcb;

	Assert (doc != docNil && !PdodDoc (doc)->fShort);

	/* calculate total number of cps in "doc" */
	cpTotal = CpMacDoc (doc);
	if ((docT = PdodDoc(doc)->docFtn) != docNil)
		cpTotal += CpMacDoc (docT);
	if ((docT = PdodDoc(doc)->docAtn) != docNil)
		cpTotal += CpMacDoc (docT);
	if ((docT = PdodDoc(doc)->docHdr) != docNil)
		cpTotal += CpMacDoc (docT);

	/* assure proposed length CAN be represented */
	if (!FCheckFcLegal(cpTotal))
		return;

	/* write the doc */
	cpSoFar = (*CcpWrite)(fn, doc, cpSoFar, cpTotal, fTranslate);
	if ((docT = PdodDoc(doc)->docFtn) != docNil)
		cpSoFar += (*CcpWrite)(fn, docT, cpSoFar, cpTotal, fTranslate);
	if ((docT = PdodDoc(doc)->docAtn) != docNil)
		cpSoFar += (*CcpWrite)(fn, docT, cpSoFar, cpTotal, fTranslate);
	if ((docT = PdodDoc(doc)->docHdr) != docNil)
		cpSoFar += (*CcpWrite)(fn, docT, cpSoFar, cpTotal, fTranslate);


	/* show % in status line */
	ReportSavePercent (cp0, 1L, 1L, fFalse);
	pfcb = PfcbFn(fn);
	pfcb->pnXLimit = (PN)
			((pfcb->cbMac + cbSector - 1) >> shftSector);
}



#ifdef WIN /* in inssubs.c if MAC */
/* F C  A P P E N D  H P R G C H  T O  F N */
/* Appends characters pointed to by hpch, length cch, to end of file fn.
Returns first fc written. */

/* %%Function:FcAppendHprgchToFn  %%Owner:peterj */
FC FcAppendHprgchToFn(fn, hpch, cch)
int fn;
char HUGE *hpch;
int cch;
{
	struct FCB *pfcb = *mpfnhfcb[fn];
	pfcb->fcPos = pfcb->cbMac;
	WriteHprgchToFn(fn, hpch, cch);
	return (pfcb->cbMac);
}


#endif /* WIN */
