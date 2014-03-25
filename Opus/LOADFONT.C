/* loadfont.c - MW font support code */

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "props.h"
#include "disp.h"
#include "doc.h"
#include "format.h"
#include "print.h"
#include "screen.h"
#include "sel.h"
#include "ch.h"
#include "debug.h"

#ifdef PCJ
/* #define DFONT */
#endif /* PCJ */
#ifdef BRYANL
#define DFONT
#endif

extern struct STTB  **vhsttbFont;
extern struct SAB  vsab;
extern int          wwMac;
extern struct WWD   **mpwwhwwd[];
extern struct DOD   **mpdochdod[];
extern struct MERR  vmerr;
extern struct PRI   vpri;
extern struct SCI   vsci;
extern struct SEL   selCur;

extern struct FLI   vfli;
extern int          vlm;
extern int          vfPrvwDisp;
extern int          vifceMac;
extern struct FCE   rgfce[ifceMax];

extern struct FTI   vfti;
extern struct FTI   vftiDxt;
extern struct PREF  vpref;
extern struct CHP   vchpStc;

extern CHAR         szEmpty[];


HANDNATIVE struct FCE *PfceLruGet();


/*  LoadFont( pchp, fWidthsOnly )
*
*  Description: LoadFont loads the font specified by pchp into appropriate DC's
*/

#ifdef DEBUG
/* %%Function:C_LoadFont %%Owner:bryanl */
HANDNATIVE C_LoadFont( pchp, fWidthsOnly )
struct CHP *pchp;
int fWidthsOnly;
{
	union FCID fcid;
	struct DOD *pdod;

	Assert( vfli.doc != docNil );
	Assert( pchp != NULL);

	/* Map font requests in short DOD back through parent DOD */
	pdod = PdodMother(vfli.doc);

	/* in Print, map docUndo to real mother doc */
	if (vfli.doc == docUndo)
		{
		Assert(vlm == lmPrint);
		Assert(PdodDoc(docUndo)->doc != docNil);
		pdod = PdodDoc(PdodDoc(docUndo)->doc);
		}

	/* font table was not actually copied to scrap, so map request back to 
		data provider doc, just like MapStc does for styles */

	else  if (pdod == PdodDoc(docScrap) && vsab.docStsh != docNil)
		pdod = PdodDoc(vsab.docStsh);

	/* Translate chp --> fcid */
	fcid.lFcid = 0L;
	fcid.fItalic = pchp->fItalic;
	fcid.fBold = pchp->fBold;
	fcid.fStrike = pchp->fStrike;
#ifdef DFONT
	if ((uns)pchp->ftc >= pdod->ftcMac)
		{
		int rgw [3];

		rgw [0] = pchp->ftc;
		rgw [1] = pdod->ftcMac;
		rgw [2] = vfli.doc;
		CommSzRgNum(SzFrame("Font ref out of range, (ftc,ftcMac,doc): "),rgw, 3);
		}
#endif
/* protect against out-of-range ftcs in case of bogus doc or
	failed ApplyGrpprlCa during font merge after interdoc paste */
	fcid.ibstFont = (**pdod->hmpftcibstFont) [min( pchp->ftc, pdod->ftcMac-1)];
	Assert( (uns)fcid.ibstFont  < (*vhsttbFont)->ibstMac );
	fcid.hps = pchp->hps;
	fcid.kul = pchp->kul;

	vfti.dxpExpanded = vftiDxt.dxpExpanded = 0;
	if (vfli.fFormatAsPrint)
		{   /* Two-device case: Displaying; formatting for printer */

		/* First determine what font we can get for the printer */
		C_LoadFcid( fcid, &vftiDxt, fTrue );
		if (pchp->qpsSpace != 0)
			vftiDxt.dxpExpanded = C_DxuExpand(pchp,vftiDxt.dxpInch);

		/* Then request a screen font based on what we got back */
		C_LoadFcid( vftiDxt.fcid, &vfti, fWidthsOnly );
		}
	else
		{   /* One-device case: 1. Printing
							2. Displaying; formatting for screen */
		C_LoadFcid( fcid, &vfti, fWidthsOnly );
		}
	if (pchp->qpsSpace != 0)
		{
		vfti.dxpExpanded = C_DxuExpand(pchp,vfti.dxpInch);
#ifdef DFONT
		CommSzNum( SzShared( "Font expansion in pixels = "),vfti.dxpExpanded );
#endif
		}
}


