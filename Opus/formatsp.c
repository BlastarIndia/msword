/* F O R M A T S P . C */

#ifdef MAC
#define EVENTS
#define INTER
#include "toolbox.h"
#endif /* MAC */

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "doc.h"
#include "disp.h"
#include "file.h"
#include "props.h"
#include "border.h"
#include "sel.h"
#include "format.h"
#include "formula.h"
#include "ch.h"
#include "field.h"
#include "inter.h"
#include "debug.h"
#ifdef WIN
#include "pic.h"
#include "print.h"
#else
#include "mac.h"
#include "font.h"
#endif /* WIN */

#include "screen.h"


#ifdef PROTOTYPE
#include "formatsp.cpt"
#endif /* PROTOTYPE */

extern int              vlm;
extern int              vbchrMac;
extern int              vdocTemp;
extern CP               vcpFetch;
extern int              vdocFetch;
extern int		vfPrvwDisp;
extern struct CA        caPara;
extern struct CA        caPage;
extern struct CA        caSect;
extern struct CHP       vchpStc;
extern struct CHP       vchpFetch;
extern struct PAP       vpapFetch;
extern struct SEP       vsepFetch;
extern struct TCC       vtcc;
extern struct SEL       selCur;
extern struct FLI       vfli;
extern struct FMTSS     vfmtss;
extern struct FTI       vfti;
extern struct FTI       vftiDxt;
extern char             (**vhgrpchr)[];
extern FC               vfcSpec;

#ifdef WIN
extern int              vflm;
extern struct PIC	vpicFetch;
extern struct PRI	vpri;
extern struct SCI       vsci;
#else
extern int              far *far *vqqmpchdxu;
extern struct FWCA      vrgfwca[];

#define qdxuFont        *((unsigned long far *far *) 0xb10)
#define Ticks           *((long far *) 0x16a)
#endif


/* F O R M A T  C H  S P E C */
/* return width (dxp and dxt) of string expansion of special char ch.
String expansion is created from FMTSS state (logical part of vfli).
Prepare vfmtss as follows:
	chPicture: do nothing
	chPage: pgn in state is already set up
	chDate: set tick in state
	chTime: set tick in state
	chFootnote: set cpRef in state to vcpFetch
	chLnn: lnn in state is already set up
*/
/* %%Function:FormatChSpec %%Owner:bryanl */
EXPORT FormatChSpec(bchr, ich, pdxp, pdxt, pdyyAscent, pdyyDescent)
int bchr, ich, *pdxp, *pdxt;
DYY *pdyyAscent, *pdyyDescent;
{
	int ch = vfli.rgch[ich];
	int cch;
	int dxp, dxt, dxpT;
	int dxa;
	int fDxt;
	char *pch;

	char rgch[cchChSpecMax];

	if (ch == chPicture)
		{
		GetPictureInfo(bchr, ich, pdxp, pdxt, pdyyAscent, pdyyDescent);
		Win( vfli.fPicture = TRUE );
#ifdef WIN
		/* note metafiles to keep out of scc cache and also to
		                     allow printing in text mode for 2 pass printers
		  */
		if (vpicFetch.mfp.mm < MM_META_MAX)
			vfli.fMetafile = TRUE;
#endif /* WIN */
		return;
		}
#ifdef MAC
	if (ch > ' ')
		{
		if (FFormatChSpecSymbol(&vchpFetch, ch, pdxp, pdxt, pdyyAscent, pdyyDescent))
			((struct CHR *)&(**vhgrpchr)[bchr])->chp.fSpec = fTrue;
/* vchpFetch.fSpec was set in proc */
		return;
		}
	LoadFont(&vchpFetch, fTrue);
#endif /* MAC */

#ifdef WIN
#ifdef DEBUG
	if (ch > chSpecLast && (ch >= chFieldMax || ch < chFieldMin))
		/* we get into this after failing the assert and going on.
		      Probably hosed -	dump out vhpchFetch and
		      give warning. bz 9/29/87
		   */
		{
		extern int vdocFetch;
		extern int vccpFetch;
		Assert (fFalse);
		CommSz (SzShared("FormatchSpec bad spec ch - you are hosed and should not continue\n"));
		CommSzNum (SzShared("FormatchSpec: Offending ch = "), ch);
		CommSzNum (SzShared("FormatchSpec: Offending ich = "), ich);
		CommSzNum (SzShared("FormatchSpec: Offending doc = "), vdocFetch);
		CommSzLong (SzShared("FormatchSpec: Offending cp = "), vcpFetch);

#ifdef CANTDOIT /* now as hpch won't work bz */
		CommSzRgCch (SzShared("VhpchFetch as an rgch: "), vhpchFetch, vccpFetch);
#endif 

		Assert (fFalse);
		}
#endif /* DEBUG */
#endif	/* WIN */
	pdyyDescent->dyp = vfti.dypDescent;
	pdyyAscent->dyp = vfti.dypAscent;

	Win( pdyyDescent->dyt = vftiDxt.dypDescent );
	Win( pdyyAscent->dyt = vftiDxt.dypAscent );
	vfmtss.cpRef = vcpFetch;
/* can't use vdocFetch, cache has been invalidated */
	vfmtss.doc = vfli.doc;
	switch (ch)
		{
	case chTFtn:
		dxa = max(0, min(dxaInch * 2,
				vfli.dxa - vpapFetch.dxaLeft + min(0, vpapFetch.dxaLeft1) - vpapFetch.dxaRight));
		goto LDxa;
	case chTFtnCont:
		dxa = max(0, vfli.dxa - vpapFetch.dxaLeft + min(0, vpapFetch.dxaLeft1) - vpapFetch.dxaRight);
LDxa:           
		*pdxt = NMultDiv(dxa, vfli.dxuInch, dxaInch);
		*pdxp = NMultDiv(dxa, vfti.dxpInch, dxaInch);
		vfli.fVolatile = fTrue;
	case chPicture:
		return;
		}
	cch = CchChSpec(ch, rgch);
/* sum up the width of cch characters */
	dxp = dxt = 0;
	fDxt = (!vfli.fPrint && vfli.fFormatAsPrint);
	for (pch = rgch, ich = 0; ich < cch; ++ich)
		{
		dxp += (dxpT = DxpFromCh(ch = *pch++, &vfti));
		dxt += (fDxt) ? DxpFromCh(ch, &vftiDxt) : dxpT;
		}
	*pdxt = dxt;
	*pdxp = dxp;
}


