#define NONCMESSAGES
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
#include "props.h"
#include "border.h"
#include "disp.h"
#include "debug.h"
#include "error.h"
#include "prm.h"
#include "sel.h"
#include "file.h"
#include "layout.h"
#include "ch.h"
#include "fontwin.h"

#define REVMARKING
#include "compare.h"

#include "inter.h" 
#include "pic.h"

#define RTFDEFS
#include "rtf.h"

#include "field.h"
#include "strtbl.h"
#define RTFTABLES
#include "rtftbl.h"
#include "style.h"
#include "fkp.h"
#include "prompt.h"
#include "message.h"
#include "table.h"
#include "screen.h"


extern struct CHP	vchpNormal;
extern struct PAP	vpapFetch;
extern struct CHP	vchpFetch;
extern int		fnFetch;

extern struct CA	caPara;
extern struct CA	caSect;
extern struct MERR	vmerr;
extern CP		vcpFetch;
extern int		vccpFetch;
extern char HUGE        *vhpchFetch;
extern struct FKPD	vfkpdText;
extern struct SAB       vsab;
extern struct SEP	vsepFetch;
extern struct CHP	vchpStc;
extern struct ESPRM	dnsprm[];

extern int vstcBackup;
extern int              cfRTF;
extern struct SCI vsci;


long   	LFromSzNumber();

extern CHAR  rgchEop[];
extern CHAR  rgchTable[];

#ifdef DEBUG
extern struct DBS	vdbs;
#endif /* DEBUG */

#ifdef DEBUG
#ifdef PCJ
/* #define DRTFDTTM */
#endif /* PCJ */
#endif /* DEBUG */


#ifdef NOASM
char *PchSzRtfMove();
#endif /* NOASM */

extern struct DTR dtrRTF;  /* date time record */

csconst char rgchRTF[] = {
	'{', '\\','r','t','f'};


/*  %%Function:  DoSpecDocProps  %%Owner:  bobz       */


DoSpecDocProps(hribl)
struct RIBL **hribl;
{
	/* for Opus, we convert the document properties pagestart and
		line start to first section props. For pagestart, we must
		turn on the fpgnRestart flag and set the value of the starting page.
		We ignore this if the values weren't entered or already match the first
		section.
	*/

	int CacheSectSedMac();
	int pgnStart = (**hribl).pgnStart;
	int lineStart = (**hribl).lineStart;
	CHAR rgb[8];
	int cb;
	BOOL fPgn;
	struct CA ca, caInval;
	int doc = (**hribl).doc;
	/* we save the dop revision marking until eod to avoid odd behavior
		inserting text with rev marking on, esp field chars
	*/
	if ((**hribl).fRevisions)
		PdopDoc(doc)->fRevMarking = fTrue;

	/* none? */
	if ( (pgnStart < 1) && lineStart == iNil)
		return;

	if (!PdodDoc(doc)->fMother)  /* should not be a subdoc  */
		return;

	/* get 1st section props */
	ExpandCaCache(PcaSet(&ca, doc, cp0, cp0),
			&caInval, &caSect, sgcSep, CacheSectSedMac);
	Assert (caSect.doc != docNil);
	Assert (caSect.cpFirst == cp0);

	/* assumptions made in handbuilding the grpprl */

	Assert (sprmSFPgnRestart < sprmSLnnMin
			&& sprmSLnnMin < sprmSPgnStart
			&& dnsprm[sprmSPgnStart].cch == 3
			&& dnsprm[sprmSFPgnRestart].cch == 2
			&& dnsprm[sprmSLnnMin].cch == 3
			&& sizeof(rgb) >= 8);

	cb = 0;
	fPgn = pgnStart > 0 && /* pgnstart specified */
	/* if same, still set if restart flag off */
	(pgnStart != vsepFetch.pgnStart || !vsepFetch.fPgnRestart);


	if (fPgn)
		{
		rgb[0] = sprmSFPgnRestart;
		rgb[1] = fTrue;
		cb = 2;
		}

	if (lineStart > 0)
		lineStart--;  /* section line start begins at 0 */
	if (lineStart !=  iNil &&
			lineStart !=  vsepFetch.lnnMin)
		{
		rgb[cb] = sprmSLnnMin;
		bltbyte(&lineStart, &rgb[cb + 1], sizeof(int));
		cb += 3;
		}

	if (fPgn)
		{
		Assert (cb >= 2);
		rgb[cb] = sprmSPgnStart;
		bltbyte(&pgnStart, &rgb[cb + 1], sizeof(int));
		cb += 3;
		}

	if (cb)
		{
		/* 1st section props  already set up, as is ca */
		ApplyGrpprlCa(rgb, cb, &ca);
		}
	/* invalidate sect */
	caSect.doc = docNil;

}



/* F  R E A D  R T F */
/*  Read text data from hData into docDest.
	Used by dde and clipboard

		Used for cfRTF
*/
/*  %%Function:  FReadRTF   %%Owner:  bobz       */

FReadRTF (docDest, cf, hData, cbInitial)
int docDest;
int cf;
HANDLE hData;
int cbInitial;
{
	BOOL fEol;
	struct RIBL **hribl;
	struct PAP papT;
	struct CHP chpT;
	LPCH lpch;
	unsigned long lcb;
	CHAR rgch[256];

	/* NOTE: Under Windows, global handle sizes are rounded up to the
		nearest 16-byte bound.  We compensate for the fact that lcb may
		be larger than the actual size of the scrap object by checking
		for a null terminator. See CchReadLineExt. */

	Assert (cf == cfRTF);

	/* note: if lcb is more than 64K, our simple methods won't work--don't try */
	if ((lcb = GlobalSize(hData)) > 0x00010000L)
		return fFalse;

	if ((lpch = GlobalLockClip(hData)) == NULL )
		return fFalse;

	/*  skip unwanted portion */
	lpch += cbInitial;
	lcb -= cbInitial;

	/* allocate and init hribl for RtfIn */
	if ((hribl = HriblCreateDoc(docDest)) == hNil)
		return fFalse;
	else
		Assert (docDest == (**hribl).doc);

	StandardChp(&chpT);

	while (lcb > 0 && !vmerr.fMemFail && !vmerr.fDiskFail)
		{
		/* Copy bytes from lpch to doc's cp stream */
		unsigned cch = lcb < 255 ? lcb : 255;

		if ((cch = CchReadLineExt( lpch, cch, rgch, &fEol))==0)
			/* Reached terminator */
			break;

		RtfIn(hribl, rgch, cch);

		lcb -= cch;
		lpch += cch;
		}

	 /* FUTURE: we should do the check for terminal eop code here
		that is done in doccreatertf1. Not doing it to avoid
		instability bz 10/19/89
	 */
	 /* so the last sections props will be copied over if we paste this */
	PdodDoc(docDest)->fSedMacEopCopied = fTrue;

	FreeHribl(hribl);

	GlobalUnlock (hData);

	return (fTrue);
}