#endif	/* DEBUG */


#ifdef DEBUG
/* %%Function:C_LoadFcidFull %%Owner:bryanl */
HANDNATIVE C_LoadFcidFull( fcid )
union FCID fcid;
{
	C_LoadFcid( fcid, &vfti, fFalse /* fWidthsOnly */);
}


#endif	/* DEBUG */


/* LoadFcid( fcid, pfti, fWidthsOnly )
*
* Description:
*
* Loads a font described by fcid. Sets up *pfti to describe the font, and
* selects the font into pfti->hdc
*
* Inputs:
*  fcid - a long describing the font
*  pfti - pointer to fti structure to fill out
*
*  pfti->hdc - If fPrinter is set, this is the printer DC.
*              If not, this is some document DC, or maybe the memory DC --
*              some screen DC into which FSelectFont selects.
*
* Outputs:
*
* *pfti is filled out with a description of the font actually realized for
* the request.  Note that it is frequently true that pfti->fcid != fcid passed
*
* Methods:
*
*  The last ifceMax fonts requested through LoadFcid are kept in a LRU cache,
*  such that the descriptive information, width tables, and mapping from
*  requested to actual font are kept around for quick access.
*
*  The fonts that are cached for the device described in *pfti are
*  kept on a chain extending from pfti->pfce.
*
*  If a font is currently selected into the device, then pfti->hfont
*  will be non-null and pfti->fcid will describe the font.
*
*  The fcid acts as the key field in looking up a font.  FCID is structured
*  so that the first word, wProps, contains those fields considered most
*  likely to vary.
*
*/