/* C C H  C H  S P E C */
/* return string expansion of special char ch based on FMTSS state.
State ensures that FormatLine and DisplayFli will see the same results.
	chPage: use pgn in state, nfcPgn from vsepFetch.
	chDate: use tick in state.
	chTime: use tick in state.
	chFootnote: use doc, cpRef to decode auto reference number.
*/
/* %%Function:CchChSpec %%Owner:bryanl */
int CchChSpec(ch, pch)
int ch;
char *pch;
{
	char *pch0 = pch;
	int doc = vfli.doc;
	struct DOD *pdod;
	CP cp = vfmtss.cpRef;
	char st[cchMaxSz];

#ifdef WIN
	/*
	 * If vfli.doc is docUndo, we are printing either a selection
	 * of the main doc or some weird entity (stsh, glossary, etc.).
	 * Change doc from docUndo to the real doc so chFtns and chAtns
	 * are evaluated correctly.
	 */
	if (vfli.doc == docUndo && (ch == chFootnote || ch == chAtn))
		{
		Assert(vlm == lmPrint);
		pdod = PdodDoc(docUndo);

		/* change to the real doc...could be either the mother doc
		       or docAtn */
		doc = pdod->doc;
		if (vpri.fPrintingAtn)
			{
			doc = PdodDoc(doc)->docAtn;
			Assert(doc != docNil);
			}
		Assert(PdodDoc(doc)->fMother || PdodDoc(doc)->fAtn);

		cp = vfmtss.cpRef + vpri.cpAdjust;
		}
#endif	 /* WIN */

	switch (ch)
		{
#ifdef MAC
	case chPage:
		vfli.fVolatile = fTrue;
		return CchIntToRgchNfc(vfmtss.pgn, pch,
				vsepFetch.nfcPgn, cchChSpecMax);
	case chDateShort:
	case chDateLong:
	case chDateAbbr:
		IUDateString(vfmtss.tick, (ch == chDateShort) ? dtfShort : (ch - chDateLong + dtfLong),
				st);
		goto LRetSt;
	case chTimeHM:
	case chTimeHMS:
		IUTimeString(vfmtss.tick, (ch == chTimeHMS) /* Seconds? */ ? PTRUE : fFalse, st);
LRetSt:         
		bltb(&st[1], pch, st[0]);
		vfli.fVolatile = fTrue;
		return(st[0]);
#endif /* MAC */
#ifdef WIN
	case chAtn:
		vfli.fVolatile = fTrue;
		Assert(vfli.doc != docNil);
		return CchAtnToPpch(doc, cp, &pch);
	case chFieldSeparate:
		*pch = chFieldSeparateDisp;
		return 1;
	case chFieldEnd:
		*pch = chFieldEndDisp;
		return 1;

	/* following come here only for Save As Plain Text */
	case chPicture:
		return 0;
	case chFieldBegin:
		*pch = chFieldBeginDisp;
		return 1;

#endif	/* WIN */
	case chFootnote:
		vfli.fVolatile = fTrue;
		return CchIntToPpch(WinMac(NAutoFtn(doc, cp), NAutoFtn(vfmtss.doc, vfmtss.cpRef)), &pch);
	case chLnn:
		return CchIntToPpch(vfmtss.lnn, &pch);
	default:
		*pch++ = '?';
		*pch = '?';
		return 2;
		}
}