/*  %%Function:  ChngToRdsPic  %%Owner:  bobz       */

ChngToRdsPic(hribl, val)
struct RIBL **hribl;
int val;
{


	struct CHP chp;
	int cb;
	char rgb[cbBlockPic];

	AssertH (hribl);

	/* note: leave props as they were so char props will come across */
	(**hribl).rds = rdsPic;

	(**hribl).fHiNibble = fTrue;
	(**hribl).bPic = 0;

	SetBytes(&(**hribl).pic, 0, cbPIC);
	(**hribl).pic.cbHeader = cbPIC;
	/* scaling values are normal %age, not scaled by 10. When
			we read in the pic struct we will scale up to
			the usual mx,my range. bz
		*/
	(**hribl).pic.mx = 100;
	(**hribl).pic.my = 100;
	(**hribl).pic.brcl = brclNone;
	(**hribl).pic.bm.bmPlanes = 1;
	(**hribl).pic.bm.bmBitsPixel = 1;
	/* goal sizes - if none (should always have one) set to 0; will
		calculate a 3" by aspect ratio size in PopRtfState
	*/
	(**hribl).iTypePic = iPicMacPict; // so unrecognized pics discarded
	(**hribl).fDisregardPic = fTrue;
	(**hribl).lcb = 0L;

	chp = (**hribl).chp;
	chp.fSpec = 1;
	chp.fnPic = 0; /* will be set at format; clears fDirty */
	/* this makes the font size 10 (opus default) not 12 (RTF
				default) so text inserted after a pic will be the right
				size.
			*/
	chp.hps = hpsDefault;

	/* ensure pic starts on a long word boundary */
	SetBytes(rgb, 0, cbBlockPic);

	chp.fcPic =  ((PfcbFn(fnScratch)->cbMac + 3) >> 2) << 2;
	PfcbFn(fnScratch)->fcPos = PfcbFn(fnScratch)->cbMac;

	/* write out enough space to get to boundary, then
		write out blanked Pic structure. Even if we find pic is
		invalid (e.g., is a mac picture), we will have a picture of
		length 0, which should be ignored
	*/


	if ((cb = (int) chp.fcPic - PfcbFn(fnScratch)->cbMac) > 0)
		WriteRgchToFn(fnScratch, rgb, cb);

	/* this has to be contiguous, don't use holes in scratch file */
	vfkpdText.pn = pnNil;


	WriteRgchToFn(fnScratch, rgb, cbBlockPic);

	Assert(chp.fcPic + cbBlockPic == PfcbFn(fnScratch)->cbMac);
	(**hribl).chp = chp;
}


/*  %%Function:  ChngToRdsGrid  %%Owner:  bobz       */

ChngToRdsGrid(hribl, val)
struct RIBL **hribl;
int val;
{
	/* if defining the grid table, this does not act like a destination, but
		just stores the val for use in the table definition. When used in the
		text it is a real destination that we switch to.
	*/

	(**hribl).iGrid = val;

	if ((**hribl).rds != rdsGridtbl)
		{
		ResetPropsToDefault(*hribl);
		(**hribl).rds = rdsGrid;
		}

	/* we are always putting this into docScrap which starts at cp0.
		cpRdsFirst is adjusted after each row in DoRtfParaEnd */
	Assert((**hribl).cpRdsFirst == cp0);
}


/*  %%Function:  ChngToRdsPrivate1  %%Owner:  bobz       */

ChngToRdsPrivate1(hribl, val)
struct RIBL **hribl;
int val;
{
	ResetPropsToDefault(*hribl);
	if (val == 1)
		(**hribl).rds = rdsPrivate1;
}


/*  %%Function:  ChngToRdsXeTc  %%Owner:  bobz       */

ChngToRdsXeTc(hribl, val)
struct RIBL **hribl;
int val;
{
	struct RCPXETC **hrcpxetc;
	struct RCPXETC *prcpxetc;

	/* need to put keyword and  " into doc */
	CHAR szKwd [cchMaxFieldKeyword + 3]; /* 3 == null, space, quote */
	int cch;
	int fVanished;

#ifdef BZ
	CommSzNum(SzShared("XeRc hplcpdc.imac at start = "),
			((struct PLC *)*(PdodDoc((**hribl).doc)->hplcpcd))->iMac);
#endif

	/* HACK NOTE   bz 9/14/89
		This code is not set up to handle nested xetc fields. The
		hribl fields get written over and trashed, and all hell breaks
		loose. To fix it, we could have all the xetc stuff be allocated,
		and stack the stuff that comes later. For now, though we just want to
		skip over the nested stuff.

		We set the fInXeTc flag here and reset in in PopRtfState.

		If we are already in this state; we put up a message and set the
		ris to skip this destination altogether.
	*/
	if ((*hribl)->fInXeTc)
		{
		(**hribl).ris = risScanByDest;
		/* just warn the user so they don't wonder why stuff is missing */
		ErrorEid(eidFormatTooComplex,"PopRtfState");
	    return;
		}

	(*hribl)->fInXeTc = fTrue; /* so we can tell if we nest */

	/* vanish used as flag to tell whether to include index text in doc */
	fVanished = (**hribl).chp.fVanish;
	(**hribl).chp.fVanish = fFalse;

	(**hribl).rds = val;
	/* note props left as is, not set to default so hidden
				etc can be dealt with outside the xe destination
			*/
	(**hribl).cpRdsFirst = CpMacDocEdit((**hribl).doc);

	cch = CchSzFromSzgId (szKwd, szgFltNormal,
			val == rdsXe ? fltXe : fltTce, cchMaxFieldKeyword);
	QszLower(szKwd);  /* table is upper case */
	Assert (cch);
	szKwd[cch - 1] = chSpace;
	szKwd[cch] = chDQuote;

	(*hribl)->fXeTcQOpen = fTrue; /* so we can end quotes */

	if (!FInsertRgch((**hribl).doc, (**hribl).cpRdsFirst, szKwd,
			cch + 1, &(**hribl).chp, 0))
		return;

	/* allocate area to hold start cps. If this fails, we can still
			do everything except put nonvanished text into doc
		*/

	if ((hrcpxetc = (**hribl).hrcpxetc) == hNil)
		{
		hrcpxetc = (**hribl).hrcpxetc = HAllocateCw(cwRCPXETC);
		if (hrcpxetc != hNil)
			{
			(*hrcpxetc)->docTemp = docNil;
			}
		else
			{
			/* vmerr.fMemFail set in FEnsureFreeCw */
			Assert (vmerr.fMemFail);
			ErrorNoMemory(eidNoMemOperation);
			vmerr.mat = matNil;
			return;
			}
		}

	/* set up start cp for field, invalid val for dcpInst */
	prcpxetc = *hrcpxetc;  /* heap pointer! */
	FreezeHp();
	/* this follows the xe/tc " */
	prcpxetc->cpFirstEntry = CpMacDocEdit((**hribl).doc);

	prcpxetc->cpLimEntry = prcpxetc->cpFirstText =
			prcpxetc->cpLimText = cpNil;
	prcpxetc->fInText = prcpxetc->fVanishText = fFalse;
	prcpxetc->fInEntry = fTrue;
	prcpxetc->fVanishEntry = fVanished;
	MeltHp();
}