#ifdef DEBUG
/* %%Function:C_LoadFcid %%Owner:bryanl */
HANDNATIVE C_LoadFcid( fcid, pfti, fWidthsOnly )
union FCID fcid;
struct FTI *pfti;
int fWidthsOnly;
{
	extern int vflm;
	int dyp;
	struct FCE *pfce;
	struct FCE *pfceHead;
	int fFallback;
	LOGFONT lf;
	TEXTMETRIC tm;
	int dxp;
	HDC hdc;

	Assert( vflm != flmIdle );

/* CASE 1: No fonts cached for this device */

	if ((pfce = pfti->pfce) == NULL)
		{
		goto LNewFce;
		}

/* CASE 2: There is a font already selected in, and it matches the request */

	if (pfti->hfont != NULL)
		{
		if (fcid.wProps == pfce->fcidRequest.wProps &&
				fcid.wExtra == pfce->fcidRequest.wExtra)
			{
			vfli.fGraphics |= pfce->fGraphics;
			return;
			}

		pfce = pfce->pfceNext;  /* start following search with 2nd font in chain */
		}

/* CASE 3: Font request matches some cached request for this device */

	while (pfce != NULL)
		{
		if (pfce->fcidRequest.wProps == fcid.wProps &&
				pfce->fcidRequest.wExtra == fcid.wExtra)
			{
			HFONT hfontT;

			Assert( pfce->hfont != NULL );

			if (vfPrvwDisp && !pfce->fPrvw || !vfPrvwDisp && pfce->fPrvw)
				goto LNextFce;

			/* Remove the FCE from its present position in its chain */

			if (pfce == pfti->pfce)
				{
				pfti->pfce = pfce->pfceNext;
				}
			else
				{
				Assert( pfce->pfcePrev != NULL );
				if ((pfce->pfcePrev->pfceNext = pfce->pfceNext) != 0)
					pfce->pfceNext->pfcePrev = pfce->pfcePrev;
				}

		/* Optimization: don't actually select in the font if we know 
					its width and we're just formatting so that's all we need */

			if (fWidthsOnly)
				pfti->hfont = NULL;
			else  if (pfce->hfont == hfontSpecNil)
				goto LValidateFce;
			else  if (hfontT = pfce->hfont, 
					!FSelectFont( pfti, &hfontT, &hdc))
/* font could not be selected so system font was selected in its place */
				{
				FreeHandlesOfPfce(pfce);
#ifdef DFONT
				CommSzSz(SzFrame("Could not re-select cached font"),szEmpty);
#endif
				goto LSystemFontErr;
				}

			goto LLoadFce;
			}
LNextFce:
		pfce = pfce->pfceNext;
		}

/* CASE 4: No FCE in table for requested font, must build one */

LNewFce:

/* Get a free FCE slot in which to build new entry */

	pfce = PfceLruGet();   /* Kicks out LRU font, if no unallocated FCE slots exist */

LValidateFce:

	pfce->fPrvw = vfPrvwDisp;
	fFallback = fFalse;
	pfce->fPrinter = pfti->fPrinter;
	pfce->fcidRequest = fcid;
	pfce->fGraphics = fTrue;	/* the safer assumption in case of failure */

	if (pfti->fTossedPrinterDC)
		{
		Assert( pfti->fPrinter );
		Assert( fWidthsOnly );
/* up till now, we have cheated -- we don't really have a printer DC,
	but we have been able to manage without it since our cache is filled
	with font widths.  Now comes the time to pay the piper.
	Create a printer IC. */
		if (!FReviveTossedPrinterDC(pfce))
			goto LLoadFce;
		}

/* Translate FCID request to a logical font */

	if (fcid.ibstFont == ibstFontNil || (!pfti->fPrinter && vpref.fDraftView))  /* ibstFontNil means use system font */
		goto LSystemFont;

	pfce->fGraphics = FGraphicsFcidToPlf( fcid, &lf, pfce->fPrinter );

/* Call Windows to request the font */

	if ((pfce->hfont = CreateFontIndirect( (LPLOGFONT)&lf )) == NULL)
		{
#ifdef DFONT
		CommSzSz(SzFrame("Failed to create logical font!"),szEmpty);
#endif
LSystemFontErr:
		SetErrorMat(matFont);
		fFallback = fTrue;
LSystemFont:
		pfce->hfont = GetStockObject( pfti->fPrinter ?
				DEVICEDEFAULT_FONT : SYSTEM_FONT );
		}
	LogGdiHandle(pfce->hfont, 10000);

#ifdef DFONT
	CommSzNum(SzShared("Font handle: "), pfce->hfont);
#endif /* DFONT */

/* Now fill out the FCE entry with the attributes of the new font */
/* A false return from FSelectFont means we got the system font instead,
	but we don't care here */
	FSelectFont( pfti, &pfce->hfont, &hdc );

LNewMetrics:
	Assert( hdc != NULL );
	GetTextMetrics( hdc, (LPTEXTMETRIC) &tm );

#ifdef DFONT
		{
		char szFace[256];
		int rgw [7];


		GetTextFace(hdc, LF_FACESIZE, (LPSTR)szFace);
		CommSzSz(SzShared("  Actual face name: "), szFace);
		rgw [0] = tm.tmHeight - tm.tmInternalLeading;
		rgw [1] = tm.tmExternalLeading;
		rgw [2] = tm.tmInternalLeading;
		rgw [3] = tm.tmAscent;
		rgw [4] = tm.tmDescent;
		CommSzRgNum( SzShared("  Actual ht(no ldng), e-ldng, i-ldng, asc (w/i-ldng), desc: "),
				rgw, 5 );

		CommFontAttr( SzShared( "  Actual Attributes: "),
				tm.tmItalic, tm.tmStruckOut, tm.tmUnderlined, tm.tmWeight > FW_NORMAL);

		rgw [0] = tm.tmPitchAndFamily >> 4;
		rgw [1] = tm.tmPitchAndFamily & 1;
		rgw [2] = tm.tmCharSet;
		rgw [3] = tm.tmAveCharWidth;
		rgw [4] = tm.tmOverhang;
		CommSzRgNum(SzShared("  Actual font family, pitch, charset, avg wid, overhang: "),
				rgw, 5 );
		}
#endif /* DFONT */

	pfce->dypAscent =   tm.tmAscent;
	pfce->dypDescent =  tm.tmDescent;
	pfce->dypXtraAscent = tm.tmAscent;
	if (pfti->fPrinter)
		pfce->dypXtraAscent += tm.tmExternalLeading;
	pfce->dxpOverhang = tm.tmOverhang;

/* Initialize the width table.
		Fixed Pitch: store fixed width
		Variable Pitch, store width table
*/

	Debug( vdbs.fShakeHeap ? ShakeHeap() : 0 );

	pfce->fFixedPitch = fTrue;
	pfce->dxpWidth = tm.tmAveCharWidth;
	pfce->fVisiBad = fFalse;
	if ((tm.tmPitchAndFamily & maskFVarPitchTM) && !fFallback)
		{
		int ch;     /* MUST be int so chDxpMax == 256 will work */
		int far *lpdxpT;
		int far *lpdxpMin;
		int idxp;
		int FAR *lpdxp;
		int mpchdxp [chDxpMax];

		if ((pfce->hqrgdxp = HqAllocLcb( (long)((chDxpMax - chDxpMin ) << 1))) == hqNil)
			{
			Assert( pfce->hfont != GetStockObject( pfti->fPrinter ?
					DEVICEDEFAULT_FONT : SYSTEM_FONT ));
#ifdef DFONT
			CommSzSz(SzFrame("Failed to allocate space for font widths!"),szEmpty);
#endif
/* lose track of logical font & hqrgdxp, but that's tolerable */
			goto LSystemFontErr;
			}
		lpdxp = LpLockHq( pfce->hqrgdxp );
		Assert( lpdxp != NULL );	/* Should be guaranteed */

/* WINDOWS BUG WORKAROUND (DAVIDBO): GetCharWidth RIPs on any mapping mode
	other than MM_TEXT */
		if (vfPrvwDisp)
			OurGetCharWidth( hdc, chDxpMin, chDxpMax - 2, lpdxp);
/* DRIVER BUG WORKAROUND (BL): Don't ask for n-255 or the EPSON24 and 
	several other raster printer drivers will crash when you ask for widths */
		else  if (!GetCharWidth( hdc, chDxpMin, chDxpMax - 2, lpdxp ))
			{
			ReportSz("Driver does not support GetCharWidth!");
			OurGetCharWidth( hdc, chDxpMin, chDxpMax - 2, lpdxp );
			}
		OurGetCharWidth( hdc, chDxpMax - 1, chDxpMax - 1, &lpdxp [chDxpMax-1] );
		if (pfce->dxpOverhang)
			{
			lpdxpMin = lpdxp;
			lpdxpT = lpdxpMin + (chDxpMax - chDxpMin - 1);
			while (lpdxpT >= lpdxpMin)
				{
				*lpdxpT-- -= pfce->dxpOverhang;
				}
			}
		pfce->fFixedPitch = fFalse;
		pfce->fVisiBad = (lpdxp [chSpace] != lpdxp [chVisSpace]);
		UnlockHq( pfce->hqrgdxp );
		}


/* Now built fcidActual according to the properties of the font we got */
/* We only care about fcidActual for printer fonts */

	if (pfti->fPrinter)
		{
		union FCID fcidActual;
		CHAR rgb [cbFfnLast];
		struct FFN *pffn = (struct FFN *) &rgb [0];

		fcidActual.lFcid = 0L;

		/* Store back the real height we got */

		fcidActual.hps = umin( 0xFF,
				(NMultDiv( tm.tmHeight - tm.tmInternalLeading, czaInch,
				vfli.dyuInch ) + (czaPoint / 4)) / (czaPoint / 2));

		if (tm.tmWeight > (FW_NORMAL + FW_BOLD) / 2)
			fcidActual.fBold = fTrue;

		if (tm.tmItalic)
			fcidActual.fItalic = fTrue;
		if (tm.tmStruckOut)
			fcidActual.fStrike = fTrue;

/* following line says: if we got back underlining, we can do double underlining too.
	That's because we print double underlines by printing underlined spaces 
	just below the first underline. Also let dotted ul through
	regardless because it is only used on the screen */
		if (tm.tmUnderlined || fcid.kul == kulDotted)
			fcidActual.kul = fcid.kul;

		fcidActual.prq = (tm.tmPitchAndFamily & maskFVarPitchTM)
				? VARIABLE_PITCH : FIXED_PITCH;

/* For printer fonts, we may get back a font with a different name than
	the one we requested.
	If this in fact happened, add the new name to the master font table
*/

		GetTextFace( hdc, LF_FACESIZE, (LPSTR) pffn->szFfn );

		if (!FNeNcSz(pffn->szFfn, lf.lfFaceName))
			{
			/* The face name is the same as what we requested; so, the
			font index should be the same. */
			fcidActual.ibstFont = pfce->fcidRequest.ibstFont;
			}
		else
			{   /* Font supplied by printer is different from request */
			int ibst;

			pffn->ffid = tm.tmPitchAndFamily & maskFfFfid;
			pffn->cbFfnM1 = CbFfnFromCchSzFfn( CchSz( pffn->szFfn ) ) - 1;
			ChsPffn(pffn) = tm.tmCharSet;

			if ((ibst = IbstFindSzFfn( vhsttbFont, pffn )) == iNil)
/* Font supplied by printer is not in table, add it */
	/* note: If ibstNil is returned from IbstAddStToSttb that's fine,
		it just means we will select the system font whenever we use the
	result. */
				{
				Assert ((ibstNil & 0xFF) == ibstFontNil);
				ibst = IbstAddStToSttb( vhsttbFont, pffn );
				}

			fcidActual.ibstFont = ibst;
			}
		pfce->fcidActual = fcidActual;
		}
	else
		{   /* Screen font - don't care about fcidActual */
		if (vpref.fDraftView && (fcid.fBold 
				|| fcid.fItalic 
				|| fcid.kul != kulNone 
				|| fcid.fStrike))
			fcid.kul = kulSingle;
		pfce->fcidActual = fcid;
		}


LLoadFce:

/* Put pfce at the head of the chain extending from pfti */
/* When we get here, pfce is not in any chain */

	pfceHead = pfti->pfce;
	pfti->pfce = pfce;
	pfce->pfcePrev = NULL;
	pfce->pfceNext = pfceHead;
	if (pfceHead != NULL)
		pfceHead->pfcePrev = pfce;

/* Load info about fce into *pfti */

	vfli.fGraphics |= pfce->fGraphics;
	bltbyte( pfce, pfti, cbFtiFceSame );
	if (pfce->fFixedPitch)
		SetWords( pfti->rgdxp, pfce->dxpWidth, 256 );
	else
		bltbh( HpOfHq( pfce->hqrgdxp ), (int HUGE *)pfti->rgdxp, 512 );

	Debug( vdbs.fCkFont ? CkFont() : 0 );
}