/* N  A U T O  F T N */
/* returns auto footnote number for reference / or start of
referenced text in document doc.
*/
/* %%Function:NAutoFtn %%Owner:bryanl */
int NAutoFtn(doc, cp)
int doc; 
CP cp;
{
	int n = 1; /* default starting number */
	int ifrd, ifrdFirst;
/* first determine doc, cp, pdod for reference */
	struct DOD *pdod = PdodDoc(doc);
	struct PLC **hplcfrd;

	if (pdod->fFtn)
		{
		ifrd /*ifnd*/ = IInPlc(pdod->hplcfnd, cp);
		doc = pdod->doc;
		pdod = PdodDoc(doc);
		hplcfrd = pdod->hplcfrd;
		cp = CpPlc(hplcfrd, ifrd);
		}
	else  if (pdod->fShort)
		{
		return(0);
		}
	else
		{
/* n=0 is returned if footnote reference was moved to document with no
footnotes */
/* we can have a hplcfrd but iMac is 0 because the last footnote is deleted */
		if ((hplcfrd = pdod->hplcfrd) == 0 || IMacPlc(hplcfrd) == 0)
			return 0;
		ifrd = IInPlcRef(hplcfrd, cp);
		}
/* reference number is function of dop.fpc and fFtnRestart */
/* determine starting n and interval [ifrdFirst, ifrd) of increments */
	if (!pdod->dop.fFtnRestart)
		{
/* numbering continuous over all bounds */
LCont:		
		n = pdod->dop.nFtn;
		ifrdFirst = 0;
		}
	else
		{
/* determine cp where numbering restarts */
#ifdef MAC
		if (pdod->dop.fpc == fpcEndDoc)
			cp = cp0;
		else  if (pdod->dop.fpc != fpcEndnote)
			{
/* per page numbering */
			CachePage(doc, cp);
			cp = caPage.cpFirst;
			}
		else
	/* per division numbering. Start with first division or division following
	a division with the fEndnote property.
*/
			{
			CacheSect(doc, cp);
			do
				{
				if (caSect.cpFirst == 0) goto LCont;
				cp = caSect.cpFirst - 1;
				CacheSect(doc, cp);
				} 
			while (!vsepFetch.fEndnote);
			cp = caSect.cpLim;
			}
#else
		CacheSect(doc, cp);
		cp = caSect.cpFirst;
#endif
		ifrdFirst = IInPlcRef(hplcfrd, cp);
		}

	MiscPlcLoops(hplcfrd, ifrdFirst, ifrd,
			&n, 1 /* NAutoFtn */);
	return n;
}


#ifdef WIN
/* %%Function:CchAtnToPpch %%Owner:bryanl */
CchAtnToPpch(doc, cp, ppch)
int doc;
CP cp;
CHAR **ppch;
{
	int cchUsrInitl = 0;
	int cchInt;
	CHAR *pch;
	struct DOD *pdod;
	struct DRP *pdrp;
	struct PLC **hplcatrd;
	struct ATRD atrd;
	int iatrd = -1;

	pch = *ppch;
	*pch++ = '[';
	pdod = PdodDoc(doc);
	hplcatrd = PdodMother(doc)->hplcatrd;

	FreezeHp();
	if (hplcatrd != hNil && (pdod->fMother || pdod->fAtn))
		{ /* can't get a usrInitl from other short doc */
		if (pdod->fAtn)
			iatrd = IInPlc( pdod->hplcand, cp );
		else
			{
			iatrd = IInPlcRef(hplcatrd, cp);
			if (iatrd != IInPlcCheck(hplcatrd, cp))
				iatrd = -1;
			}

		if (iatrd >= 0)
			{
			GetPlc( hplcatrd, iatrd, &atrd );
			pch = bltbyte(&atrd.stUsrInitl[1], pch, cchUsrInitl = atrd.stUsrInitl[0]);
			Assert(cchUsrInitl >= 0 && cchUsrInitl < ichUsrInitlMax);
			}
		}
	MeltHp();

	cchInt = CchIntToPpch(iatrd + 1, &pch);
/* Note : pch points past the last character in the string after the call. */
	*pch++ = ']';
	*pch = '\0';
	return(cchUsrInitl+cchInt+2);
}