/*  %%Function:  ChngToRdsXeRT  %%Owner:  bobz       */

ChngToRdsXeRT(hribl, val)
struct RIBL **hribl;
int val;
{
	struct RCPXETC *prcpxetc;
	int doc = (**hribl).doc;

	/* HACK NOTE   bz 9/14/89
		This code is not set up to handle nested xetc fields. The
		hribl fields get written over and trashed, and all hell breaks
		loose. To fix it, we could have all the xetc stuff be allocated,
		and stack the stuff that comes later. For now, though we just want to
		skip over the nested stuff.

		We set the fInXeRT flag here and reset in in PopRtfState.

		If we are already in this state; we put up a message and set the
		ris to skip this destination altogether.
	*/
	if ((*hribl)->fInXeRT)
		{
		(**hribl).ris = risScanByDest;
		/* just warn the user so they don't wonder why stuff is missing */
		ErrorEid(eidFormatTooComplex,"PopRtfState");
	    return;
		}

	(*hribl)->fInXeRT = fTrue; /* so we can tell if we nest */

	(**hribl).rds = val;

	TrnsfrmVal(hribl, val == rdsTxe ? chFldXeText : chFldXeRange, NULL);

	if ((**hribl).hrcpxetc != hNil)
		{
		prcpxetc = *((**hribl).hrcpxetc);  /* heap pointer! */
		if (prcpxetc->cpFirstEntry == CpMacDocEdit(doc))
			/* no text yet. have to rebuild structures */
			{

			prcpxetc->cpFirstEntry = cpNil; /* so we can restore later */
			Assert ((**hribl).fXeTcQOpen == fTrue);
			}
		else  /* insert quote for switch text */			
			{
			int ch = chDQuote;
			FInsertRgch(doc, CpMacDocEdit(doc), &ch,
					1, &(**hribl).chp, 0);
			}

		(*hribl)->fXeTcQOpen = fTrue; /* so we can end quotes */

		prcpxetc = *((**hribl).hrcpxetc);  /* heap pointer! */
		prcpxetc->fInEntry = fFalse;

		if (val == rdsTxe)
			{
			prcpxetc->cpFirstText = CpMacDocEdit(doc);
			prcpxetc->fInText = fTrue;
			/* if vanished in the entry text, not put into chp */
			prcpxetc->fVanishText =
					(**hribl).chp.fVanish | prcpxetc->fVanishEntry;
			}
		}

}


/*  %%Function:  SpecChToFld  %%Owner:  bobz       */

SpecChToFld(hribl, val)
struct RIBL **hribl;
int val;
{
	char rgch[cchMaxSz -1];
	char *pch;
	int doc;
	CP cpFirst, dcpT;
	int cbInst;
	int ifld;

	doc = (**hribl).doc;
	/* note that desired isz is stored in val */

	pch = PchSzRtfMove(val, rgch);	/* get string in local array */
	cbInst = pch - rgch;
	Assert (cbInst <= sizeof(rgch));

	/* put instruction text in cp stream */

	FInsertRgch(doc, (cpFirst = CpMacDocEdit(doc)), rgch,
			cbInst, &(**hribl).chp, 0);

	/* fieldify, parse, and calc result */

	dcpT = cbInst;
	if (vmerr.fMemFail || 
			!FPutCpsInField (fltUnknownKwd, doc, cpFirst, &dcpT, (CP)0, 
			&(**hribl).chp))
		return;

	FltParseDocCp (doc, cpFirst,
			(ifld = IfldFromDocCp (doc, cpFirst, fTrue)),
			fFalse  /* fChangeView */, fFalse /* fEnglish */);

	FCalcFieldIfld (doc, ifld, frmUser, 0, fFalse);
}


/*  %%Function:  TrnsfrmTxt  %%Owner:  bobz       */

TrnsfrmTxt(hribl, val)  /* can't be a macro; address used in tables */
struct RIBL **hribl;
int val;
{
	TrnsfrmVal(hribl, val, NULL);
}


/*  %%Function:  TrnsfrmVal  %%Owner:  bobz       */

TrnsfrmVal(hribl, ch, szVal)
struct RIBL **hribl;
int ch;
CHAR *szVal;

{
	/* put into text stream modified: used for field switches in tc and xe */

	char rgch[cchMaxSz -1];
	char *pch;
	int cbInst;
	int doc = (**hribl).doc;
	CP cpInsert = CpMacDocEdit(doc);

	/* note that desired switch ch is stored in val if not NULL */

	pch = rgch;

	if (ch == chColon) /* others are field switches */
		*pch++ = ch;
	else 		
		{
		if ((*hribl)->fXeTcQOpen == fTrue)
			CloseQuoteXeTc (hribl, &cpInsert);

#ifdef DEBUG
		Assert(ch == chFldXeBold || ch == chFldXeItalic || 
				ch == chFldXeText || ch == chFldXeRange  ||
				ch == chFldTceTableId || ch == chFldTceLevel);
#endif

		*pch++ = chSpace;
		*pch++ = chBackslash;

		*pch++ = ch;

		/* will still be open only if switch came before text
				or we had a colon. Always follow switch by space
				if text still to come  */

		if (szVal != NULL)
			{
			int cch = CchSz (szVal) - 1;
			bltbyte (szVal, pch, cch);
			pch += cch;
			}

		/* if text still open or switch that will use ", precede " with space */

		if ((*hribl)->fXeTcQOpen == fTrue ||
				ch == chFldXeText || ch == chFldXeRange)
			*pch++ = chSpace;
		}

	cbInst = pch - rgch;
	Assert (cbInst <= sizeof (rgch));
	/* put instruction text in cp stream */
	/* may go before " if switch before text */
	FInsertRgch(doc, cpInsert, rgch,
			cbInst, &(**hribl).chp, 0);

	/* restore text cp's if we have to */
	if ((*hribl)->fXeTcQOpen == fTrue && ch != chColon)
		{
		if ((**hribl).hrcpxetc != hNil)
			{
			struct RCPXETC *prcpxetc;
			prcpxetc = *((**hribl).hrcpxetc);  /* heap pointer! */
			prcpxetc->cpFirstEntry = CpMacDocEdit(doc);
			prcpxetc->fVanishEntry = (**hribl).chp.fVanish;
			prcpxetc->fInEntry = fTrue;
			Assert (!prcpxetc->fInText);
			}
		}
}