#endif	/* DEBUG */

#ifdef DEBUG /* Done in a near routine in formatn2.asm */
/* O U R  S E L E C T  O B J E C T */
/*  performs a SelectObject, reducing the swap area SOME if it fails */
/* %%Function:OurSelectObject %%Owner:bryanl */
OurSelectObject(hdc, h)
HDC hdc;
HANDLE h;
{
	extern int vsasCur;
	HANDLE hRet = SelectObject(hdc, h);

	if (hRet == NULL && vsasCur <= sasMin)
		{
		/* reduce swap area to an acceptable level (won't swap too much, but
			may have more room for fonts) */
		OurSetSas(sasMin);
		hRet = SelectObject(hdc, h);
		/* increase the swap area back as far as we can */
		OurSetSas(sasFull);
		}

	return hRet;
}


#endif	/* DEBUG */

/* FSelectFont( pfti, phfont, phdc )
*
* Description:
*
* Selects the font hfont into the DC(s) for the screen (!pfti->fPrinter) or
* the printer (pfti->fPrinter).
*
* Inputs:
*
* pfti		Specifies device (screen or printer) into which to select font
*
* Outputs:
*
* pfti->hfont  Set to *phfont
* *phfont      if return is FALSE, may differ from provided value
*
* Returns TRUE if the select succeeded; FALSE if it failed and we
* selected in the system/device default font instead
*
*/

