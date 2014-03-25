#define NODRAWFRAME
#define NORASTEROPS
#define NOCTLMGR
#define NOMINMAX
#define NOMSG
#define NORECT
#define NOSCROLL
#define NOKEYSTATE
#define NOCREATESTRUCT
#define NOICON
#define NOPEN
#define NOREGION
#define NODRAWTEXT
/* #define NOMB */
#define NOWINOFFSETS
/* #define NOMETAFILE */
#define NOCLIPBOARD
#define NOSOUND
#define NOCOMM
#define NOKANJI
/* #define NOGDI */

#define FONTS
#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "version.h"
#include "heap.h"
#include "doc.h"
#include "disp.h"
#include "props.h"
#include "border.h"
#include "debug.h"
#include "pic.h"
#include "prm.h"
#include "sel.h"
#include "file.h"
#include "layout.h"
#include "ch.h"
#include "fontwin.h"
#include "inter.h"
#include "error.h"

#define RTFDEFS
#include "rtf.h"

#include "field.h"
#include "strtbl.h"
#define RTFTABLES
#define RTFOUT
#include "rtftbl.h"
#include "style.h"
#include "dde.h"



int		FPropToRtfSpecMacPlus();
int		FEmitForChpx();
DWORD           GetRgbIco(int);

extern struct CHP	vchpNormal;
extern struct PAP	vpapFetch;
extern struct CHP	vchpFetch;
extern struct TAP vtapFetch;
extern struct TCC vtcc;
extern int		fnFetch;

extern struct CA	caPara;
extern struct CA	caSect;
extern struct CA  caTap;
extern struct MERR	vmerr;
extern CP		vcpFetch;
extern int		vccpFetch;
extern int		vfPCFlip;
extern struct PIC	vpicFetch;
extern FC               vfcFetchPic;    /* fc of last fetched pe */
extern CHAR HUGE        *vhpchFetch;
extern struct DOD	**mpdochdod[];
extern struct SPX	mpiwspxSep[];
extern int		docGlobalDot;
extern int		rgftc[iftcMax];
extern int		viftcMac;
extern struct SAB       vsab;

extern struct SEP	vsepFetch;
extern struct CHP	vchpStc;
extern struct STTB	**vhsttbFont;

extern CHAR  rgchEop[];
extern int ftcMapDef;

#ifdef DEBUG
extern struct DBS	vdbs;
#endif /* DEBUG */

int vfRTFInternal;  /* when true we are writing RTF for internal consumption */
int vcchEopRtf; /* count of chars since last chEop emitted by RtfOut */

#ifdef NOASM
extern char *PchSzRtfMove();
#endif /* NOASM */
void AppendRgchToHandleRtf();
void AppendRgchToFnRtf();

csconst char rgchRTF[] = {
	'{', '\\','r','t','f'};


/*	R T F	O U T

Inputs:
	pca	Text to output
	pfnWrite	Function to write characters
	pArgWrite	Argument to pfnWrite (usually fn)
	roo		RtfOut Options


Writes RTF form text given text in internal form. Options are listed in rtf.h.
*/
/*  %%Function:  RtfOut  %%Owner:  bobz       */