#endif	/* WIN */


/* F o r m a t   L i n e   F s p e c */
/* %%Function:FFormatLineFspec %%Owner:bryanl */
EXPORT FFormatLineFspec(ww, doc, dxl, ch)
int ww, doc, dxl, ch;
{
/* format a line containing the fSpec character ch, with styles from doc */
#ifdef MAC
	int stc = (ch == chPage) ? stcPgn : (ch == chLnn) ? stcLnn : stcNormal;
#else
	int stc = (ch == chLnn) ? stcLnn : stcNormal;
#endif /* MAC */
	int dxa;

	caPara.doc = docNil;    /* in case caPara.doc == vdocTemp */
	InvalFli();
	if (!FCreateChSpec(doc, ch))
		return fFalse;
	MapStcToTempDoc(doc, stc);
	LinkDocToWw(vdocTemp, wwTemp, ww);

#ifdef WIN  /* no grpfvisi in wwd for MAC */
	if (ww != wwNil)
		{ /* so that vfli.grpfvisi get the latest grpfvisi */
		PwwdWw(wwTemp)->grpfvisi = PwwdWw(ww)->grpfvisi;
		}
	else
		PwwdWw(wwTemp)->grpfvisi.flm = vflm;
#endif

	dxa = (ch != chTFtn && ch != chTFtnCont) ? xaRightStd :
			DxaFromDxp(PwwdWw(ww), dxl);
	FormatLineDxa(wwTemp, vdocTemp, cp0, dxa);
	return fTrue;
}



#ifdef WIN

#define ichPubSimMax	6		// Max characters to use to simulate a pub char

/* F o r m a t  C h  P u b s */
/* determine width for publishing chars */

/* %%Function:FormatChPubs %%Owner:bryanl */
EXPORT FormatChPubs( ch, pdxp, pdxt )
int ch;
int *pdxp, *pdxt;
{
	char rgch [ichPubSimMax];
	int rgdxp [ichPubSimMax];
	int cch;

	Assert( ch >= chPubMin && ch < chPubMax );

	cch = CchExpandChPubs( rgch, rgdxp, pdxp, ch, &vfti );
	if (vfli.fFormatAsPrint)
		cch = CchExpandChPubs( rgch, rgdxp, pdxt, ch, &vftiDxt );
	else
		*pdxt = *pdxp;
}


/* C c h  E x p a n d  C h  P u b s */
/* generate simulated ANSI equivalents for publishing chars */

/* %%Function:CchExpandChPubs %%Owner:bryanl */
CchExpandChPubs( rgch, rgdxp, pdxpSum, ch, pfti )
char rgch[];
uns rgdxp[];
int *pdxpSum;
int ch;
struct FTI *pfti;
{
	int cch = 1;
	int dxpT;
	uns dxpHyphen;
	uns dxpHyphenFull;
	int ich;
	int dxpSum = 1;
	BOOL fDxpFromRealCh = fFalse;	/* Use the width supplied in the fti? */

	// If we're looking at a printer fti, and the printer supports the
	// publishing characters, get the pub char width from DxpFromCh(ch).
	if (pfti->fPrinter)
		fDxpFromRealCh = vpri.fSupportPubChars;