#ifdef DEBUG
/* %%Function:FSelectFont %%Owner:bryanl */
HANDNATIVE FSelectFont( pfti, phfont, phdc )
struct FTI *pfti;
HFONT *phfont;
HDC *phdc;
{
	HFONT hfontOrig = *phfont;
	int ww;
#ifdef DFONT
	static int fDFComm = fTrue;
#endif

	Assert( *phfont != NULL && *phfont != hfontSpecNil );
	Assert( pfti != NULL );

	if (pfti->fPrinter)
		{       /* Selecting into printer DC */
#ifdef DFONT
		if (*phfont == GetStockObject( DEVICEDEFAULT_FONT ))
			CommSz(SzShared( "Selecting printer font: DEVICEDEFAULT_FONT\r\n"));
		else
			CommSzNum(SzShared("Selecting printer font: "), *phfont );
#endif  /* DFONT */

		Assert( vpri.hdc != NULL );
		/* We should not have been called if there is no printer device */
		Assert( vpri.pfti != NULL );

		*phdc = vpri.hdc;

		if (OurSelectObject( *phdc, *phfont ) == NULL)
			{
			/* Selecting a font into the printer DC failed */
			/* This probably means we are out of global memory */
			/* Use DEVICEDEFAULT_FONT instead; it is guaranteed to succeed */
#ifdef DFONT    
			CommSzSz(SzFrame("Could not select printer font"),szEmpty);
#endif
			SetErrorMat(matFont);
			*phfont = GetStockObject( DEVICEDEFAULT_FONT );
			AssertDo( OurSelectObject( *phdc, *phfont ) );
			}
		}
	else  if (vfPrvwDisp)
		{
		SelectPrvwFont(phfont, phdc);
		}
	else
		{   /* Selecting into screen DCs */
		HFONT hfontSystem = GetStockObject( SYSTEM_FONT );

#ifdef DFONT

		if (fDFComm)
			if (*phfont == GetStockObject( SYSTEM_FONT ))
				CommSz(SzShared( "Selecting screen font: SYSTEM_FONT\r\n"));
			else
				CommSzNum(SzShared("Selecting screen font: "), *phfont);
#endif /* DFONT */

		Assert( vsci.pfti != NULL );    /* This should always be true */
		/* or we should not have been called */

		Assert( vsci.hdcScratch );
		*phdc = vsci.hdcScratch;

#ifdef DEBUG
		if (vpref.fDraftView)
			Assert( *phfont == hfontSystem );
#endif

#ifdef DISABLE /* GlobalWire taking ~50% of time! */
		if (*phfont != hfontSystem)
			{
			HFONT hfontSav;
			HANDLE hfontPhy;
			int f;

			if (!(hfontSav = OurSelectObject( vsci.hdcScratch, *phfont)))
				goto LScreenFail;
			hfontPhy = GetPhysicalFontHandle( vsci.hdcScratch );
			if (!OurSelectObject( vsci.hdcScratch, hfontSav))
				goto LScreenFail;
#ifdef DFONT
			fDFComm = fFalse;
#endif
			ResetFont(fFalse /*fPrinterFont*/);
#ifdef DFONT
			fDFComm = fTrue;
#endif
			if (((f=GlobalFlags(hfontPhy)) & GMEM_LOCKCOUNT) == 0 &&
					(f & ~GMEM_LOCKCOUNT) != 0)
				{
				Scribble(ispWireFont, 'W');

				GlobalWire(hfontPhy);
				GlobalUnlock(hfontPhy); /* because GlobalWire locked it */
				}
			}
#endif /* DISABLE */

		for ( ;; )
			{
			/* Select the font into the scratchpad memory DC and all window DC's*/

			if (vsci.hdcScratch)
				if (!OurSelectObject( vsci.hdcScratch, *phfont ))
					{
/* could not select in the font: revert back to the system font */
LScreenFail:
#ifdef DFONT		  
					CommSzSz(SzFrame("Failed to select screen font"),szEmpty);
#endif
					SetErrorMat(matFont);
					if (*phfont != hfontSystem)
						{
						*phfont = hfontSystem;
						continue;
						}
/* this should never happen, according to paulk, but just in case.. */
					Assert( fFalse );
					break;
					}

			for ( ww = wwDocMin; ww < wwMac; ww++ )
				{
				struct WWD **hwwd = mpwwhwwd [ww];
				HDC hdc;
/* test for hdc == NULL is for wwdtClipboard, whose DC is only 
	present inside clipboard update messages.  See clipdisp.c */
				if (hwwd != hNil && (hdc = (*hwwd)->hdc) != NULL)
					{
/* Hack to workaround problem with font mapper 3/18/89: for some unknown 
	reason, the mapper won't select the Preview and Terminal screen
	fonts for a window hdc, although it will for a memory DC compatible
	with it(!).  To workaround this, store away hdc to make sure
	the font cache gets the metrics that will accurately reflect the screen
	results. (BL) */
					*phdc = hdc;
					if (!OurSelectObject( hdc, *phfont ))
						goto LScreenFail;
					}
				}

			break;
			}   /* end for ( ;; ) */
		}

	Scribble(ispWireFont, ' ');
	pfti->hfont = *phfont;
	return *phfont == hfontOrig;
}