/*  %%Function:  CloseQuoteXeTc   %%Owner:  bobz       */

CloseQuoteXeTc (hribl, pcpInsert)
struct RIBL **hribl;
CP * pcpInsert; /* where text will do after this routine */
{
	int ch = chDQuote; /* end open arg quotes. */

	Assert ((*hribl)->fXeTcQOpen == fTrue);

	*pcpInsert = CpMacDocEdit((**hribl).doc);

	if ((**hribl).hrcpxetc != hNil)
		{
		struct RCPXETC *prcpxetc;
		prcpxetc = *((**hribl).hrcpxetc);  /* heap pointer! */
		FreezeHp();

		if (prcpxetc->cpFirstEntry == *pcpInsert)
			{
			/* no text entered yet. will have to slip switch in before
				quote and not close quote.
			*/
			(*pcpInsert)--;
			MeltHp();
			return;
			}

		/* if terminating entry or text, store cp */
		if (prcpxetc->fInEntry)
			prcpxetc->cpLimEntry = *pcpInsert;
		else  if (prcpxetc->fInText)
			prcpxetc->cpLimText = *pcpInsert;
		MeltHp();
		}

	FInsertRgch((**hribl).doc, *pcpInsert, &ch,
			1, &(**hribl).chp, 0);

	(*pcpInsert)++;
	(*hribl)->fXeTcQOpen = fFalse; /* so we won't end quotes twice */

}


/*  %%Function:  TrnsfrmTcf  %%Owner:  bobz       */

TrnsfrmTcf(hribl, val)
struct RIBL **hribl;
int val;
{
	/* put into text stream modified: used for field switches in tc and xe */
	/* convert val to a string */

	char rgch[3];

	/* string maps into A-Z. if out of range after ChUpper, make into
				default 'C'
			*/
	*rgch = chSpace;
	*(rgch + 1) = ChUpper(val);
	if (*(rgch + 1) < 'A' || *(rgch + 1) > 'Z')
		*(rgch + 1) = 'C';

	*(rgch + 2) = 0;

	TrnsfrmVal(hribl, chFldTceTableId, rgch);

}


/*  %%Function:  TrnsfrmTcl  %%Owner:  bobz       */

TrnsfrmTcl(hribl, val)
struct RIBL **hribl;
int val;
{
	/* put into text stream modified: used for field switches in tc and xe */
	/* convert val to a string */

	char rgch[7];
	char *pch;

	*rgch = chSpace;
	pch = rgch + 1;

	CchIntToPpch(val, &pch);
	*pch = 0;

	TrnsfrmVal(hribl, chFldTceLevel, rgch);

}


/*  %%Function:  FVanishXeTc  %%Owner:  bobz       */

FVanishXeTc(hribl, fVanish)
struct RIBL **hribl;
int fVanish;
{
	struct RCPXETC *prcpxetc;

	/* for xe and tc, vanished is not put into the chp but
				is a signal to omit the entry texts from the document
				stream. Also for text in the \txe switch of xe
				*/
	if (((**hribl).rds == rdsXe || (**hribl).rds == rdsTc
			|| (**hribl).rds == rdsTxe)
			&& (**hribl).hrcpxetc != hNil)
		{
		prcpxetc = *((**hribl).hrcpxetc);
		if (prcpxetc->fInEntry)
			prcpxetc->fVanishEntry = fVanish;
		else  if (prcpxetc->fInText)
			prcpxetc->fVanishText = fVanish;
		return (fTrue);
		}

	return (fFalse);
}




/*  %%Function:  LFromSzNumber  %%Owner:  bobz       */

long LFromSzNumber(szNumber)
char *szNumber;
{
	char *PchSkipSpacesPch();
	long l = 0;
	char ch;

	szNumber = PchSkipSpacesPch(szNumber);
	while ( ((ch = *szNumber++) >= '0') && (ch <= '9') )
		l = (l * 10) + (ch - '0');

	return l;
}


/* D T T M  F R O M  P D T R */
/*  %%Function:  DttmFromPdtr  %%Owner:  bobz       */

struct DTTM DttmFromPdtr(pdtr)	/* convert struct to internal date/time form  */
struct DTR	*pdtr;
{
	struct DTTM dttm;

	dttm.yr   = pdtr->yr - dyrBase;
	dttm.mon  = pdtr->mon;
	dttm.dom  = pdtr->dom;
	dttm.hr   = pdtr->hr;
	dttm.mint = pdtr->mint;
	dttm.wdy  = pdtr->wdy;
	return (dttm);
}


/*  %%Function:  FinishXeTc  %%Owner:  bobz       */

FinishXeTc(hribl)
struct RIBL **hribl;
{
	int doc;
	CP cp, dcp;
	struct RCPXETC *prcpxetc;

	/* all the characters in the instruction text have been
				written to the CP stream; If we never closed the
				instruction text with a switch, we will need to add
				a quote to the stream.
				We saved the starting CP when the \xe or \tc was
				encountered.
			Now we call field processing routines to turn the CP's
			(with any nexted fields already fieldified) into fields
			and force the field to be parsed.
	*/

	doc = (**hribl).doc;
	if  ((**hribl).fXeTcQOpen)
		{
		char ch = chDQuote;

		cp = CpMacDocEdit(doc);

		/* get end cp of text we are closing off */
		if ((**hribl).hrcpxetc != hNil)
			{
			prcpxetc = *((**hribl).hrcpxetc);  /* heap pointer! */
			if (prcpxetc->fInEntry)
				prcpxetc->cpLimEntry = cp;
			else  if (prcpxetc->fInText)
				prcpxetc->cpLimText = cp;
			}

		FInsertRgch(doc, cp, &ch,
				1, &(**hribl).chp, 0);
		}