	switch (ch)
		{
	default:
		Assert( fFalse );
		break;

	case chPubLDblQuote:
		*((int *) rgch) = chLQuote + (chLQuote << 8);
		goto LDblQuote;

	case chPubRDblQuote:
		*((int *) rgch) = chRQuote + (chRQuote << 8);
LDblQuote:
		cch++;
		if (fDxpFromRealCh)
			{
			// Get the width of the actual char, then for simulation, give
			// the second single quote 4/7 of the total width, and the rest
			// to the first single quote.
			dxpSum = DxpFromCh(ch, pfti);
			rgdxp [1] = (dxpSum << 2) / 7;	/* Four sevenths of dxpSum */
			rgdxp [0] = dxpSum - rgdxp[1];
			}
		else
			{
			rgdxp [0] = rgdxp [1] = DxpFromCh( rgch [0], pfti );
			rgdxp [0] -= rgdxp [0] >> 2;	/* push quotes together by 1/4 of width */
			dxpSum = rgdxp [0] + rgdxp[1];
			}
		break;

	case chPubBullet:
		rgch [0] = chSimBullet;
		rgdxp [0] = dxpSum = DxpFromCh( fDxpFromRealCh ? ch : rgch [0], pfti );
		break;

	case chPubEmDash:
		dxpT = DxpFromCh( fDxpFromRealCh ? chPubEmDashPS : 'm', pfti );
LDashes:
		SetBytes( rgch, chHyphen, ichPubSimMax );
		dxpHyphenFull = DxpFromCh( chHyphen, pfti );
		dxpHyphen = dxpHyphenFull - (dxpHyphenFull / 3);	/* compensate for space after */
		/* Find how many two-third length hyphens it takes to fill dxpT
		 * minus the length of a full hyphen */
		cch = min( ichPubSimMax - 1,
				(max(0, dxpT + dxpHyphen - dxpHyphenFull - 1) / dxpHyphen ));
		SetWords( rgdxp, dxpHyphen, cch );
		/* Last hyphen will be full length */
		rgdxp[cch] = dxpHyphenFull;
		dxpSum = dxpHyphen * cch + dxpHyphenFull;
		cch++;
		if (cch > 1 && dxpSum > dxpT)
			{
			rgdxp [cch-2] -= dxpSum - dxpT;
			dxpSum = dxpT;
			}
		break;

	case chPubEnDash:
		dxpT = DxpFromCh( fDxpFromRealCh ? chPubEnDashPS : 'n', pfti );
		goto LDashes;
		}

	Assert( cch <= ichPubSimMax );
	*pdxpSum = dxpSum;

	return cch;
}


/* D r a w  C h  P u b s */
/* Draw publishing chars */

/* %%Function:DrawChPubs %%Owner:bryanl */
EXPORT DrawChPubs( hdc, ppt, ch, prcOpaque, prcErase, fErased, eto )
HDC hdc;
struct PT *ppt;
int ch;
struct RC *prcErase, *prcOpaque;
int eto;
{
	int cch;
	struct RC rcT;
	int rgdxp [ichPubSimMax];
	char rgch [ichPubSimMax];
	int xp = ppt->xp;
	int xpNew;
	int yp = ppt->yp - vfti.dypAscent;
	int dxp;
	int wCharSet = 1;

	cch = CchExpandChPubs( rgch, rgdxp, &dxp, ch, &vfti );
	xpNew = xp + dxp;

	rcT = *prcOpaque;
	if (!fErased)
		rcT.xpRight = prcErase->xpLeft = xpNew;

	if (vfli.fPrint)
		{
		if (Escape(hdc, SETCHARSET, NULL, (LPINT)&wCharSet,(LPINT)NULL))
			{	/* Driver supports special pub chars escape */
			Assert(vpri.fSupportPubChars);
#ifdef BRYANL
			CommSzNum( SzShared( "Printer supports SETCHARSET, using it for publishing char: "), ch );
#endif
			// If printing a Em or En dash, print the proper Postscript character
			switch (ch)
				{
			case chPubEmDash:
				ch = chPubEmDashPS;
				break;
			case chPubEnDash:
				ch = chPubEnDashPS;
				break;
				}

			/* The line below was changed without talking to BryanL.  I believe
			   that the parameter passed should be wCharSet rather than
			   &wCharSet (the previous parameter).  I have interpreted the
			   comment to mean the writer is asking forgiveness for trying
			   to save code space while making the entire line unintelligible */
			ExtTextOut( hdc, xp, yp, 0, (LPRECT) 0, 
					(LPCH)&ch, wCharSet/*sorry*/,
					(LPINT) 0 );
			wCharSet = 0;
			Escape( hdc, SETCHARSET, NULL, (LPINT)&wCharSet, (LPINT)NULL);
			}
		else
			{
			/* no special driver support, simulate */
			Assert(!vpri.fSupportPubChars);
#ifdef BRYANL
			CommSzNum( SzShared( "Printer does not support SETCHARSET, simulating pub char: "), ch );
#endif
			ExtTextOut( hdc, xp, yp, 0, 
					(LPRECT) 0,
					(LPCH)rgch, cch, (LPINT)rgdxp );
			if (vfti.fcid.kul)
				{
				int dxpSpace;
				char rgchT[2];

				dxpSpace = vfti.rgdxp[(int) ' '];
				if (dxpSpace <= dxp)
					{
					rgchT[0] = ' ';
					TextOut( hdc, xpNew - dxpSpace, yp, (LPCH) rgchT, 1);
					}
				}
			}
		}
	else
		ExtTextOut( hdc, xp, yp, eto, 
				(LPRECT) &rcT,
				(LPCH)rgch, cch, (LPINT)rgdxp );

	if (!fErased)
		prcOpaque->xpLeft = xpNew;

	ppt->xp = xpNew;
}