#endif	/* DEBUG */

/* R E S E T  F O N T */
#ifdef DEBUG
/* %%Function:C_ResetFont %%Owner:bryanl */
HANDNATIVE C_ResetFont(fPrinterFont)
BOOL fPrinterFont;
{
/* This routine sets to NULL the currently selected printer or screen font,
	depending on the value of fPrint.  It does not free any fonts, leaving the
	device's font chain intact.

	This should be done before freeing fonts to assure that none are selected
	into DC's.  It may also be used if explicit use of the system font is required.

*/

	extern BOOL vfPrinterValid;
	struct FTI *pfti;
	HDC hdcT;
	HFONT hfont;

	if ((pfti = (fPrinterFont) ? vpri.pfti : vsci.pfti) == NULL)
		return; /* device does not exist */

	if (fPrinterFont && vpri.pfti->fTossedPrinterDC)
		{   /* printer DC was tossed away */
		goto LNoFont;
		}
	hfont = GetStockObject( fPrinterFont ? DEVICEDEFAULT_FONT : SYSTEM_FONT );
	AssertDo(FSelectFont( pfti, &hfont, &hdcT ));
LNoFont:
	pfti->hfont = NULL;   /* So we know there's no cached font in pfti->hdc */
	pfti->fcid.lFcid = fcidNil;
}