	if ((**hribl).hrcpxetc != hNil)
		{
		prcpxetc = *((**hribl).hrcpxetc);  /* heap pointer! */
		if (!prcpxetc->fVanishEntry || !prcpxetc->fVanishText)
			{
			struct CA caT, caIns;
			int docHold;

			/* create a holding doc in hribl for temp use (note that
					vdocScratch, vdocTemp and docScratch are unavailable and
					docUndo is questionable, so just create one. Note heap
					pointers restored whenever in danger.
			
				*/
			if ((docHold = (**(**hribl).hrcpxetc).docTemp) == docNil)
				docHold = (**(**hribl).hrcpxetc).docTemp =
						DocCloneDoc(docNew, dkDoc);

			if (docHold != docNil)
				{
				BOOL fEntry, fText;

				SetWholeDoc(docHold, PcaSetNil(&caT));
				 /* this sets up the initial style in the temp doc so the
					props of the text being inserted will be wrt the style.
					First point stsh to that of normal doc.
		 		*/
				{
				struct DOD *pdod = PdodDoc(docHold);
				pdod->fMotherStsh = fTrue;
				pdod->doc = doc;
				}

				prcpxetc = *((**hribl).hrcpxetc);  /* heap pointer! */
#ifdef XBZ
				CommSzRgNum(SzShared("rcpxetc at end of xetc: "), prcpxetc, cwRCPXETC);
#endif
				/* append text to doc, then move it out */
				fEntry = (!prcpxetc->fVanishEntry && prcpxetc->cpFirstEntry != cpNil);
				fText = (!prcpxetc->fVanishText && prcpxetc->cpFirstText != cpNil);
				/* if text before entry, switch */

				if (fEntry && fText && prcpxetc->cpFirstEntry > prcpxetc->cpFirstText)
					{
					Assert (prcpxetc->cpLimEntry != cpNil && prcpxetc->cpLimText != cpNil);
					InsertXeTcText(hribl, docHold, prcpxetc->cpFirstText, prcpxetc->cpLimText);
					prcpxetc = *((**hribl).hrcpxetc);  /* restore heap pointer! */
					InsertXeTcText(hribl, docHold, prcpxetc->cpFirstEntry, prcpxetc->cpLimEntry);
					}
				else
					{
					if (fEntry)
						{
						Assert (prcpxetc->cpLimEntry != cpNil);
						InsertXeTcText(hribl, docHold, prcpxetc->cpFirstEntry, prcpxetc->cpLimEntry);
						prcpxetc = *((**hribl).hrcpxetc);  /* restore heap pointer! */
						}

					if (fText)
						{
						Assert (prcpxetc->cpLimText != cpNil);
						InsertXeTcText(hribl, docHold, prcpxetc->cpFirstText, prcpxetc->cpLimText);
						}
					}
				  /* we added an extra crlf to dochold to ensure correct properties
					 but we do not want to copy it. bz
				  */
				if (CpMacDocEdit(docHold) > (CP)0)
					{
					/* shove added text before the field & adjust field cp */
					PcaPoint(&caT, doc, (**hribl).cpRdsFirst);
					PcaSet(&caIns, docHold, cp0, CpMacDocEdit(docHold));

					(**hribl).cpRdsFirst += (caIns.cpLim - caIns.cpFirst);
					FReplaceCps(  &caT, &caIns );
					}
				}

			else
				{
				ErrorNoMemory(eidNoMemOperation);
				ReportSz("Warning - No hold doc for xe text transfer - text lost");
				}

			}
		}

	cp = (**hribl).cpRdsFirst;
	dcp = CpMacDocEdit(doc) - cp;
	if (!FPutCpsInField (fltUnknownKwd, doc,
			cp, &dcp, (CP)0 /* dcpRslt - none for dead fields */, NULL))
		return;

	FltParseDocCp (doc, cp,
			IfldFromDocCp (doc, cp, fTrue),
			fFalse	/* fChangeView */, fFalse /* fEnglish */);

#ifdef BZ
	CommSzNum(SzShared("XeRc hplcpdc.iMac at end = "),
			((struct PLC *)*(PdodDoc(doc)->hplcpcd))->iMac);
#endif


}


/*  %%Function:  InsertXeTcText  %%Owner:  bobz       */

InsertXeTcText(hribl, docHold, cpFirst, cpLim)
struct RIBL **hribl;
int docHold;
CP cpFirst, cpLim;
{
	CHAR HUGE *hpch;
	CHAR HUGE *hpchLim;
	CHAR rgch[64];
	CHAR *pch;
	struct PAP pap;
	struct CHP chp;
	int ch;
	BOOL fHadSlash = fFalse;
	int doc = (**hribl).doc;

	while (cpFirst < cpLim)
		{
		pch = rgch;  /* need to reset each time in loop */
		FetchCpAndPara(doc, cpFirst, fcmChars+fcmProps);
		chp = vchpFetch;
		hpch = vhpchFetch;
		hpchLim = vhpchFetch +
				(int) CpMin(vccpFetch, cpLim - cpFirst);
		ApplyStcEndOfDoc(docHold, vpapFetch.stc);
		while (hpch < hpchLim)
			/* handle converted colons and quotes */
			/* : or " preceded by \ go in without the \. For other
						chars, output the \ we omitted before and continue on.
					*/
			{
			if (fHadSlash)  /* last char was a backslash */
				{
				fHadSlash = fFalse;
				if (*hpch != chColon && *hpch != chDQuote)
					{
					/* add back \ */
					AddChXeTc(docHold, chBackslash, rgch, &pch, sizeof (rgch), &chp);
					}
				}
			else  if (*hpch == chBackslash || *hpch == chColon)
				/* skip over but set flag if backslash */
				{
				fHadSlash = (*hpch == chBackslash);
				hpch++;
				continue;
				}

#ifdef INFOONLY
			else
				{
				--- other cases just leave char in buffer -----
				}
#endif


			AddChXeTc(docHold, *hpch, rgch, &pch, sizeof (rgch), &chp);
			hpch++;

			} /* while */

		if (pch != rgch)
			{
			/* if we got here, the last char was not a para terminator, or
			   AddChXeTc would have	dumped the text and reset pch.
			*/
			Assert ((ch = *(pch - 1)) != chEol && ch != chSect &&
					ch != chTable);

			FInsertRgch(docHold, CpMacDocEdit(docHold), rgch,
					pch - rgch, &chp, NULL);
			}
		cpFirst += vccpFetch;
		}
}


/*  %%Function:  AddChXeTc  %%Owner:  bobz       */

AddChXeTc(doc, ch, pchStart, ppch, cchMax, pchp)
int doc;
int ch;
CHAR *pchStart;
CHAR **ppch;
int cchMax;
struct CHP *pchp;
{
	struct PAP *ppap;
	int dch, cchLim;
	/* put the ch in the buffer. Dump out when only 2 spaces are left
				unless a chReturn, in which case you can wait and put it in the 2nd
				to last spot, leaving room for its partner. This is to avoid splitting
				up cr-lf's.
			*/
	Assert (cchMax > 2);

	ppap = (ch  == chEol || ch == chSect ||
			ch == chTable) ? &vpapFetch : NULL;

	**ppch = (CHAR)ch;
	(*ppch)++;
	dch = *ppch - pchStart;
	Assert (dch <= cchMax);

	cchLim = (ch == chReturn ? cchMax - 1 : cchMax - 2);

	if (dch > cchLim || ppap != NULL)
		{
		FInsertRgch(doc, CpMacDocEdit(doc), pchStart,
				dch, pchp, ppap);
		*ppch = pchStart;
		}

}