RtfOut(pca, pfnWrite, pArgWrite, roo)
struct CA *pca;
int (*pfnWrite)();
int *pArgWrite;
int roo;
{
	int fNewPara, fNewSect;
	char *pch;
	char *pchOut;
	char *pchRebl;
	struct PLC **hplcfrd;
	struct PLC **hplcatrd;
	int ifrd, iatrd, ihdd;
	CP cpNextFrd;
	CP cpNextAtrd;
	CP cpNextSea;
	CP cpFirstPara;
	CP cpFetchAdjusted = cpNil;
	FC    fcPic;
	long  cfcCur;
	long  cfcPicT;
	int   cstcStd;
	struct CA caT;
	int stcp, stcpMac;
	int stc;
	int ich;
	char *pst;
	int fPicFSpec;
	int ipe;
	int fBracketProp = fFalse;
	   /* set true to force char prop output 1st time, before chp set to vchpFetch */
	int fBracketBroken = fTrue;
	int fSpec;
	int fDelimField;
	int fFacingPages;
	int fPutSpace;
	int fNewTap = fFalse;
	int fNewTc = fFalse;
	struct TC *ptc;
	int *pdxa;
	int itc;
	struct TAP tap;
	struct TC tc;
	struct CHP chpx;
	int maskSetZero;
	struct PLC **hplcsea;
	char rgbSea[cbSEA];
	int isea;
	char *pchT;
	char *pchLimT;
	int isz;
	int ch;
	int (*pfnFEmit)();
	struct STTB **hsttbName;
	struct PLESTCP *pplestcp;
	struct ESTCP *pestcp;
	struct DOD *pdod;
	struct CHP *pchp;
	struct CHP chp, chpT;
	struct PAP pap, papT;
	struct REBL rebl;
	char stName[256];

	struct BKMKRTF bkmkRtf;

	int doc = pca->doc;
	int docMother = DocMother (doc);

	int docHdr = docNil;
	int docFtn = docNil;
	int docAtn = docNil;

	int stateRdsField = rdsMain; /* used for state when rtfouting fields */

	/* this stack lets us determine the current field type in nested fields */
	struct FLDS rgfltStack[cNestFieldMax];
	int ifldsTop = 0;
	struct FLDS fldsCur;
	BOOL fHadSlashFld = fFalse;

	bkmkRtf.fHaveBkmks = fFalse;
	bkmkRtf.fHaveBkmkInRun = fFalse;

	fldsCur = 0;
	fldsCur.flt = fltNil;
#ifdef MAC
	rebl.sl = slWord;
#endif /* MAC */

	rebl.pArgWrite = pArgWrite;
	rebl.pfnWrite = pfnWrite;

	/* **************** WARNING ******************* */
	/* Rtfout is not a state machine; though it can take a range of cps to
		process, it assumes that they are a well-formed group. It can handle
		coming in in the middle of a paragraph or section, though it will put out
		the section/para/char properties again.
	
		However, at very least, it cannot handle starting in the middle of a
		field since it uses the field fspec characters to start up its output
		properly. For that reason we check FLegalSel here. If any other
		restrictions become clear they should be tested for here
	*/

	Assert (FLegalSel (doc, pca->cpFirst, pca->cpLim));

	/* **************** End WARNING ******************* */



	vfRTFInternal = (roo & rooInternal) != 0;

	if (roo & rooBegin)
		{
		  /* global used by rtfstandardchp; may be reset by rtfin. For
			 rtfout should always be the Opus (not RTF) default font.
		  */
		ftcMapDef = ftcDefault;
		RtfVersion(&rebl);
		}
	pdod = PdodDoc(doc);
	FreezeHp();  /* for pdod */

	if (pdod->fShort)
		{ /* Header or footnote or annotation doc */
		hplcfrd = 0;
		hplcatrd = 0;
		cpNextFrd = pca->cpLim;
		cpNextAtrd = pca->cpLim;
		cpNextSea = pca->cpLim;
		}
	else
		{
		hplcfrd = pdod->hplcfrd;
		if (hplcfrd != 0)
			{
			ifrd = IInPlcRef(hplcfrd, pca->cpFirst);
			cpNextFrd = CpPlc( hplcfrd, ifrd );
			docFtn = pdod->docFtn;
			Assert(docFtn != docNil);
			}
		else
			cpNextFrd = pca->cpLim;

		hplcatrd = pdod->hplcatrd;
		if (hplcatrd != 0)
			{
			iatrd = IInPlcRef(hplcatrd, pca->cpFirst);
			cpNextAtrd = CpPlc( hplcatrd, iatrd );
			docAtn = pdod->docAtn;
			Assert(docAtn != docNil);
			}
		else
			cpNextAtrd = pca->cpLim;

		docHdr = pdod->docHdr;

		hplcsea = pdod->hplcsea;
		if (hplcsea != 0)
			{
			if (CpPlc( hplcsea, IMacPlc( hplcsea )) <= pca->cpFirst)
				cpNextSea = pca->cpLim;
			else
				{
				isea = (CpPlc( hplcsea, 0 ) > pca->cpFirst) ?
						0 : IInPlc(hplcsea, pca->cpFirst);
				cpNextSea = CpPlc( hplcsea, isea );
				}
			}
		else
			cpNextSea = pca->cpLim;
		}

	MeltHp();  /* for pdod */

	if (roo & rooDoc)
		{ /* Write font table */
		int iftc;
		int fmc;
		char stFont[LF_FACESIZE + 1];

		pdod = PdodDoc(doc);
		FreezeHp();  /* for pdod */

		pchRebl = rebl.rgch;
		*pchRebl++ = chRTFOpenBrk;
		*pchRebl++ = chBackslash;
		pchRebl = PchSzRtfMove(iszRTFfonttbl, pchRebl);
		for (iftc = 0; iftc < pdod->ftcMac; iftc++)
			{
			*pchRebl++ = chRTFOpenBrk;
			*pchRebl++ = chBackslash;
			pchRebl = PchSzRtfMove(iszRTFf, pchRebl);
			CchIntToPpch(iftc, &pchRebl);
			*pchRebl++ = chBackslash;
			GetFontFamily((**pdod->hmpftcibstFont)[iftc], &fmc);
			switch (fmc)
				{
			case FF_MODERN:
				isz = iszRTFfmodern;
				break;

			case FF_ROMAN:
				isz = iszRTFfroman;
				break;

			case FF_SWISS:
				isz = iszRTFfswiss;
				break;

			case FF_SCRIPT:
				isz = iszRTFfscript;
				break;

			case FF_DECORATIVE:
				isz = iszRTFfdecor;
				break;

			default:
				isz = iszRTFfnil;
				break;
				}
			pchRebl = PchSzRtfMove(isz, pchRebl);
			*pchRebl++ = ' ';
			GetFontName((**pdod->hmpftcibstFont)[iftc], stFont);
			pchRebl = bltbyte(stFont + 1, pchRebl, stFont[0]);
			*pchRebl++ = ';';
			*pchRebl++ = chRTFCloseBrk;
			FlushRebl(&rebl, &pchRebl);
			}
		*pchRebl++ = chRTFCloseBrk;
		FlushRebl(&rebl, &pchRebl);
		MeltHp();  /* for pdod */
		}

	if (roo & rooDoc)
		{
		long rgb;
		int ico, i;

		pchRebl = rebl.rgch;
		*pchRebl++ = chRTFOpenBrk;
		*pchRebl++ = chBackslash;
		pchRebl = PchSzRtfMove(iszRTFcolortbl, pchRebl);
		*pchRebl++ = ';';   /* for auto */
		for (ico = 1; ico < icoMax; ico++)
			{
			int isz;
			char iColor;

			rgb = GetRgbIco(ico);
			for (i = 0; i < 3; i++)
				{
				*pchRebl++ = chBackslash;
				switch (i)
					{
				case 0:
					isz = iszRTFred;
					iColor = GetRValue(rgb);
					break;
				case 1:
					isz = iszRTFgreen;
					iColor = GetGValue(rgb);
					break;
				case 2:
					isz = iszRTFblue;
					iColor = GetBValue(rgb);
					break;
					}
				pchRebl = PchSzRtfMove(isz, pchRebl);
				CchIntToPpch((unsigned)iColor, &pchRebl);
				}
			*pchRebl++ = ';';
			FlushRebl(&rebl, &pchRebl);
			}
		*pchRebl++ = chRTFCloseBrk;
		FlushRebl(&rebl, &pchRebl);
		}

	if (roo & rooDoc)
		{ /* Write style sheet */
		pchRebl = rebl.rgch;
		*pchRebl++ = chRTFOpenBrk;
		*pchRebl++ = chBackslash;
		pchRebl = PchSzRtfMove(iszRTFstylesheet, pchRebl);
		pdod = PdodDoc(docMother);
		FreezeHp();  /* for pdod */
		cstcStd = pdod->stsh.cstcStd;
		hsttbName = pdod->stsh.hsttbName;
		pplestcp = *pdod->stsh.hplestcp;
		stcpMac = (*hsttbName)->ibstMac;
		for (stcp = 0; stcp < stcpMac; stcp++)
			{
			if (FStcpEntryIsNull(hsttbName, stcp))
				continue;
			*pchRebl++ = chRTFOpenBrk;
			FlushRebl(&rebl, &pchRebl);
			MapStc(pdod, StcFromStcp(stcp, cstcStd), &chpT, &papT);
			PropToRtf(&rebl, &papT, irrbPapFirst, irrbPapLim,
					0, FPropToRtfSpecMacPlus);
			PropToRtf(&rebl, &chpT, irrbChpFirst, irrbChpLim,
					0, FPropToRtfSpecMacPlus);
			pchRebl = rebl.rgch;

			pestcp = &pplestcp->dnstcp[stcp];
			if (pestcp->stcBase != stcStdMin)
				{
				*pchRebl++ = chBackslash;
				pchRebl = PchSzRtfMove(iszRTFsbasedon, pchRebl);
				CchIntToPpch(pestcp->stcBase, &pchRebl);
				}
			*pchRebl++ = chBackslash;
			pchRebl = PchSzRtfMove(iszRTFsnext, pchRebl);
			CchIntToPpch(pestcp->stcNext, &pchRebl);
			*pchRebl++ = ' ';
			GenStyleNameForStcp(stName, hsttbName, cstcStd, stcp);
			pchRebl = bltbyte(stName + 1, pchRebl, stName[0]);
			*pchRebl++ = ';';
			*pchRebl++ = chRTFCloseBrk;
			FlushRebl(&rebl, &pchRebl);
			}
		*pchRebl++ = chRTFCloseBrk;
		FlushRebl(&rebl, &pchRebl);
		MeltHp();  /* for pdod */
		}



	if (roo & rooInfo)

#ifdef DEBUG  /* no info block if true - bz test */
		if (!vdbs.fNoInfoRTF)
#endif
			{ /* Write info block */
			int flt;
			int cch;
			struct STTB **hsttbAssoc = PdodDoc (docMother)->hsttbAssoc;
			char *pchSrc;
			int fNoConvPlain;

			pchRebl = rebl.rgch;
			*pchRebl++ = chRTFOpenBrk;
			*pchRebl++ = chBackslash;
			pchRebl = PchSzRtfMove(iszRTFinfo, pchRebl);
			FlushRebl(&rebl, &pchRebl);

		/* skip \info field - done already */
			for (flt = fltInfoMin + 1; flt <= fltInfoMaxRTF; flt++)
				{
			/* slightly odd mechanism here: we shove <opnbrk>\<title>
			into rebl.rgch, then call CchSzInfoFrom... which puts the
			value into stName.  If cch == 0, we just bag out
			and throw away what we put in. Otherwise we dump the
			leading text, and convert the info text to plain text,
			escaping special chars. The way we do it allows the
			plain text to be > 255 chars, though the reconverted
			text could not be.
			*/
				pchRebl = rebl.rgch;
				*pchRebl++ = chRTFOpenBrk;
				*pchRebl++ = chBackslash;

				fNoConvPlain = fFalse;
				fPutSpace = fFalse;
				switch (flt)
					{
				case fltTitle:
					isz = iszRTFtitle;
					fPutSpace = fTrue;
					break;
				case fltSubject:
					isz = iszRTFsubject;
					fPutSpace = fTrue;
					break;
				case fltAuthor:
					isz = iszRTFauthor;
					fPutSpace = fTrue;
					break;
				case fltKeyWords:
					isz = iszRTFkeywords;
					fPutSpace = fTrue;
					break;
				case fltComments:
					isz = iszRTFdoccomm;
					fPutSpace = fTrue;
					break;
				case fltLastRevBy:
					isz = iszRTFoperator;
					fPutSpace = fTrue;
					break;
				case fltCreateDate:
					isz = iszRTFcreatim;
					fNoConvPlain = fTrue;
					break;
				case fltRevDate:
					isz = iszRTFrevtim;
					fNoConvPlain = fTrue;
					break;
				case fltPrintDate:
					isz = iszRTFprintim;
					fNoConvPlain = fTrue;
					break;
				case fltRevNum:
					isz = iszRTFversion;
					break;
				case fltEditTime:
					isz = iszRTFedmins;
					fNoConvPlain = fTrue;
					break;
				case fltNumPages:
					isz = iszRTFnofpages;
					break;
				case fltNumWords:
					isz = iszRTFnofwords;
					break;
				case fltNumChars:
					isz = iszRTFnofchars;
					break;


#ifdef INFOONLY
			/*  not supported in rtf */
				case fltFileName:
					break;
				case fltDot:  /* template handled elsewhere */
					break;
#endif /* INFOONLY */


				default:
					Assert (fFalse);
					continue;   /* ignore invalid data */
					}
				pchRebl = PchSzRtfMove(isz, pchRebl);
				if (fPutSpace)
					*pchRebl++ = ' ';
			/* note we use stName as a normal rgch here */
				cch = CchSzInfoFromIifd (IifdFromFlt(flt), doc, stName,
						hsttbAssoc, ifcRTF, NULL);
				Assert (cch <= 255);

				if (cch)
					{
		/* dates have backslashes we don't convert */
					if (fNoConvPlain)
						{
						pchRebl = bltbyte(stName, pchRebl, cch);
						*pchRebl++ = chRTFCloseBrk;
						FlushRebl(&rebl, &pchRebl);
						}

					else
						{
						pchSrc = stName;
#ifdef BZTESTX
						CommSzNum(SzShared("cch before dumping info = "), cch);
#endif
						while (cch)
							{

				/* NOTE that neither of these pchRebl/pchSrc points
				to a heap object so I can rely on them
				being undamaged as I loop
				*/
						/* when we first get here, keyword is not dumped yet.
							This assures that kwd and numeric arg will not be split.
							after 1st flush, pchRebl == rebl.rgch, so full buffer
							available.
						*/

				/* Dch... returned the # of input chars used. and
				adjusted the pointers to point past the last
				char in the destination buffer and to the next src
				char. If the source chars
				are not all used, then the rgch filled up
				so dump it. If they are used up, there is space
				for the closing brace, so fall through and dump
				all at once
				*/
							cch -= DchTextToPlainText(&pchRebl, &pchSrc, cch,
									sizeof (rebl.rgch) - 1 - (pchRebl - rebl.rgch));
#ifdef BZTESTX
							CommSzNum(SzShared("cch after plain text conv = "), cch);
#endif

							if (cch)
								FlushRebl(&rebl, &pchRebl );  /* dump filled dest buffer */
							}
						*pchRebl++ = chRTFCloseBrk;
						FlushRebl(&rebl, &pchRebl);  /* dump data and brace */
						}
					}
				}
		/* version is in the info block, but not a field. We put out
		the internal Opus version # followed by the locale
		with a space between the numbers
		*/

			pchRebl = rebl.rgch;
			*pchRebl++ = chRTFOpenBrk;
			*pchRebl++ = chBackslash;
			pchRebl = PchSzRtfMove(iszRTFvern, pchRebl);
			CchUnsToPpch(nProductCurrent, &pchRebl);
			*pchRebl++ = chRTFCloseBrk; /* for version */

			*pchRebl++ = chRTFCloseBrk; /* for info block */
			FlushRebl(&rebl, &pchRebl);
			}


	/* set up tables to search for bookmarks, if any */

	if (!(roo & rooSub)) /* subdocs don't have bookmarks */
		{
		int ibkmk;
		int iFirst, iLim;
		char *pst;

		hsttbName = PdodDoc(doc)->hsttbBkmk;

		bkmkRtf.fHaveBkmks = hsttbName != hNil
				&& ((bkmkRtf.ibkmkMac = (*hsttbName)->ibstMac)) > 0;

		if (bkmkRtf.fHaveBkmks)
			{
		/* 1. allocate and fill table of ibst's for bookmark
			end entries
		*/
#ifdef DRTFBKMK
			CommSzNum(SzShared("bkmkRtf.ibkmkmac = "), bkmkRtf.ibkmkMac);
#endif
			/* HEAP MOVEMENT! */
			if ((bkmkRtf.hmpIbklIbst =
					HAllocateCw(sizeof (int) * bkmkRtf.ibkmkMac))
					== hNil)
				{
				ErrorNoMemory(eidNoMemOperation);
				bkmkRtf.fHaveBkmks = fFalse;   /* just don't put out bkmks */
				}

			else
				{
				struct BKF bkf;
				struct PLC **hplcbkf = PdodDoc(doc)->hplcbkf;
				struct PLC **hplcbkl = PdodDoc(doc)->hplcbkl;
				
				Assert (hplcbkf != hNil);
				Assert (hplcbkl != hNil);

			/* 2. walk the hplcbkf and generate table of ibst for
				the hplcbkl, since it does not contain ibst info
			*/

				for (ibkmk = 0; ibkmk < bkmkRtf.ibkmkMac; ibkmk++)
					{
			/* list of strings for bookmark ends */
					GetPlc( hplcbkf, ibkmk, &bkf );
					*(*bkmkRtf.hmpIbklIbst + bkf.ibkl) = ibkmk;
#ifdef DRTFBKMK
					CommSzNum(SzShared("ibst = "), ibkmk);
					CommSzNum(SzShared("ibkl = "), bkf.ibkl);
#endif
					}

				 /* set up the next start or endpoint of a bookmark in the pca.
					The start and end may be for different bookmarks. If starting
					at cp0, we can get the first of each, but convertors may start
					in the middle of things on 2nd tries, and so we want bkmks that
					have terminal points inthe range of the pca.  bz 10/26/89
				 */

				bkmkRtf.icpFirstBkmk =
					IcpFirstBkmk(hplcbkf, pca->cpFirst, &bkmkRtf.cpFirstBkmk);
				bkmkRtf.icpLimBkmk =
					IcpFirstBkmk(hplcbkl, pca->cpFirst, &bkmkRtf.cpLimBkmk);

				if (bkmkRtf.cpFirstBkmk	>= pca->cpLim &&
					bkmkRtf.cpLimBkmk >= pca->cpLim)
						{
						bkmkRtf.fHaveBkmks = fFalse;
						}
				}
			}

		}

	fFacingPages = PdodDoc(doc)->dop.fFacingPages;

	/* Initialize section properties. Need for pgnStart,lineStart doc prop */
	/* don't do it if doc is a header or footnote doc, as when we are
		called recursively in OutHdrs or bad things happen
	*/
	if (!(roo & rooSub))
		{
		Assert (!PdodDoc(doc)->fShort);
		CacheSect(doc, pca->cpFirst);
		}

	if (roo & rooDoc)
		{ /* Write document properties */

		struct STTB **hsttbAssoc = PdodDoc (docMother)->hsttbAssoc;

		PropToRtf(&rebl, &PdodDoc(doc)->dop, irrbDopFirst, irrbDopLim,
				0, FPropToRtfSpecMacPlus);


		/* template is a doc property, but was a ChngDest, so had no
		irrb to use in the PropToRtf loop above.
		*/

		if (hsttbAssoc != hNil)
			{
			char stTemplate[cchMaxFile];
			char *pst = stTemplate;
			int cch;

			/* get template */
			GetStFromSttb(hsttbAssoc, ibstAssocDot, stTemplate);
			if (cch = *pst)
				{
				pchRebl = rebl.rgch;
				*pchRebl++ = chRTFOpenBrk;
				/* ignorable destination */
				*pchRebl++ = chBackslash;
				*pchRebl++ = '*';
				*pchRebl++ = chBackslash;
				pchRebl = PchSzRtfMove(iszRTFtemplate, pchRebl);
				*pchRebl++ = ' ';

				FlushRebl(&rebl, &pchRebl); /* dump keyword; sets pchRebl=rebl.rgch */

				pst++;	/* point to start of data */

			/* Assumption note: since a path name can be no > 120 chars, and
			they can't all be backslashes, the worst case would fill this
			buffer with ~ 240 characters and there is room for 256.
					Since that is the case
			we will only call DchFrom... once
			and Assert that we didn't overfill the buffer
			*/
				FreezeHp();
				cch -= DchTextToPlainText(&pchRebl, &pst, cch,
						sizeof (rebl.rgch) - 1);
				MeltHp();

#ifdef BZTESTX
				CommSzNum(SzShared("cch after template plain text conv = "), cch);
#endif
				Assert (cch == 0); /* all source chars used */

				*pchRebl++ = chRTFCloseBrk;
				FlushRebl(&rebl, &pchRebl);	/* dump data and brace */
				}
			}


		OutHdrs(&rebl, docHdr, 0, IhddFromDocCp(PcaPoint(&caT,doc, cp0), 0, 0),
				PdodDoc(doc)->dop.grpfIhdt, ihdtTFtn, fFacingPages, fFalse, roo);
		}
	/* Initialize paragraph, character properties */
	CachePara(doc, pca->cpFirst);
	if (FInTableVPapFetch(doc, pca->cpFirst))
		{
		CpFirstTap(pca->doc, pca->cpFirst);
		CacheTc(wwNil, pca->doc, pca->cpFirst, fFalse, fFalse);
		fNewTap = fTrue;
		CachePara(pca->doc, pca->cpFirst);
		}
	FetchCp(doc, pca->cpFirst, fcmChars+fcmProps);
	tap.itcMac = itcMax + 1;
	RtfStandardChp(&chp);
	StandardPap(&pap);
/* store impossible stc so that style chp is emitted for first paragraph
	even if its style is Normal. */
	pap.stc = stcStdMin;
	fNewPara = fTrue;
	fNewSect = !(roo & rooSub);

		/* (pj 3/12): all pcode RtfOut takes ~70% of save time!! */
		{{ /* NATIVE - RtfOut */
		while (fTrue)
			{ /* Loop for all text */
			CP cp = vcpFetch;
			char rgbBuf[cbRtfBuf];
			int ccp = (int)CpMin((CP)vccpFetch,
					CpMin(pca->cpLim,
					CpMin(cpNextSea,
					CpMin(cpNextFrd + 1, cpNextAtrd + 1))) - cp);

			ccp = min(ccp, cbRtfBuf);

			if (vmerr.fDiskFail || vmerr.fMemFail)
				return;

		/* are there bookmarks starts or ends in current run? */
			if (bkmkRtf.fHaveBkmks)
				{
				/* CSBUG split up due to "complex" expression */
				CP cpLim = cp + ccp - 1;
				bkmkRtf.fHaveBkmkInRun = FBkmksInRun(cp, cpLim);
				}
			if (cp == cpNextSea && cp < pca->cpLim)
				{{ /* !NATIVE - RtfOut */
				hplcsea = PdodDoc(doc)->hplcsea;
				if (isea < IMacPlc( hplcsea ))
					{
					pchRebl = rebl.rgch;
					*pchRebl++ = chRTFOpenBrk;
					SzToRtf(&pchRebl, iszRTFprivate);
					*pchRebl++ = '1';
					*pchRebl++ = ' ';
					GetPlc( hplcsea, isea++, rgbSea );
					for (pchT = rgbSea, pchLimT = pchT + cbSEA;
							pchT < pchLimT; pchT++)
						{
						int w;
						w = *pchT / 16;
						*pchRebl++ = (w < 10) ? '0' + w : 'a' + w - 10;
						w = *pchT % 16;
						*pchRebl++ = (w < 10) ? '0' + w : 'a' + w - 10;
						}
					*pchRebl++ = chRTFCloseBrk;
					FlushRebl(&rebl, &pchRebl);
					cpNextSea = CpPlc( hplcsea, isea );
						{{
						goto LRefetch;
						}} /* !NATIVE - RtfOut */
					}
				}}

			/* Write section, paragraph properties if necessary */
			if (fNewSect) /* Output section props */
				{{ /* !NATIVE - RtfOut */
				int ihddMin, ihddMac;
				struct CA ca;
				pchRebl = rebl.rgch;
				CacheSect(doc, cp); /* ensure validity of vsepFetch and caSect */

				if (fBracketProp)
					{
					*pchRebl++ = chRTFCloseBrk;
					fBracketProp = fFalse;
					fBracketBroken = fTrue;
					}
				*pchRebl++ = chBackslash;
				pchRebl = PchSzRtfMove(iszRTFsectd, pchRebl);
				*pchRebl++ = ' ';
				FlushRebl(&rebl, &pchRebl);
				PropToRtf(&rebl,  &vsepFetch, irrbSepFirst, irrbSepLim,
						0, FPropToRtfSpecMacPlus);
				Assert (caSect.doc != docNil);
				ca = caSect;
				ihddMac = IhddFromDocCp(&ca, &ihddMin, 0);
				OutHdrs(&rebl, docHdr, ihddMin, ihddMac, vsepFetch.grpfIhdt, ihdtTLeft, fFacingPages,
						vsepFetch.fTitlePage, roo);

				CacheSect(doc, cp);
        		if (FInTableDocCp(doc, cp))
        		 	{
        			CpFirstTap(doc, cp);
        			CacheTc(wwNil, doc, cp, fFalse, fFalse); 
        			fNewTap = fTrue;
        			}
        		CachePara(doc, cp); 
				FetchCp(doc, cp, fcmProps+fcmChars);
                /* blow saved pap so we always emit proper para and char props after section
                    break */
	            pap.stc = stcStdMin;
	            fBracketBroken = fTrue;
				}}

			fNewSect = (!(roo & rooSub) && cp + ccp == caSect.cpLim);

			if (fNewTap && (tap.itcMac != vtapFetch.itcMac ||
					FNeRgw(&tap, &vtapFetch, cwTAPBase) ||
					FNeRgw(tap.rgdxaCenter, vtapFetch.rgdxaCenter, tap.itcMac + 1) ||
					FNeRgch(tap.rgtc, vtapFetch.rgtc, tap.itcMac * sizeof(struct TC)) ||
					(ifldsTop > 0 && !fldsCur.fTableOut)))
				{{ /* !NATIVE - RtfOut */
				/* Write table props */
				if (ifldsTop > 0)
					fldsCur.fTableOut = fTrue;
				pchRebl = rebl.rgch;
				if (fBracketProp)
					{
					*pchRebl++ = chRTFCloseBrk;
					fBracketProp = fFalse;
					fBracketBroken = fTrue;
					}
				*pchRebl++ = chBackslash;
				pchRebl = PchSzRtfMove(iszRTFtrowd, pchRebl);
				*pchRebl++ = ' ';
				FlushRebl(&rebl, &pchRebl);
				PropToRtf(&rebl, &vtapFetch, irrbRTapFirst, irrbRTapLim,
						0, FPropToRtfSpecMacPlus);
				for (itc = 0, ptc = (struct TC *)vtapFetch.rgtc,
						pdxa = &vtapFetch.rgdxaCenter[1];
						itc < vtapFetch.itcMac; itc++, ptc++, pdxa++)
					{
					if (itc == 0 || FNeRgch(ptc, &tc, cbTC))
						{
						pchRebl = rebl.rgch;
						FlushRebl(&rebl, &pchRebl);
						PropToRtf(&rebl, ptc, irrbRTcFirst, irrbRTcLim, 0, 
								FPropToRtfSpecMacPlus);
						}
					pchRebl = rebl.rgch;
					*pchRebl++ = chBackslash;
					pchRebl = PchSzRtfMove(iszRTFcellx, pchRebl);
					CchIntToPpch(*pdxa, &pchRebl);
					FlushRebl(&rebl, &pchRebl);
					}
				*pchRebl++ = ' ';
				tap = vtapFetch;
				}}
			fNewTap = (cp + ccp == caTap.cpLim && caTap.doc == doc);


			if (!(roo & (rooSub|rooInternal)) && fNewPara)
				ReportSavePercent(cp0, pca->cpLim, cp, fFalse);

			if (fNewPara && (FNeRgw(&pap, &vpapFetch, cwPAPBase) ||
					FNeRgw(pap.rgdxaTab, vpapFetch.rgdxaTab, pap.itbdMac) ||
					FNeRgch(pap.rgtbd, vpapFetch.rgtbd, pap.itbdMac)))
				{
				DumpParaProps(&rebl, &pchRebl, &pap, &fBracketProp, &fBracketBroken);
				}

			fNewPara = (cp + ccp == caPara.cpLim);

	/* Write character properties if they have changed */

		/* FSpec note: in Opus, the field delimiters have fSpec, as
			do pictures, footnote chars and tables. Field delimiters
			may have been given attributes different from the surrounding
			text. However, in RtfIn, we only give the instruction and
			result text to the document and let the fieldify routines
			add in the delimiters, with the attributes of the text
			preceding the inst/result text.
		
			For this reason, we ignore ANY attributes attached to a field
			delimiter since the resulting property block would be attached
			to nothing and have no effect.
		
			So, if fDelimField is true, we bypass all the char prop
			output stuff and even handle the BracketProp stuff in
			the character handling code.
		*/

			fSpec = vchpFetch.fSpec;
			vchpFetch.fSpec = fFalse;  /* ignore fSpec for this props comparison */

			fDelimField = fSpec &&
					((ch = *vhpchFetch) == chFieldBegin
					|| ch == chFieldSeparate
					|| ch == chFieldEnd)
					? fTrue : fFalse;


			if (!fDelimField)  /* leave chp as it was if fDelimField */
				{
				if (FNeRgch(&vchpFetch, &vchpStc, cbCHP))
					{
					if (FNeRgch(&chp, &vchpFetch, cbCHP) || fBracketBroken)
						{{ /* !NATIVE - RtfOut */
						pchRebl = rebl.rgch;
						if (fBracketProp)
							{
							*pchRebl++ = chRTFCloseBrk;
							FlushRebl(&rebl, &pchRebl);
							}
						*pchRebl++ = chRTFOpenBrk;
						fBracketProp = fTrue;
						CbGenChpxFromChp(&chpx, &vchpFetch, &vchpStc, fFalse);
						maskSetZero = (*(int *)&chpx) & (*(int *)&vchpStc) &
								(~maskFs);
/* question: are we turning off a toggled property?	*/
						if (maskSetZero == 0)
							{
							pchp = &chpx;
/* if no, and none of the non-toggles are equal to the RTF standard props
	then we can just emit RTF for the exceptions. */
							if (*(int *)pchp & maskFs)
								{
								if ((pchp->fsFtc && pchp->ftc == ftcMapDef) ||
										(pchp->fsHps && pchp->hps == fsDefRTF) ||
										(pchp->fsKul && pchp->kul == 0) ||
										(pchp->fsPos && pchp->hpsPos == 0) ||
										(pchp->fsSpace && pchp->qpsSpace == 0 ))
									goto LBuildFromPlain;
								}
							pfnFEmit = FEmitForChpx;
							}
						else
/* else if yes, we set back to plain and emit RTF for all of the chp props.*/
							{
LBuildFromPlain:
							*pchRebl++ = chBackslash;
							pchRebl = PchSzRtfMove(iszRTFplain, pchRebl);
							*pchRebl++ = ' ';
							pchp = &vchpFetch;
							pfnFEmit = 0;
							}
						FlushRebl(&rebl, &pchRebl);

						if (fldsCur.flt != fltNil && (fldsCur.flt == fltXe || fldsCur.flt == fltTce))
							{
						/* always add in vanished when outputting text in these fields */
							chp = *pchp;
							chp.fVanish = fTrue;
							pchp = &chp;
							}
						PropToRtf(&rebl, pchp, irrbChpFirst, irrbChpLim,
								pfnFEmit, FPropToRtfSpecMacPlus);
						chp = vchpFetch;
						}}
					}

				else  /* same as vchpStc */		
					{
					chp = vchpFetch;
					if (fBracketProp)
						{
						pchRebl = rebl.rgch;
						*pchRebl++ = chRTFCloseBrk;
						FlushRebl(&rebl, &pchRebl);
						fBracketProp = fFalse;
						}
					}
				} /*	else if (!fDelimField) */

			fBracketBroken =  fFalse;
			vchpFetch.fSpec = fSpec;  /* restore */

			Assert (ccp <= cbRtfBuf);
			bltbh(vhpchFetch, rgbBuf, ccp);

			for (pch = pchOut = rgbBuf, ich = 0; ich < ccp; ich++, pch++)
				{
				fPutSpace = fTrue;
				isz = -1;

		/* check for bookmarks if there are any in this run. Write out
			bookmark name if found and update current bkmk pointers */

				if (bkmkRtf.fHaveBkmkInRun)
					{{ /* !NATIVE - RtfOut */
					CP cpCur = cp + ich;

			/* get all bookmark starts or ends that match this CP */
					DumpBkmks(doc, cpCur, &bkmkRtf, &rebl, &pch, &pchOut);
				/* if no more in run, turn off flag */
					/* CSBUG broke out cp + ccp -1 from macro call. Note,
					not actually needed since out of native, but
					protection in case things change
					*/
					cpCur = cp + ccp - 1;
					bkmkRtf.fHaveBkmkInRun = FBkmksInRun(cp, cpCur);
					}}      /* if (bkmkRtf.fHaveBkmkInRun) */


				ch = *pch;
				if (fldsCur.flt != fltNil && (fldsCur.flt == fltXe || fldsCur.flt == fltTce))
					{{ /* !NATIVE - RtfOut */
				/* we skip over all white space in these fields not in quotes
					but white space is a terminator for the \r and \t switches of
					xe.
				*/
					if (!fldsCur.fOpenQuote)
						{
/* Perform FMatchWhiteSpace in line, because this is the
	only non-assembler caller
			if (FMatchWhiteSpace(ch))
*/
						if (ch == chSpace || ch == chNonBreakSpace || ch == chTab)
							{
							if (fldsCur.fInTRSwText)  /* have already encountered text so this ends sw */
								{
								Assert (fldsCur.grpfOpenSw == grpfSwXeR ||  fldsCur.grpfOpenSw == grpfSwXeT);
								FlushReblPch(&rebl, pch, pchOut);
								pchOut = pch + 1;

								pchRebl = rebl.rgch;
								*pchRebl++ = chRTFCloseBrk;
								FlushRebl(&rebl, &pchRebl);

								fldsCur.grpfOpenSw = grpfSwNone;
								fldsCur.fInTRSwText = fFalse;
								}
						/* just skip over these */
							FlushReblPch(&rebl, pch, pchOut);
							pchOut = pch + 1;
			/* a bit more explicit than "continue" */
								{{
								goto EndForLoop;
								}} /* !NATIVE - RtfOut */
							}
						else  /* not white space */						
							{
							if (fldsCur.grpfOpenSw == grpfSwXeR ||
									fldsCur.grpfOpenSw == grpfSwXeT)
								{
								fldsCur.fInTRSwText = fTrue;
								}
							}
						}


					if (fldsCur.grpfOpenSw != grpfSwNone)
						{
						/* processing text following an opened field switch range */
						switch (fldsCur.grpfOpenSw)
							{
						case grpfSwTcL:
							/* just take this char as argument; finished */
							fldsCur.grpfOpenSw = grpfSwNone;
							break;
						case grpfSwTcF:
							fldsCur.grpfOpenSw = grpfSwNone;
							/* turn the character into its ANSI code, dump and continue */
							FlushReblPch(&rebl, pch, pchOut);
							pchOut = pch + 1;
							pchRebl = rebl.rgch;
							CchIntToPpch(ch, &pchRebl);
							FlushRebl(&rebl, &pchRebl);


								/* continue the for (pch = pchOut = rgbbuf ...) loop */
					/* a bit more explicit than "continue" */
								{{
								goto EndForLoop;
								}} /* !NATIVE - RtfOut */

						case grpfSwXeR:
						case grpfSwXeT:
							/* terminate these on a closing quote (not escaped) or
								whitespace if no opening quote. Put out a closing
								brace, then process the character.
								Note the quote processing below
								will take care of setting the closing switch
							*/
							if (ch == chDQuote && fldsCur.fOpenQuote && !fHadSlashFld)
								{
								FlushReblPch(&rebl, pch, pchOut);
								pchOut = pch + 1;
								pchRebl = rebl.rgch;
								*pchRebl++ = chRTFCloseBrk;
								FlushRebl(&rebl, &pchRebl);

								fldsCur.grpfOpenSw = grpfSwNone;
								fldsCur.fInTRSwText = fFalse;
								}
							break;

#ifdef DEBUG
						default:
							Assert (fFalse);
#endif /* DEBUG */
							}
						}

					if (fHadSlashFld)
						{
						fHadSlashFld = fFalse;
						Assert(fldsCur.flt == fltXe || fldsCur.flt == fltTce);
						if (fldsCur.flt == fltXe)
							switch (ch)
								{
							default:
					/* had a \. Put back and also dump ch */
								FlushReblPch(&rebl, pch, pchOut);
								pchOut = pch + 1;
								pchRebl = rebl.rgch;
								*pchRebl++ = chBackslash;
								*pchRebl++ = ch;
								FlushRebl(&rebl, &pchRebl);
								break;

							/* these go out w/o \ */
							case chDQuote:
							case chColon:
								FlushReblPch(&rebl, pch, pchOut);
								pchOut = pch + 1;
								pchRebl = rebl.rgch;
								*pchRebl++ = ch;
								FlushRebl(&rebl, &pchRebl);
								break;

							case chBackslash:
								FlushReblPch(&rebl, pch, pchOut);
								pchOut = pch + 1;
								pchRebl = rebl.rgch;
								/* // -> //// */
								*pchRebl++ = ch;
								*pchRebl++ = ch;
								*pchRebl++ = ch;
								*pchRebl++ = ch;
								FlushRebl(&rebl, &pchRebl);
								break;

							case chFldXeBold:
								isz = iszRTFbxe;
								break;
							case chFldXeItalic:
								isz = iszRTFixe;
								break;
							case chFldXeText:
							case chFldXeRange:
								/* precede \txe or \rxe with opnbrk */
								FlushReblPch(&rebl, pch, pchOut);
								pchOut = pch + 1;
								pchRebl = rebl.rgch;
								if (fBracketProp)
									{
									*pchRebl++ = chRTFCloseBrk;
									fBracketProp = fFalse;
									}
								*pchRebl++ = chRTFOpenBrk;
								FlushRebl(&rebl, &pchRebl);
								if (ch == chFldXeText)
									{
									isz = iszRTFtxe;
									fldsCur.grpfOpenSw = grpfSwXeT;
									}
								else
									{
									isz = iszRTFrxe;
									fldsCur.grpfOpenSw = grpfSwXeR;
									}
								break;
								}

						else
							switch (ch)
								{
							default:
					/* had a \. Put back and also dump ch */
								FlushReblPch(&rebl, pch, pchOut);
								pchOut = pch + 1;
								pchRebl = rebl.rgch;
								*pchRebl++ = chBackslash;
								*pchRebl++ = ch;
								FlushRebl(&rebl, &pchRebl);
								break;

							/* these go out w/o \ */
							case chDQuote:
							case chColon:
								FlushReblPch(&rebl, pch, pchOut);
								pchOut = pch + 1;
								pchRebl = rebl.rgch;
								*pchRebl++ = ch;
								FlushRebl(&rebl, &pchRebl);
								break;

							case chBackslash:
								FlushReblPch(&rebl, pch, pchOut);
								pchOut = pch + 1;
								pchRebl = rebl.rgch;
								/* // -> //// */
								*pchRebl++ = ch;
								*pchRebl++ = ch;
								*pchRebl++ = ch;
								*pchRebl++ = ch;
								FlushRebl(&rebl, &pchRebl);
								break;

							case chFldTceTableId:
								isz = iszRTFtcf;
								fldsCur.grpfOpenSw = grpfSwTcF;
								fPutSpace = fFalse;
								break;

							case chFldTceLevel:
								isz = iszRTFtcl;
								fldsCur.grpfOpenSw = grpfSwTcL;
								fPutSpace = fFalse;
								break;
								}
						}
					else  /* !fHadSlashFld */					
						{
						switch (ch)
							{
						default:
								{{
								goto LNonFldSpecCh;
								}} /* !NATIVE - RtfOut */

						case chDQuote:
							/* skip these; they just delimit text */
							/* closing of field swithces already done */
							FlushReblPch(&rebl, pch, pchOut);
							pchOut = pch + 1;
							fldsCur.fOpenQuote = ~fldsCur.fOpenQuote;
							break;
						case chBackslash:
							fHadSlashFld = fTrue; /* skip over but set flag */
							FlushReblPch(&rebl, pch, pchOut);
							pchOut = pch + 1;
							break;
						case chColon:
							/* dump all to now */
							FlushReblPch(&rebl, pch, pchOut);
							pchOut = pch + 1;
							pchRebl = rebl.rgch;
							/* : -> \: */
							*pchRebl++ = chBackslash;
							*pchRebl++ = chColon;
							FlushRebl(&rebl, &pchRebl);
							break;
							} /* switch */
						}
					}}
				else  /* not in xe/tce fields */				
					{
LNonFldSpecCh:
					switch (ch)
						{
					case chTab:
						isz = iszRTFtab;  /* even if internal for OPus (ebcdic convertors! */
						break;
#ifdef CRLF
					case chReturn:
			/* We want to throw out CR chars due to problem of
				splitting CRLF pairs when EnsureRtfChEop is
				called. We dump out the text here and skip over
				the CR, then when we get a LF we put out both
				CR and LF
			*/
						FlushReblPch(&rebl, pch, pchOut);
						pchOut = pch + 1;
						break;
#endif
					case chEop:
						isz = iszRTFpar;

#ifdef WIN
						if (fldsCur.flt != fltNil)
							fldsCur.fParInFld = fTrue;   /* para encountered while in field */
#endif /* WIN */
						break;

					case chCRJ:
						isz = iszRTFline;
						break;
					case chTable:
							{{ /* !NATIVE - RtfOut */
#ifdef MACONLY	 /* opus uses different character mapping */
				/* when fSpec, ch is a chTFtnCont */
							if (vchpFetch.fSpec)
								goto LFSpec;
#endif /* MACONLY */
							if (!vpapFetch.fInTable)
								{{
								goto LDefault;
								}} /* !NATIVE - RtfOut */
					/* note bz: 1 is intentional, not ccpEop since we
						skip over the CR of the table marker, and are sitting
						at the chTable char here.
					*/
                            Assert (caTap.doc == doc);
                            Assert (vtcc.doc == doc);
							if (cp + ich + 1 == caTap.cpLim)
								isz = iszRTFrow;
							else  if (cp + ich + 1 == vtcc.cpLim)
								isz = iszRTFcell;
							else
								{{
								goto LDefault;
								}} /* !NATIVE - RtfOut */
							fNewTc = fTrue;
							}} /* end !NATIVE */
						break;

#ifdef MACONLY	 /* opus uses fields for these */
					case chFormula:
						isz = iszRTFFormula;
						fPutSpace = fFalse;
						break;
#endif /* MACONLY */

					case chNonReqHyphen:
						isz = iszRTFHyphen;
						fPutSpace = fFalse;
						break;
					case chNonBreakSpace:
						isz = iszRTFTilde;
						fPutSpace = fFalse;
						break;
					case chNonBreakHyphen:
						isz = iszRTFUnderscore;
						fPutSpace = fFalse;
						break;
					case chBackslash:
						isz = iszRTFBackslash;
						fPutSpace = fFalse;
						break;
					case chRTFOpenBrk:
						isz = iszRTFLBracket;
						fPutSpace = fFalse;
						break;
					case chRTFCloseBrk:
						isz = iszRTFRBracket;
						fPutSpace = fFalse;
						break;
					case chSect:
						if (ich + 1 == ccp)
							{
							if (!fNewSect)
								isz = iszRTFpage;
							else
								isz = iszRTFsect;
							}
						else
							isz = iszRTFpage;
						break;

					case chColumnBreak:
						isz = iszRTFcolumn;
						break;

					case chFieldBegin:
					case chFieldSeparate:
					case chFieldEnd:
						if (fSpec)
							{{ /* !NATIVE - RtfOut */
			/* clear out anything up to this point */
							FlushReblPch(&rebl, pch, pchOut);
			/* bypass character for which rtf has been dumped */
							pchOut = pch + 1;

							pchRebl = rebl.rgch;
				/* we do this here instead of in the char prop
				code in case we have other special chars with
				the same props as a field delimiter, and then
				we would not be properly set up.
				*/

							if (fBracketProp)
								{
								*pchRebl++ = chRTFCloseBrk;
								FlushRebl(&rebl, &pchRebl);
								fBracketProp = fFalse;
								fBracketBroken = fTrue;
								}

							switch (ch)
								{
							case chFieldBegin:
									{
									int ifld;
									int flt;
									struct FFB ffb;
									struct CHP chpFld;
									struct FLCD flcd;
									CP cpCur = cp + ich;

				/* write out any relevant field flags */
				/* note that edit implies dirty (we assert
					this) so only write out dirty if edit
					is false. On input, reading edit sets
					both edit and dirty.
				*/

									ifld = IfldFromDocCp (doc, cpCur, fTrue);

									if (ifld == ifldNil)
										{
							/* have to do this twice because FltParseDocCP may resurrect a
								dead field and so change ifld.
							*/
										flt = FltParseDocCp (doc, cpCur, ifldNil,
												 fFalse /* fChgView */, fFalse /* fEnglish */);
										ifld = IfldFromDocCp (doc, cpCur, fTrue);
								/* since FltParse... calls fetchcp, set this so
								that either resurrected fields or non xe/tc
								dead fields will make the next real fetchcp correctly.
								*/
										cpFetchAdjusted  = cpCur + 1;
										}

									if (ifld == ifldNil)
										{
								/* Note: this will call FetchCp and thus trash
									the saved variables
								*/
										flt = FltParseDocCp (doc, cpCur, ifldNil,
												 fFalse /* fChgView */, fFalse /* fEnglish */);
										}
									else
										{
											/* field is LIVE */
										GetIfldFlcd (doc, ifld, &flcd);
										flt = flcd.flt;
										}

							/* save previous field state and set new one */
									fldsCur = FldsStackFlt(fldsCur, flt, rgfltStack,
											&ifldsTop);
#ifdef XBZ
									CommSzNum(SzShared("fldsCur after push "), fldsCur);
#endif

									if (flt == fltXe || flt == fltTce)
										{
										CP cpFirst, cpLim;

										stateRdsField = rdsXe;  /* just has to be != rdsField */

								/* get char props of 1st char of kwd to use
								as props of the keyword - always vanished
								for xe and tc fields. Other dead fields will
								start fetching at the next char like usual but have to set
								cpFetchAdjusted to avoid hassles with comparisons to vccpFetch.    if
								*/

										SetFfbDeadFlt (&ffb, doc, cpCur,
												flt);
										FetchCpAndPara(doc, ffb.cpFirst,
												fcmProps);
										chpFld = vchpFetch;
										chpFld.fVanish = fTrue;

											/* emit <opnbrk>\pard\plain <para and char props>  */
										*pchRebl++ = chRTFOpenBrk;
										*pchRebl++ = chBackslash;
										pchRebl = PchSzRtfMove(iszRTFpard, pchRebl);
										*pchRebl++ = chBackslash;
										pchRebl = PchSzRtfMove(iszRTFplain, pchRebl);
										*pchRebl++ = chSpace;
										FlushRebl(&rebl, &pchRebl);
										PropToRtf(&rebl, &vpapFetch, irrbPapFirst,
												 irrbPapLim,
												0, FPropToRtfSpecMacPlus);
										PropToRtf(&rebl, &chpFld, irrbChpFirst, irrbChpLim,
												0, FPropToRtfSpecMacPlus);
											/* emit <opnbrk>\xe|tc<sp> */
										*pchRebl++ = chRTFOpenBrk;
										*pchRebl++ = chBackslash;
										pchRebl = PchSzRtfMove(flt == fltXe ?
												iszRTFxe : iszRTFtc, pchRebl);
										*pchRebl++ = chSpace;

										  /* skip over keyword */
										AssertDo (FFetchArgExtents (&ffb,
												 &cpFirst, &cpLim, fFalse));
									/* next real FetchCp will start after the field kwd */
										cpFetchAdjusted  = cpLim;

										}
									else
										{
										/* this lets us write out \fldrslt when there
											is no field separator, e.g., for uncalculated
											fields.
										*/	 
										stateRdsField = rdsField;

				/* emit <opnbrk>\field[\flags]<opnbrk>\fldinst<sp> */

										*pchRebl++ = chRTFOpenBrk;

										*pchRebl++ = chBackslash;
										pchRebl = PchSzRtfMove(iszRTFfield, pchRebl);

										if (ifld != ifldNil)
											{   /* field is LIVE */

											if (flcd.fResultEdited)
												{
												*pchRebl++ = chBackslash;
												pchRebl = PchSzRtfMove(iszRTFfldedit,
														 pchRebl);
												Assert(flcd.fResultDirty);
												}
											else  if (flcd.fResultDirty)
												{
												*pchRebl++ = chBackslash;
												pchRebl = PchSzRtfMove(iszRTFflddirty,
														 pchRebl);
												}

											if (flcd.fLocked)
												{
												*pchRebl++ = chBackslash;
												pchRebl = PchSzRtfMove(iszRTFfldlock,
														 pchRebl);
												}

											if (flcd.fPrivateResult)
												{
												*pchRebl++ = chBackslash;
												pchRebl = PchSzRtfMove(iszRTFfldpriv,
														 pchRebl);
												}
											}

										*pchRebl++ = chRTFOpenBrk;
								/* ignorable destination */
										*pchRebl++ = chBackslash;
										*pchRebl++ = '*';
										*pchRebl++ = chBackslash;
										pchRebl = PchSzRtfMove(iszRTFfldinst, pchRebl);
										*pchRebl++ = chSpace;
										 /* ref field w/o kwd? emit kwd */
										if (flt == fltPosBkmk)
											{
											int cch;
											if ((cch = CchSzFromSzgId (stName, szgFltNormal, fltBkmkRef,
												cchMaxFieldKeyword)) > 1)
												{
												QszLower(stName);  /* table is upper case */
												pchRebl = bltbyte(stName, pchRebl, cch - 1);
												*pchRebl++ = chSpace;
												}
											}
										}

									FlushRebl(&rebl, &pchRebl);
									break;
									}
							case chFieldSeparate:
									{
									int ifld;
									struct FLCD flcd;

			/* this keeps us from writing out an extra
				\fldrslt at close time
			*/
									stateRdsField = rdsFldRslt;

				/* emit <clsbrk><opnbrk>\fldrslt<sp> */

									*pchRebl++ = chRTFCloseBrk;
									*pchRebl++ = chRTFOpenBrk;
									*pchRebl++ = chBackslash;
									pchRebl = PchSzRtfMove(iszRTFfldrslt, pchRebl);
									*pchRebl++ = chSpace;

				/* check for private result special handling */

									ifld = IfldFromDocCp (doc, (CP)cp + ich, fTrue);
									Assert (ifld != ifldNil);
									GetIfldFlcd (doc, ifld, &flcd);

									if (flcd.fPrivateResult)
										{
					/* add code if we ever have these
					to grab all the binary chars in a loop like the
					picture handling and spit them out. For now,
					assume no private result special handling needed
								and just output the chars as normal, or inhibit
								the actually result.
									
								The correct way to do this would be to loop until
								you get the chFieldEnd character, and throw away
								any intervening characters you get, thus disposing the
								private result. That would involve some playing with
								the input buffer and setting up resumable states.
								For now (7/12/88) the only private result is a tiff
								file picture char, and we dump that during the chPicture
								handling. If we get any more we will have to generalize
								this and do it right. bz
					*/
										}

									FlushRebl(&rebl, &pchRebl);
									break;
									}
							case chFieldEnd:
									{

						/* if we never encountered a field separator,
						write out an empty \fldrslt so rtfin
						processing will be more consistent.
						*/
									if (stateRdsField == rdsField)
					/* emit <clsbrk><opnbrk>\fldrslt<sp> */
										{
										*pchRebl++ = chRTFCloseBrk;
										*pchRebl++ = chRTFOpenBrk;
										*pchRebl++ = chBackslash;
										pchRebl = PchSzRtfMove(iszRTFfldrslt, pchRebl);
										*pchRebl++ = chSpace;
										}

									stateRdsField = rdsMain;  /* default */

				/* emit <clsbrk><clsbrk> */

									*pchRebl++ = chRTFCloseBrk;
									*pchRebl++ = chRTFCloseBrk;

									FlushRebl(&rebl, &pchRebl);

									Assert (fldsCur.flt != fltNil);
									if (fldsCur.fParInFld)
										{
							/* force para, char props to reemit */
							/* get props for cp following end of field */
										FetchCpAndPara(doc, cp + ich + 1, fcmProps);
										StandardPap(&papT);  /* force props */
										papT.stc = stcStdMin;
										DumpParaProps(&rebl, &pchRebl, &papT,
												&fBracketProp, &fBracketBroken);
								/* set global state back to where it was */
										FetchCpAndPara(doc, cp, fcmProps);
										}
							/* pop back the field state */
									fldsCur = FldsPopFlt(rgfltStack, &ifldsTop);
#ifdef XBZ
									CommSzNum(SzShared("fldsCur after pop "), fldsCur);
#endif
									break;
									}
								}  /*  switch (ch)  */

							}}	   /* if (fSpec)    */
						break;
					case chFootnote:
					case chTFtn:
					case chTFtnCont:
					case chAtn:
					case chPicture:
						if (fSpec)
							{{ /* !NATIVE - RtfOut */
LFSpec:
							switch (ch)
								{
			/* no default here since we filter above */

							case chFootnote:
								isz = iszRTFchftn;
								break;

							case chTFtn:
								isz = iszRTFchftnsep;
								break;

							case chTFtnCont:
								isz = iszRTFchftnsepc;
								break;

							case chAtn:
									{
					/* clear out anything up to this point */
								   	FlushReblPch(&rebl, pch, pchOut);
			/* bypass character for which rtf has been dumped */
									pchOut = pch + 1;

									pchRebl = rebl.rgch;

				/* if we have user initials, generate them first */
									if (hplcatrd != 0)
										{
										struct ATRD atrd;

										Assert (cp + ich  == cpNextAtrd);

				/* here output {\atnid xxx} */
										*pchRebl++ = chRTFOpenBrk;
								/* ignorable destination */
										*pchRebl++ = chBackslash;
										*pchRebl++ = '*';
										*pchRebl++ = chBackslash;
										pchRebl = PchSzRtfMove(iszRTFatnid, pchRebl);
										*pchRebl++ = chSpace;

										GetPlc( hplcatrd, iatrd, &atrd );
										pchRebl = bltbyte(&atrd.stUsrInitl[1],
												pchRebl, atrd.stUsrInitl[0]);

										*pchRebl++ = chRTFCloseBrk;
										FlushRebl(&rebl, &pchRebl);
										}

				/* now dump the chatn keyword */

				/* pchRebl unchanged
					or flushrebl reset pchRebl to rebl.rgch */
									*pchRebl++ = chBackslash;
									pchRebl = PchSzRtfMove(iszRTFchatn, pchRebl);
									*pchRebl++ = chSpace;
									FlushRebl(&rebl, &pchRebl);

									break;
									}
							case chPicture:
								FlushReblPch(&rebl, pch, pchOut);
								pchOut = pch + 1;
#ifdef MAC
/* skip chPicture */
								if (vfRTFInternal && viConvertType == iIsIBM)
									break;
#endif
								if (roo & rooNoPict)
									break;

			/* Note that FetchPe will set vchpFetch.fnPic to fnFetch */

								FetchPe(&vchpFetch, doc, (CP)cp + ich);

									/* tiff or old format metafile last result disappears bz */
								if (vpicFetch.mfp.mm == MM_TIFF ||
				    					(vpicFetch.mfp.mm < MM_META_MAX &&
		               					!FEmptyRc(&vpicFetch.rcWinMF)))
									{
									break;
									}
								pchRebl = rebl.rgch;
								*pchRebl++ = chRTFOpenBrk;

								*pchRebl++ = chBackslash;
								pchRebl = PchSzRtfMove(iszRTFpict, pchRebl);
								*pchRebl++ = chBackslash;

								if (vpicFetch.mfp.mm == MM_BITMAP)
									{
					/* \wbitmapNNN\picwNNN\pichNNN */
									pchRebl = PchSzRtfMove(iszRTFwbitmap, pchRebl);
									CchIntToPpch(vpicFetch.bm.bmType, &pchRebl);
									*pchRebl++ = chBackslash;
									pchRebl = PchSzRtfMove(iszRTFpicw, pchRebl);
									CchIntToPpch(vpicFetch.bm.bmWidth, &pchRebl);
									*pchRebl++ = chBackslash;
									pchRebl = PchSzRtfMove(iszRTFpich, pchRebl);
									CchIntToPpch(vpicFetch.bm.bmHeight, &pchRebl);
				/* \wbmbitspixelNNN\wbmPlanesNNN\wbmwidthbytesNNN */
									*pchRebl++ = chBackslash;
									pchRebl = PchSzRtfMove(iszRTFwbmbitspixel, pchRebl);
									CchIntToPpch((int)vpicFetch.bm.bmBitsPixel, &pchRebl);
									*pchRebl++ = chBackslash;
									pchRebl = PchSzRtfMove(iszRTFwbmplanes, pchRebl);
									CchIntToPpch((int)vpicFetch.bm.bmPlanes, &pchRebl);
									*pchRebl++ = chBackslash;
									pchRebl = PchSzRtfMove(iszRTFwbmwidthbytes, pchRebl);
									CchIntToPpch(vpicFetch.bm.bmWidthBytes, &pchRebl);
									}
								else		/* metafile */				
									{  /* \wmetafileNNN\picwNNN\pichNNN */
									Assert (vpicFetch.mfp.mm < MM_META_MAX);
									pchRebl = PchSzRtfMove(iszRTFwmetafile, pchRebl);
									CchIntToPpch(vpicFetch.mfp.mm, &pchRebl);
									*pchRebl++ = chBackslash;
									pchRebl = PchSzRtfMove(iszRTFpicw, pchRebl);
									CchIntToPpch(vpicFetch.mfp.xExt, &pchRebl);
									*pchRebl++ = chBackslash;
									pchRebl = PchSzRtfMove(iszRTFpich, pchRebl);
									CchIntToPpch(vpicFetch.mfp.yExt, &pchRebl);
									}

								FlushRebl(&rebl, &pchRebl);

					/* goal sizes */

								*pchRebl++ = chBackslash;
								pchRebl = PchSzRtfMove(iszRTFpicwGoal, pchRebl);
								CchIntToPpch(vpicFetch.dxaGoal, &pchRebl);

								*pchRebl++ = chBackslash;
								pchRebl = PchSzRtfMove(iszRTFpichGoal, pchRebl);
								CchIntToPpch(vpicFetch.dyaGoal, &pchRebl);

					/* border, scaling and cropping */

								if (vpicFetch.brcl != brclNone)
									{
									*pchRebl++ = chBackslash;
									pchRebl = PchSzRtfMove(mpbrclisz[vpicFetch.brcl], pchRebl);
									}

								if (vpicFetch.mx != mx100Pct)
									{
									*pchRebl++ = chBackslash;
									pchRebl = PchSzRtfMove(iszRTFpicscalex, pchRebl);
									CchIntToPpch(vpicFetch.mx / PctTomx100Pct, &pchRebl);
									}

								if (vpicFetch.my != my100Pct)
									{
									*pchRebl++ = chBackslash;
									pchRebl = PchSzRtfMove(iszRTFpicscaley, pchRebl);
									CchIntToPpch(vpicFetch.my / PctTomy100Pct, &pchRebl);
									}

								if (vpicFetch.dxaCropLeft)
									{
									*pchRebl++ = chBackslash;
									pchRebl = PchSzRtfMove(iszRTFpiccropl, pchRebl);
									CchIntToPpch(vpicFetch.dxaCropLeft, &pchRebl);
									}

								if (vpicFetch.dyaCropTop)
									{
									*pchRebl++ = chBackslash;
									pchRebl = PchSzRtfMove(iszRTFpiccropt, pchRebl);
									CchIntToPpch(vpicFetch.dyaCropTop, &pchRebl);
									}

								if (vpicFetch.dxaCropRight)
									{
									*pchRebl++ = chBackslash;
									pchRebl = PchSzRtfMove(iszRTFpiccropr, pchRebl);
									CchIntToPpch(vpicFetch.dxaCropRight, &pchRebl);
									}

								if (vpicFetch.dyaCropBottom)
									{
									*pchRebl++ = chBackslash;
									pchRebl = PchSzRtfMove(iszRTFpiccropb, pchRebl);
									CchIntToPpch(vpicFetch.dyaCropBottom, &pchRebl);
									}

								*pchRebl++ = ' ';
								FlushRebl(&rebl, &pchRebl);

								cfcPicT = vpicFetch.lcb - vpicFetch.cbHeader;
								if (cfcPicT > 0L)
									{
									fcPic = vfcFetchPic;

									if (vfRTFInternal)
										{
										*pchRebl++ = chBackslash;
										pchRebl = PchSzRtfMove(iszRTFbin, pchRebl);
										CchLongToPpch(cfcPicT, &pchRebl);
										*pchRebl++ = ' ';
										FlushRebl(&rebl, &pchRebl);
										while (cfcPicT > 0)
											{
											cfcCur = CpMin(cfcPicT, (CP) sizeof(rebl.rgch));
											FetchPeData(fcPic,
													(char HUGE *)rebl.rgch, cfcCur);
											pchRebl = rebl.rgch + (int) cfcCur;
											fcPic += cfcCur;
											cfcPicT -= cfcCur;
											FlushRebl(&rebl, &pchRebl);
											}
										}
									else
										{
										char rgb[sizeof(rebl.rgch)/2];
										while (cfcPicT > 0)
											{
											int i;
											char *pchT;

											cfcCur = CpMin(cfcPicT, (CP) (sizeof(rebl.rgch) / 2));
											FetchPeData(fcPic,
													(char HUGE *)rgb, cfcCur);
											for (i = 0, pchT = rgb; i < cfcCur; i++,
													 pchT++)
												{
												int w;
												w = *pchT / 16;
												*pchRebl++ = (w < 10) ? '0' + w : 'a' + w - 10;
												w = *pchT % 16;
												*pchRebl++ = (w < 10) ? '0' + w : 'a' + w - 10;
												}
											fcPic += cfcCur;
											cfcPicT -= cfcCur;
											FlushRebl(&rebl, &pchRebl);
											}
										}
									}
								*pchRebl++ = chRTFCloseBrk;
								FlushRebl(&rebl, &pchRebl);
								break;
								}  /*  switch (ch)  */

								{{
								break;
								}} /* !NATIVE - RtfOut */
							}}	   /* if (fSpec)    */
				/* fall thru for errant non fspec control chars */
					default:
LDefault:
						if (!vfRTFInternal

#ifdef CRLF
								&& ch != chReturn
#endif /* CRLF */

								&& (ch < 0x20 || ch > 0x7f))
							{
							isz = iszRTFQuote;
							fPutSpace = fFalse;
							}
						break;
						} /* switch (ch = *pch) */
					}   /* else not in xe/tce fields */

EndForLoop:

				if (isz != -1)
					{
		/* Write characters */
					FlushReblPch(&rebl, pch, pchOut);
			/* bypass character for which rtf has been dumped */
					pchOut = pch + 1;

					pchRebl = rebl.rgch;
		/* put out chEop's at the end of
			paragraphs so that the rtf is
			split up for better scrolling
			as a text file in Word */
					if (isz == iszRTFpar)
						{
#ifdef CRLF
				/* we ate previous CR chars, so add them back in here */
						*pchRebl++ = chReturn;
#endif
						*pchRebl++ = chEop;
						}
					*pchRebl++ = chBackslash;
					pchRebl = PchSzRtfMove(isz, pchRebl);
					if (isz == iszRTFQuote)
						{
						int w;
						w = *pch / 16;
						*pchRebl++ = (w < 10) ? '0' + w : 'a' + w - 10;
						w = *pch % 16;
						*pchRebl++ = (w < 10) ? '0' + w : 'a' + w - 10;
						}
					if (fPutSpace)
						*pchRebl++ = ' ';
					FlushRebl(&rebl, &pchRebl);
					if (isz == iszRTFpar)
						vcchEopRtf = 0;
					}
				}	/* for (pch = pchOut = rgbbuf ...) loop */


	/* Write characters */
			FlushReblPch(&rebl, pch, pchOut);
			pchOut = pch;  /* not + 1; dumping normal chars, not skipping */
			Assert (doc == pca->doc);

			if (cp + ccp == cpNextFrd + 1)
				{{ /* !NATIVE - RtfOut */
				/* Output footnote instead */
				struct CA ca;

				pchRebl = rebl.rgch;
				CaFromIhdd(docFtn, ifrd++, &ca);
				cpNextFrd = CpPlc( hplcfrd, ifrd );
				*pchRebl++ = chRTFOpenBrk;
				*pchRebl++ = chBackslash;
				pchRebl = PchSzRtfMove(iszRTFfootnote, pchRebl);
				goto DumpFtnAtn;
				}}
			else  if (cp + ccp == cpNextAtrd + 1)
				{{ /* !NATIVE - RtfOut */
				/* Output annotation instead */
				struct CA ca;

				pchRebl = rebl.rgch;
				CaFromIhdd(docAtn, iatrd++, &ca);
				cpNextAtrd = CpPlc( hplcatrd, iatrd );
				*pchRebl++ = chRTFOpenBrk;
				/* ignorable destination */
				*pchRebl++ = chBackslash;
				*pchRebl++ = '*';
				*pchRebl++ = chBackslash;
				pchRebl = PchSzRtfMove(iszRTFannotation, pchRebl);
DumpFtnAtn:
				*pchRebl++ = ' ';
				FlushRebl(&rebl, &pchRebl);
				RtfOut(&ca, pfnWrite, pArgWrite, (roo|rooSub)&roomSub);
				*pchRebl++ = chRTFCloseBrk;
				FlushRebl(&rebl, &pchRebl);
				CacheSect(doc, cp);
				CachePara(doc, cp);
				if (vpapFetch.fInTable)
					{
					CpFirstTap(doc, cp);
					CacheTc(wwNil, doc, cp, fFalse, fFalse);
					CachePara(doc, cp);
					}
				}}

			if (cp + ccp >= pca->cpLim)
				{
				if (fNewSect && cp + ccp < CpMacDoc(doc))
					{
					pchRebl = rebl.rgch;
					if (fBracketProp)
						{
						*pchRebl++ = chRTFCloseBrk;
						fBracketProp = fFalse;
						}
					*pchRebl++ = chBackslash;
					pchRebl = PchSzRtfMove(iszRTFsectd, pchRebl);
					*pchRebl++ = ' ';
					FlushRebl(&rebl, &pchRebl);
					}
				break;    /* Note: this is the big out of main loop break! */
				}
#ifdef DEBUG
			NatDummy();
#endif /* DEBUG */
			if (fNewSect)
				CacheSect(doc, caSect.cpLim);
			if (fNewPara)
				{
				int fInTablePrev = vpapFetch.fInTable;
				CachePara(doc, caPara.cpLim);
				cpFirstPara = caPara.cpFirst;
				if (!vpapFetch.fInTable)
					fNewTap = fNewTc = fFalse;
				else  if (!fInTablePrev)
					fNewTap = fNewTc = fTrue;
				}
			if (fNewTap)
				CpFirstTap(doc, cpFirstPara);
			if (fNewTc)
				{
				CacheTc(wwNil, doc, cpFirstPara, fFalse, fFalse);
				fNewTc = fFalse;
				}
			if (fNewTap || fNewTc)
				CachePara(doc, cpFirstPara);

			if (cpFetchAdjusted != cpNil) /* special for skipping field kwds */
				{
				FetchCpAndPara(doc, cpFetchAdjusted, fcmChars+fcmProps);
				cpFetchAdjusted = cpNil;
				}
			else
LRefetch:
				FetchCpAndPara(doc, cp + ccp, fcmChars+fcmProps);
			}
		}}
	if (fBracketProp)
		{
		if (!vfRTFInternal)
			EnsureRtfChEop(&rebl, 1);
		rebl.rgch[0] = chRTFCloseBrk;
		(*pfnWrite)(pArgWrite, rebl.rgch, 1);
		}
	if (!(roo & rooSub))
		{
		/* pick up any bkmks that end at cpMacDocEdit */
		if (bkmkRtf.fHaveBkmks)
			if (bkmkRtf.cpLimBkmk == pca->cpLim)
				{
				DumpBkmks(pca->doc, pca->cpLim, &bkmkRtf, &rebl, &pch, &pchOut);
				}
		if (!vfRTFInternal)
			EnsureRtfChEop(&rebl, 1);
		if (roo & rooEnd)
			{
			rebl.rgch[0] = chRTFCloseBrk;
			(*pfnWrite)(pArgWrite, rebl.rgch, 1);
			}
		if (!(roo & rooInternal))
			/* 100% */
			ReportSavePercent (cp0, 1L, 1L, fFalse);
		}

	if (bkmkRtf.fHaveBkmks)
		{
		Assert(bkmkRtf.hmpIbklIbst != hNil);
		FreeH(bkmkRtf.hmpIbklIbst);
		}
}


/*  %%Function:  IcpFirstBkmk  %%Owner:  bobz       */
/* returns the i of the first bookmark start or end that begins >= cpFirst */
IcpFirstBkmk(hplc, cpFirst, pcp)
struct PLC **hplc;
CP cpFirst;
CP *pcp;
{
	int iFirst;

	/* returns i whose cp is <= cpFirst. If not found, thus -1, start at 0 */

	iFirst = max (IInPlcCheck(hplc, cpFirst), 0);
	/* if no entry >= cpFirst, we will end up with iFirst == iMac for the plc */
	while (CpPlc(hplc, iFirst) < cpFirst)
		iFirst++;

	*pcp = CpPlc(hplc, iFirst);

#ifdef BZ
	CommSzNum(SzShared("IcpFirstBkmk iFirst = "), iFirst);
	CommSzLong(SzShared("IcpFirstBkmk cp = "), *pcp);
#endif /* BZ */

	Assert (iFirst <= IMacPlc(hplc));

	return(iFirst);
}

/* overcome slm bogosities */
#include "rtfout2.c"