#endif	/* DEBUG */


/* F C I D  T O  P L F */

#ifdef DEBUG
/* %%Function:C_FGraphicsFcidToPlf %%Owner:bryanl */
HANDNATIVE C_FGraphicsFcidToPlf( fcid, plf, fPrinterFont )
union FCID fcid;
LOGFONT *plf;
int fPrinterFont;
{   /* Translate an FCID into a windows logical font request structure */
	int ps;
	struct FFN *pffn;

	SetBytes(plf, 0, sizeof(LOGFONT));

/* Scale the request into device units */

	Assert( fcid.hps > 0 );
	plf->lfHeight = NMultDiv( fcid.hps * (czaPoint / 2),
			fPrinterFont ? vfli.dyuInch : vfli.dysInch,
			czaInch );

/* (BL) *** HACK ***   ***  ACK *** **** GAG ****  *** BARF ***
		The Windows courier font has interesting non-international pixels
		in its internal leading area. So, when picking courier
		screen fonts, use positive lfHeight, meaning choose the font by 
		CELL height instead of CHARACTER height. 
		ChrisLa says this fiasco is Aldus' fault. */
	if (fcid.ibstFont != ibstCourier || fPrinterFont)
		plf->lfHeight = -plf->lfHeight;

	Assert( fcid.ibstFont < (*vhsttbFont)->ibstMac );
	pffn = PstFromSttb( vhsttbFont, fcid.ibstFont );
/* bits 0-1: pitch request
	bits 4-6: font family */
	plf->lfPitchAndFamily = (pffn->ffid & maskFfFfid) | fcid.prq;
	Assert( (plf->lfPitchAndFamily & maskPrqLF) == FIXED_PITCH || 
			(plf->lfPitchAndFamily & maskPrqLF) == VARIABLE_PITCH || 
			(plf->lfPitchAndFamily & maskPrqLF) == DEFAULT_PITCH );

/* Set Bold, Italic, StrikeOut, Underline */

	plf->lfWeight = fcid.fBold ? FW_BOLD : FW_NORMAL;
	if (fcid.fItalic)
		plf->lfItalic = 1;
	if (fcid.fStrike)
		plf->lfStrikeOut = 1;
/* underlining for the screen is handled by drawing lines */
	if (fPrinterFont && fcid.kul != kulNone)
		plf->lfUnderline = 1;

	plf->lfCharSet = ChsPffn(pffn);
	bltbyte( pffn->szFfn, plf->lfFaceName, LF_FACESIZE );

	if (vfPrvwDisp && !fPrinterFont)
		GenPrvwPlf(plf);

#ifdef DFONT
		{
		int rgw [5];

		CommSzSz(SzShared("Built logfont for : "), plf->lfFaceName);
		rgw [0] = -plf->lfHeight;
		rgw [1] = plf->lfWidth;
		rgw [2] = plf->lfPitchAndFamily >> 4;
		rgw [3] = plf->lfPitchAndFamily & 3;
		rgw [4] = plf->lfCharSet;
		CommFontAttr( SzShared( "	 Requested Attributes: "),
				plf->lfItalic, plf->lfStrikeOut, plf->lfUnderline, plf->lfWeight != FW_NORMAL);
		}
#endif /* DFONT */
	return pffn->fGraphics;
}