/* grabbed from tablemac.c. In Mac, this is a user interface level function */
/*  T A B L E  F R O M  S I D E  B Y  S I D E */
/* Formats caSel from side by side into a table */
/*  %%Function:  FTableFromSideBySide  %%Owner:  bobz       */

FTableFromSideBySide(pcaSel, fn)
struct CA *pcaSel;
int fn;
{
	/* normal default will cut off double para borders */
#define dxaGapHalfRtf (dxaDefaultGapHalf + 6)

	CP cpCur, cpLim, cpLimSect, cpFirst;
	struct CA caT, caUndo, caCurPara;
	int dxaLeft, cCol, dxaColumnWidth, dxaRightLast = 0;
	struct TAP	tap;
	struct CHP	chpT;
	struct PAP	papT;
	int doc = pcaSel->doc;
	int docTemp;
	int fAllTable = fTrue;
	int cchPapx = 0;
	int cTables = 0;
	char *pch;
#ifdef MAC
	struct DLG *pdlg = PdlgFrame(&dlgSideBySide);
	struct CAB **hcab;
#endif /* MAC */
	char prlSide[8];
	char papx[cchPapxMax+1];
	char szConverted[cchMaxSz];
	BOOL fBorder = fFalse;
	BOOL fRet = fTrue;

#ifdef MAC
	TurnOffSel(&selCur);
#endif /* MAC */

	if ((docTemp = DocCreate(fnNil)) == docNil)
		return (fFalse);

	/* Make sure we cover full paragraphs */
	ExtendPcaParas(pcaSel);

	caUndo = *pcaSel;

#ifdef MAC
	if (!FSetUndoB1(ucmNewTable, uccPaste, &caUndo))
		return cmdCancelled;
	InvalAgain();

	StartLongOp();
#endif /* MAC */

	CachePara(doc, caUndo.cpLim - ccpEop);
	/* Check to see if we are selecting the EOD mark */
	/* if so we back up and insert extra eop */
	if (vpapFetch.fSideBySide && !FCheckPcaEndDoc(&caUndo))
		{
		fRet = fFalse;
		goto LDisposeDoc;
		}
	SetWholeDoc(docTemp, pcaSel);
	if (vmerr.fMemFail)
		{
		fRet = fFalse;
		goto LDisposeDoc;
		}
	{
	struct DOD *pdodTemp, *pdod;   /* localize these */

	FreezeHp();
	(pdodTemp = PdodDoc(docTemp))->fMotherStsh = fTrue;
	pdodTemp->doc = doc;

	pdod = PdodDoc(doc);
	pdodTemp->dop = pdod->dop;
	pdodTemp->dop.grpfIhdt = 0;
	if (pdod->fShort || pdod->fMotherStsh)
		pdodTemp->doc = pdod->doc;
	MeltHp();
	}
	/* Initialize values */
	prlSide[0] = sprmPFSideBySide;
	prlSide[1] = fFalse;
	prlSide[2] = sprmPDxaRight;
	prlSide[3] = prlSide[4] = (char) 0;
	prlSide[5] = sprmPDxaLeft;
	prlSide[6] = prlSide[7] = (char) 0;
	dxaLeft = 0;
	cCol = 0;
	cpCur = cp0;
	cpLim = pcaSel->cpLim - pcaSel->cpFirst;

	/* Get the first end of section */
	CacheSect(docTemp, cpCur);
	dxaColumnWidth = vsepFetch.dxaColumnWidth;
	cpLimSect = caSect.cpLim;
	SetWords(&tap, 0, cwTAP);

	while (cpCur < cpLim)
		{

#ifdef MAC
		SetLlcPercentCp(cp0, cpLim, cpCur);
#endif /* MAC */

		CachePara(docTemp, cpCur);
		cpCur = caPara.cpLim;
		caCurPara = caPara;
		cpFirst = caPara.cpFirst;


#ifdef WIN
			{
			/* if borders anywhere in row we will add a gaphalf */
			int ibrc;
			WORD *pbrc = vpapFetch.rgbrc;
			for (ibrc = 0; ibrc < ibrcPapLim; ibrc++)
				fBorder |= (*(pbrc + ibrc));
			}
#endif /* WIN */


		if (cCol && (!vpapFetch.fSideBySide || vpapFetch.fPageBreakBefore ||
				vpapFetch.fInTable || caPara.cpFirst >= cpLimSect ||
				caPara.cpLim == cpLimSect ||
				vpapFetch.dxaLeft < dxaLeft ||
				cCol >= itcMax - 1))
			{	/* We have come to the end of a row */
			if (!FInsertCellMark(PcaSet(&caT, docTemp, caCurPara.cpFirst - cchEop,
					caCurPara.cpFirst), &chpT, &papx, cchPapx, papT.stc))
				{
				fRet = fFalse;
				goto LDisposeDoc;
				}
			papT.ptap = &tap;
			tap.itcMac = cCol;
			tap.rgdxaCenter[cCol] = max(tap.rgdxaCenter[cCol - 1] + dxaMinDefaultColumnWidth,
					dxaColumnWidth - dxaRightLast);
			/* leave space for borders */
			tap.dxaGapHalf = fBorder ? dxaGapHalfRtf : 0;
			fBorder = fFalse;

			if (!FInsertRowMark(PcaSet(&caT, docTemp, caCurPara.cpFirst,
					caCurPara.cpFirst), &chpT, &papT))
				{
				fRet = fFalse;
				goto LDisposeDoc;
				}
			CachePara(docTemp, cpCur);
			if (!(vpapFetch.fSideBySide && vpapFetch.dxaLeft < dxaLeft))
				cTables++;
			cpCur += ccpEop;
			cpFirst += ccpEop;
			cpLim += ccpEop;
			cpLimSect += ccpEop;
			SetWords(&tap, 0, cwTAP);
			cCol = 0;
			dxaLeft = 0;
			dxaRightLast = 0;

			/* if the piece table gets too big, save */
			CheckCompressDoc(docTemp);
			}

		if (caCurPara.cpFirst >= cpLimSect)
			{
			/* A new section */
			CacheSect(docTemp, cpFirst);
			dxaColumnWidth = vsepFetch.dxaColumnWidth;
			cpLimSect = caSect.cpLim;
			}

		if (!vpapFetch.fSideBySide || vpapFetch.fInTable || caCurPara.cpLim == cpLimSect)
			{
			fAllTable = fFalse;
			continue;
			}

		CachePara(docTemp, cpFirst);
		caT = caPara;
		ApplyTableProps(&caT, fTrue);
		ApplyGrpprlCa(prlSide, 8, &caT);
		if (vmerr.fMemFail)
			{
			fRet = fFalse;
			goto LDisposeDoc;
			}
		CachePara(docTemp, cpFirst);
		dxaRightLast = vpapFetch.dxaRight;

		if (cCol == 0)
			{
			dxaLeft = vpapFetch.dxaLeft;
			tap.rgdxaCenter[0] = dxaLeft;
			cCol++;
			}
		else
			{	/* Not the first cell */
			if (dxaLeft < vpapFetch.dxaLeft)
				{
				dxaLeft = vpapFetch.dxaLeft;
				/* Same row, one cell over */
				if (!FInsertCellMark(PcaSet(&caT, docTemp, cpFirst - cchEop,
						cpFirst), &chpT, &papx, cchPapx, papT.stc))
					{
					fRet = fFalse;
					goto LDisposeDoc;
					}
				tap.rgdxaCenter[cCol] = dxaLeft;
				cCol++;
				}
			}

		/* Remember paragraph properties */
		FetchCpAndPara(docTemp, cpFirst, fcmProps);
		chpT = vchpFetch;
		CleanTableChp(pcaSel->doc, &chpT);
		papT = vpapFetch;
		CleanTablePap(&papT, docTemp, fFalse, clrAll);
		cchPapx = CbGenTablePapxFromPap(docTemp, &papT, &papx);

		}

	if (cCol)
		{	/* We have a row to finish off */
		CachePara(docTemp, cpCur);
		cpFirst = caPara.cpFirst;

		if (!FInsertCellMark(PcaSet(&caT, docTemp, cpFirst - cchEop,
				cpFirst), &chpT, &papx, cchPapx, papT.stc))
			{
			fRet = fFalse;
			goto LDisposeDoc;
			}
		papT.ptap = &tap;
		tap.itcMac = cCol;
		tap.rgdxaCenter[cCol] = max(tap.rgdxaCenter[cCol - 1] + dxaMinDefaultColumnWidth,
				dxaColumnWidth - dxaRightLast);
		/* leave space for borders */
		tap.dxaGapHalf = fBorder ? dxaGapHalfRtf : 0;
		fBorder = fFalse;
		if (!FInsertRowMark(PcaSet(&caT, docTemp, cpFirst, cpFirst),
				&chpT, &papT))
			{
			fRet = fFalse;
			goto LDisposeDoc;
			}
		cpLim += ccpEop;
		cTables++;
		}

	/* if the piece table gets too big, save */
	CheckCompressDoc(docTemp);

	FReplaceCps(pcaSel, PcaSet(&caT, docTemp, cp0, cpLim));

	if (vmerr.fMemFail)
		{
		fRet = fFalse;
		goto LDisposeDoc;
		}

	pcaSel->cpLim = cpLim + pcaSel->cpFirst;


#ifdef MAC
	InvalPageView(doc);

	UnprotectLlc();
	/* INTERNATIONAL: The message being sent to the llc is
		* "n paragraphs" where n is the number of paragraphs converted
		* from side by side to tables.  The constant cchSzParagraphs
		* is the count of characters in the string plus 1 so that the
		* 0 is added to the end of stConverted. */
#define cchSzTable	                                                                                     7
#define cchSzTables	8
#define cchSzCreated	9

	if (!cTables)
		{
		AlertStop(SzSharedKey("No side by side paragraphs found.", NOSIDEBYSIDE));
		}
	else
		{
		pch = &stConverted[1];
		CchIntToPpch(cTables, &pch);
		if (cTables == 1)
			{
			CopyRgch(SzFrameKey(" table", SIDETABLE), pch, cchSzTable);
			pch += cchSzTable;
			}
		else
			{
			CopyRgch(SzFrameKey(" tables", SIDETABLES), pch, cchSzTables);
			pch += cchSzTables;
			}
		CopyRgch(SzFrameKey(" created.", SIDECREATED), pch, cchSzCreated);
		stConverted[0] = pch - stConverted + cchSzCreated - 1;

		pstSideBySideMsg = &stConverted[0];
		hcab = HcabAlloc(iagSideBySideMac);
		if (hcab != hNil)
			{
			InitCab(hcab, iagSideBySideMac);
			TmcDoDlg(pdlg, hcab);
			}
		}

	if (doc == selCur.doc)
		{
		if (fAllTable)
			SelectRow(&selCur, pcaSel->cpFirst, pcaSel->cpLim);
		else
			Select(&selCur, pcaSel->cpFirst, pcaSel->cpLim);
		}

	SetUndoAfter(&caUndo);


#endif /* MAC */
LDisposeDoc:
	DisposeDoc(docTemp);

#ifdef MAC
	EndLongOp();
	if (vmerr.fMemFail)
		{
		SetUndoNil();
		return cmdNoMemory;
		}
	else
		{
		DirtyOutline(selCur.doc);
		return cmdOK;
		}
#endif /* MAC */

	return fRet;
}