#endif	/* WIN */



/* S H O W  S P E C */
/* show the special char ch */
/* %%Function:ShowSpec %%Owner:bryanl */
EXPORT ShowSpec(ww, ich, pptPen, yp, pchp, puls)
int ww;
int ich;
struct PT * pptPen;
int yp;
struct CHP *pchp;
struct ULS *puls;
{
	int ch = vfli.rgch[ich];
	int cch;
	int dxpChSpec, dxpSpace, cchSpaces, cchOut;
	int xp;
	HDC hdc = PwwdWw(ww)->hdc;
	CHAR rgch[ichChSpecMax];

	if (ch > ' ')
		{
		ShowSpecSymbol(ich, pchp, puls, hdc, pptPen);
		return;
		}

	Assert(ch <= chLnn || (ch < chFieldMax && ch >= chFieldMin));

	switch (ch)
		{
	case chPicture:
		/* if printing but not in a graphics band, don't! */
		/* except print metafiles anyway since they might have text */
		if (vfli.fPrint && !vpri.fGraphics && !vfli.fMetafile)
			return;
		/* Warning DrawChPic may move heap. pchp is heap pointer */
		DrawChPic(ww, ich, pptPen, yp, pchp);
		return;
	case chTFtn:
	case chTFtnCont:
/* show footnote separators as strikethru spaces */
		dxpChSpec = vfli.rgdxp[ich];
		dxpSpace = DxpFromCh(chSpace, &vfti);
		SetBytes(rgch, chSpace, ichChSpecMax);
		xp = pptPen->xp;
		cchSpaces = (dxpChSpec + dxpSpace - 1)/dxpSpace;
		if (vfPrvwDisp)
			{
			DrawPrvwLine(hdc, xp, yp+vfli.dypLine/2,dxpChSpec,1,colFetch);
			return;
			}
		while (cchSpaces > 0)
			{
			cchOut = cchSpaces > ichChSpecMax ? ichChSpecMax:cchSpaces;
			TextOut( hdc, xp, yp, (LPSTR) &rgch, cchOut );
			xp += cchOut * dxpSpace;
			cchSpaces -= cchOut;
			}
		return;
		}

	Assert(ch == chLnn || ch == chFootnote || ch == chAtn
			|| (ch >= chFieldMin && ch < chFieldMax));
	cch = CchChSpec( ch, rgch );
	TextOut( hdc, pptPen->xp, yp, (LPSTR) &rgch, cch );
}


/* %%Function:DrawDottedLineBox %%Owner:bryanl */
EXPORT DrawDottedLineBox(hdc, rcBox)
HDC hdc;
struct RC rcBox;
{
	int plt = pltOnePixel;
	int dzp;

/* Draw vertical lines */
	dzp = rcBox.ypBottom - rcBox.ypTop;
	DrawPatternLine(hdc, rcBox.xpLeft, rcBox.ypTop, dzp, ipatVertGray, plt+pltVert);
	DrawPatternLine(hdc, rcBox.xpRight, rcBox.ypTop, dzp, ipatVertGray, plt+pltVert);
/* Draw horizontal lines first */
	dzp = rcBox.xpRight - rcBox.xpLeft;
	DrawPatternLine(hdc, rcBox.xpLeft, rcBox.ypTop, dzp, ipatHorzGray, plt+pltHorz);
	DrawPatternLine(hdc, rcBox.xpLeft, rcBox.ypBottom, dzp, ipatHorzGray, plt+pltHorz);
}