#endif	/* DEBUG */

/* P F C E  L R U  G E T */

#ifdef DEBUG
/* %%Function:PfceLruGet %%Owner:bryanl */
HANDNATIVE struct FCE *PfceLruGet()
	/* tosses out the LRU cache entry's information */
{
	struct FCE *pfce, *pfceEndChain;
	struct FTI *pfti;

	Debug( pfceEndChain = NULL );

/* motivation for algorithm is to free the LRU font if there is only */
/* one device's fonts in the cache; and to alternate randomly between */
/* the LRU fonts for each device if there are 2 */

/* NOTE: This algorithm could repeatedly select the same slot if there */
/*       was a length-1 chain and a length-ifcMax-1 chain, but this case */
/*       is not interesting here */

/* Search for an appropriate FCE slot to free: */
/*     1. If we find a slot that is free, use that */
/*     2. If not, use the last end-of-chain we come to */

	for ( pfce = rgfce; pfce < &rgfce [ifceMax]; pfce++ )
		{
		if (pfce->hfont == NULL)
			return pfce;
		else  if (pfce->pfceNext == NULL)
			pfceEndChain = pfce;
		}
#ifdef DFONT
	CommSzNum(SzFrame("PfceLruGet: allocate cache slot = "), pfceEndChain - rgfce );
#endif

/* found a currently allocated cache entry at the end of its device's LRU
	chain.  Remove it from the chain and free its handles */

	Assert( pfceEndChain != NULL );
	pfti = pfceEndChain->fPrinter ? vpri.pfti : vsci.pfti;
	if (pfceEndChain->pfcePrev == NULL)
		{
		/* Since this font is at the head of the chain, it may be the one */
		/* currently selected into the device DC(s).  If so, deselect it */

		ResetFont( pfti->fPrinter );
		pfti->pfce = NULL;
		}
	else
		{
		/* also reset font selected into DC if the hfont is null -- we may
			have left this font selected in because of the fWidthsOnly
			optimization */
		if (pfti->hfont == NULL)
			ResetFont( pfti->fPrinter );

		pfceEndChain->pfcePrev->pfceNext = NULL;
		pfceEndChain->pfcePrev = NULL;
		}

	FreeHandlesOfPfce( pfceEndChain );
	return pfceEndChain;
}


#endif	/* DEBUG */

#ifdef DEBUG
/* O u r  G e t  C h a r  W i d t h */
/* %%Function:OurGetCharWidth %%Owner:bryanl */
HANDNATIVE OurGetCharWidth( hdc, chFirst, chLast, lpdxp )
HDC hdc;
int chFirst, chLast;
int far *lpdxp;
{
	Assert( hdc != NULL );
	Assert( chFirst >= 0 && chLast <= 255 );

	while (chFirst <= chLast)
		{
		*lpdxp++ = GetTextExtent( hdc, &chFirst, 1 );
		chFirst++;
		}
}


#endif	/* DEBUG */

#ifdef DFONT
CommFontAttr(szHeader, fItalic, fStrikeOut,fUnderline,fBold)
char szHeader[];
{
	CommNLFFontAttr(szHeader, fItalic, fStrikeOut,fUnderline,fBold);
	CommSz( SzShared( "\r\n") );
}


#endif	/* DFONT */