/* F O R M A T  T A B L E  H R I B L */
/* Post-processing for a table row that came in as RTF.  In the temp
	doc, it has tabs.   We use the gridtbl info in the hribl to format
	that tabified text as a table.  The hsttbGrid has the information
	about the number of columns and widths of columns.  It has one st
	entry, which has the following format:

	Byte 0:  cb
		Byte 1:  unused
		Byte 2...Byte cb:  rgdxa[], where each dxa is a word and
		there are cColumns of them (cColumns = (cb -1)/2)

	The pap in the hribl also contains the tab information that lets
	us know how the cells should be justified.  pap.rgtbs parallels
	rgdxa, and the tab jc's correspond to paragraph jc's for the
	cells (each cell will be one paragraph).  We are assuming we won't
	get any decimal tabs.
*/
/*  %%Function:  FFormatTableHribl  %%Owner:  rosiep   */

BOOL FFormatTableHribl(hribl)
struct RIBL **hribl;
{
	extern struct CA caTap;

	int doc = (**hribl).doc;
	CP cpFirst = (**hribl).cpRdsFirst;
	CHAR *pst, grpprl[2];
	int cColumns, itc, dxaCenter;
	int *rgdxa;
	struct TBD *ptbd;
	struct CA caT, caDummy;
	struct TAP tapNew;

#ifdef RPDEBUG
	ShowFetch(doc);
#endif
	/* remove leading tab; Excel always puts one in (except bug below) */
	FetchCp(doc, cpFirst, fcmChars);
	if (*vhpchFetch == chTab)
		{
		PcaSetDcp(&caT, doc, cpFirst, 1);
		FDelete(&caT);
		}
	else
		{
		/* Warning: EXCEL BUG!! */
		/* If Excel selection contains empty rows, they give us empty paras,
		which do not begin with a tab, so don't try to delete it.  We will
		still not be able to correctly convert the bogosity they give us.
		They should be giving us a grid anyway, with the correct number of
		rows and columns.  Best we can do now is leave the empty paras as
		empty paras.  This is as of Excel 2.0 - rp */
		return fFalse;
		}

	/* get number of columns */
	pst = PstFromSttb((**hribl).hsttbGrid, 0);
	cColumns = (*pst - 1)/2;

	/* if too many columns, leave as tabified text */
	if (cColumns >= itcMax)
		return fTrue;  /* user notified when gridtbl was being processed */

	/* build up tap for new row */
	SetWords(&tapNew, 0, cwTAP);
	tapNew.dyaRowHeight = (**hribl).pap.dyaLine;
	tapNew.dxaGapHalf = dxaDefaultGapHalf;
	tapNew.rgdxaCenter[0] = dxaCenter = -dxaDefaultGapHalf;
	rgdxa = pst + 2;
	for (itc = 1; itc <= cColumns; itc++)
		{
		tapNew.rgdxaCenter[itc] =
				(dxaCenter += rgdxa[itc-1] + 2 * dxaDefaultGapHalf);
		/* table too wide, leave as tabified text */
		if (dxaCenter >= xaRightMaxSci ||
				dxaCenter < tapNew.rgdxaCenter[itc-1] /* int overflow */)
			return fTrue; /* user notified too large when gridtbl was being built */
		}

	/* format text as table row */
	PcaSet(&caT, doc, cpFirst, CpMacDocEdit(doc));
	if (!FFormatTablePca(&caT, &caDummy, tblfmtTab, 1, cColumns, &tapNew))
		return fFalse;

	/* apply para formatting if necessary */
	CpFirstTap(doc, cpFirst);
	for (itc = 0, ptbd = (**hribl).pap.rgtbd; itc < cColumns; itc++, ptbd++)
		{
		Assert(ptbd->jc >= 0 && ptbd->jc <= 2);
		if (ptbd->jc > 0)
			{
			PcaColumnItc(&caT, &caTap, itc);
			grpprl[0] = sprmPJc;
			grpprl[1] = ptbd->jc;
			ApplyGrpprlCa(grpprl, 2, &caT);
			}
		}

	return fTrue;
}


/* NOTE: it's usually totally harmless if this fails; style gets mapped to
	stcNormal */
/* FUTURE: should be folded back into FEnsureStcDefined with an
	fRTF flag added to the parameter list.  The only difference is that
	this version does not ensure a standard's next style.  If we did,
	FCopyStyleToDestStsh would fail.
	We don't change this now due to MacWord ship status (davidbo) */
/*  %%Function:  FEnsureRtfStcDefined  %%Owner:  bobz       */

FEnsureRtfStcDefined(doc, stc, pcstcStd)
int doc, stc;
int *pcstcStd;
{
	int stcpNew;
	int stcp;
	int stcpMac;
	char stNil[1], stZero[1];
	struct ESTCP estcp;
	struct STSH stsh;
	struct STTB **hsttbChpe, **hsttbPape;

	RecordStshForDoc(doc, &stsh, &hsttbChpe, &hsttbPape);
	*pcstcStd = stsh.cstcStd;

	stZero[0] = 0;
	if (stc > stcStdMin)
		{
		if ((cstcMax - stc) > *pcstcStd)
			{
			if (!FAddStdStcsToStsh(doc, stc, pcstcStd))
				return (fFalse);
			}
		else  if (FStcpEntryIsNull(stsh.hsttbName, stcp = StcpFromStc(stc, *pcstcStd)))
			{
			/* stZero is empty, therefore this is no larger then the string
			that was here already--won't fail.*/
			AssertDo(FChangeStInSttb(stsh.hsttbName, stcp, stZero));
			}

		Mac( vspop.fSttbIncomplete = fTrue; )
				goto LDirtyStsh;
		}
	else
		{
		stcpNew = StcpFromStc(stc, *pcstcStd);
		if (stcpNew >= (stcpMac = (*stsh.hsttbName)->ibstMac))
			{
			stNil[0] = 255;
			estcp.stcBase = stcNormal;
			for (stcp = stcpMac; stcp <= stcpNew; stcp++)
				{
				estcp.stcNext = StcFromStcp(stcp, *pcstcStd);
				if (!FInsertInPl(stsh.hplestcp, stcp, &estcp))
					return (fFalse);
				if (!FStretchSttb(stsh.hsttbName, 1, 1) ||
						!FStretchSttb(hsttbChpe, 1, 1) ||
						!FStretchSttb(hsttbPape, 1, 1) ||
						!FStretchSttb(stsh.hsttbChpx, 1, 1) ||
						!FStretchSttb(stsh.hsttbPapx, 1, 1))
					{
					((struct PL *)*stsh.hplestcp)->iMac--;
					return (fFalse);
					}
				/* since everything is pre-stretched, the inserts shouldn't fail */
				AssertDo(FInsStInSttb(stsh.hsttbName, stcp, stNil));
				AssertDo(FInsStInSttb(stsh.hsttbChpx, stcp, stNil));
				AssertDo(FInsStInSttb(stsh.hsttbPapx, stcp, stNil));
				AssertDo(FInsStInSttb(hsttbChpe, stcp, stNil));
				AssertDo(FInsStInSttb(hsttbPape, stcp, stNil));
				}
			}
		if (FStcpEntryIsNull(stsh.hsttbName, stcpNew))
			{
			FChangeStInSttb(stsh.hsttbName, stcpNew, stZero);
LDirtyStsh:
			PdodDoc(doc)->fStshDirty = fTrue;
			}
		}
	return fTrue;
}




#ifdef BZ

/*  %%Function:  DumpStshNml  %%Owner:  bobz       */

DumpStshNml(pstsh, hsttbPape)
struct STSH *pstsh;
struct STTB **hsttbPape;
{
	int stc, stcp, stcpMac;
	char st[cchMaxSz];
	int rgNum[cchMaxSz + 1];

		{
		stc = stcNormal;
		stcp = pstsh->cstcStd;

		GetStFromSttb(pstsh->hsttbName, stcp, st);
		if (st[0] == 0)
			GenStyleNameForStcp(st, pstsh->hsttbName, pstsh->cstcStd, stcp);
		else  if (st[0] == 0xff)
			{
			st[0] = 1;
			st[0] = 'x';
			}
		StToSzInPlace(st);
		CommSz(st);
		CommSzNum(SzShared("  "), stc);

		GetStFromSttb(hsttbPape, stcp, st);
		StToRgNum(st, rgNum);
		CommSzRgNum(SzShared("\tPape: "), rgNum, st[0]);

		CommSz(SzShared("\n\r"));
		}
}


/*  %%Function:  StToRgNum  %%Owner:  bobz       */

StToRgNum(st, rg)
char st[];
int rg[];
{
	int i, iMac;

	/* make st's count byte include itself */
	st[0] += 1;
	iMac = st[0];
	for (i = 0; i < iMac; i++)
		rg[i] = (int) st[i];
}


#endif /* BZ */
