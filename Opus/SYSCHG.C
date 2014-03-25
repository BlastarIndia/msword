/* S Y S C H G . C */
#define NOMINMAX
#define NOSCROLL
#define NOREGION
#define NOWH
#define NOMETAFILE
#define NOWNDCLASS
#define NOBRUSH
#define NONCMESSAGES
#define NOKEYSTATE
#define NOCLIPBOARD
#define NOHDC
#define NOCREATESTRUCT
#define NODRAWTEXT
#define NOCTLMGR
#define NOSYSMETRICS
#define NOMENUS
#define NOMB
#define NOFONT
#define NOOPENFILE
#define NOMEMMGR
#define NORESOURCE
#define NOICON
#define NOKANJI
#define NOSOUND
#define NOCOMM


#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "version.h"
#include "disp.h"
#include "screen.h"
#include "scc.h"
#include "doc.h"
#include "ch.h"
#include "props.h"
#include "opuscmd.h"
#include "format.h"
#include "splitter.h"
#include "print.h"
#include "doslib.h"
#include "sel.h"
#include "debug.h"
#include "inter.h"
#include "heap.h"
#include "file.h"
#include "layout.h"
#include "rareflag.h"
#include "resource.h"


extern CHAR	    	szDoc[];
extern CHAR	    	szDot[];
extern CHAR	    	szBak[];
extern CHAR	    	szApp[];
extern CHAR             szNone[];
extern CHAR		szEmpty[];
extern struct MERR      vmerr;
extern struct FLI       vfli;
extern struct PRI       vpri;
extern struct SCI       vsci;
extern struct WWD       **mpwwhwwd[];
extern struct DOD       **mpdochdod[];
extern int              wwMac;
extern HWND             vhwndRibbon;
extern int              vfInitializing;
extern int              vlm;
extern int              vflm;
extern struct ITR	vitr;
extern struct PREF      vpref;
extern struct STTB    **vhsttbCvt;
extern int              vfConversion;
extern int              vfCustomDTPic;
extern struct SEL	selCur;
extern int              vfDrawPics;
extern HWND             vhwndApp;          /* handle to parent's window */
extern HWND				vhwndCBT;
extern int					vulm;
extern uns		cKBitmap;
extern BYTE     fProtectModeWindows;

CHAR (**HszCreateSYS())[];

/* %%Function:AppWinSysChange %%Owner:davidbo */
AppWinSysChange(wm, lpsz)
int wm;
LPSTR lpsz;
{
	/* This routine processes the WM_SYSCOLORCHANGE, WM_WININICHANGE,
	WM_DEVMODECHANGE, and WM_FONTCHANGE  messages. */

	void SetWindowColors();

	int doc;
	struct DOD *pdod;
	CHAR szField[ichMaxIDSTR];
	RECT rc;
	CHAR szPrinter[ichMaxProfileSz];

	switch (wm)
		{
	case WM_SYSCOLORCHANGE:
		if (vfDrawPics)  /* let this happen for print/repag  */
			{
			vpri.wmm |= wmmSysColorChange;
			break;
			}
		/* Someone is changing the system colors. */
		SetWindowColors();
		/* set SDM colors */
		/* SDM must be initialized before calling ChangeColors */
		if (vmerr.fSdmInit)
			ChangeColors();
		break;

	case WM_WININICHANGE:
		/* We only care if the "windows", "opus", "intl"  or the "devices" 
		fields have changed. */
		if (lpsz == NULL || (CchCopyLpszCchMax(lpsz, (LPSTR)szField, ichMaxIDSTR),
				FEqNcSz(szField, SzSharedKey("devices",DevicesWININI))) || 
				FEqNcSz(szField, SzSharedKey("windows",WindowsWININI))  ||
				FEqNcSz(szField, szApp) ||
				FEqNcSz(szField, SzSharedKey("intl",intl)))
			{
			if (vlm == lmPrint || vlm == lmFRepag || vfDrawPics)
				{
				vpri.wmm |= wmmWinIniChange;
				break;
				}

			if (lpsz == NULL)
				/* assume everything else has changed too */
				SetWindowColors();

			ReadUserProfile(fFalse);

			/* force conversion information to be reread */
			vfConversion = -1;
			if (vhsttbCvt != hNil)
				{
				FreeHsttb(vhsttbCvt);
				vhsttbCvt = hNil;
				}

			/* force refetching custom date/time pic */
			vfCustomDTPic = -1;

			/* Force re-reading of underline mode */
			vulm = ulmNil;

			/* Reestablish the printer from the profile in case it was the
			printer that was changed. */
			FreePh( &vpri.hszPrinter );
			FreePh( &vpri.hszPrDriver );
			FreePh( &vpri.hszPrPort );
			FGetPrinterFromProfile();
			vmerr.fPrintEmerg = fFalse;

			goto LGetNewPrinter;
			}
		break;

	case WM_DEVMODECHANGE:
		vmerr.fPrintEmerg = fFalse;
		/* The device mode for some printer has changed; check to see if it was
		our printer. */
		if (vpri.hszPrinter != NULL && (lpsz == NULL ||
				(CchCopyLpszCchMax(lpsz, (LPSTR)szPrinter, ichMaxProfileSz),
				FEqNcSz(szPrinter, **vpri.hszPrinter))))
			{
			if (vlm == lmPrint || vlm == lmFRepag)
				{
				vpri.wmm |= wmmDevModeChange;
				break;
				}
			goto LGetNewPrinter;
			}
		break;

	case WM_FONTCHANGE:
		if (vlm == lmPrint || vlm == lmFRepag)
			{
			vpri.wmm |= wmmFontChange;
			break;
			}
		/* Delete all of the current fonts. */
		FreeFontsPfti( vsci.pfti );
		FreeFontsPfti( vpri.pfti );
LGetNewPrinter:
		PrinterChanged();
		vrf.fCkPgMarg = fTrue;  /* do actual check in idle */
		break;
		}   /* end switch */
}


/* %%Function:PrinterChanged %%Owner:davidbo */
PrinterChanged()
{
	int doc;
	struct DOD *pdod, **hdod;
	extern int docMac;
	extern struct PL **vhplbmc;

	Assert( vlm != lmPrint );

	vpri.fHaveEnv = FALSE;

	vulm = UlmFromDrvWinIni();

	for (doc = docMinNormal; doc < docMac; doc++)
		{
		hdod = mpdochdod[doc];
		if (hdod == hNil)
			continue;
		pdod = *hdod;
		if (pdod->fMother)
			pdod->fEnvDirty = TRUE;
		}

/* The printer has changed; toss the old DC and get a new one */
	FreePhsttb( &vpri.hsttbPaf );
	SetFlm( flmIdle );

/* dirty lists of fonts and points in ribbon */
	if (vhwndRibbon)
		DirtyRibbonFontLists();

/* get new environment */
	GetPrintEnv();

/* Everything must be redisplayed because the world may have changed. */
	TrashAllWws();
	vpri.fHaveBinInfo = fFalse;
	if (vlm == lmPreview)
		CmdRegenPrvw();

    /* Free the cached bitmap because it was stretched for the display to
    reflect its appearance on the printer. */
	if (vhplbmc != hNil)
		InvalBmc(docNil, cp0, cp0);

#ifdef NOTUSED   /* fcheck call delayed to idle */
/* Bug fix BL 4/4/89: force getting a printer DC so margins are correct
	for FCheckPageAndMargins call */
	SetFlm( flmRepaginate );
#endif /* NOTUSED */
}



/* %%Function:SetWindowsColors %%Owner:davidbo */
void SetWindowColors()
{
	/* Sets up vsci fields
			frees & sets these:
			hbrBkgrnd
			hbrText
			hbrBorder
			hbrDesktop
		hbrScrollBar
			hpen
			hpenBorder
			ropErase
			dcib's
	
	*/

	LOGPEN lp;
	HBITMAP hbm;
#ifdef WIN23
	DWORD	rgbTmp;
#endif /* WIN23 */

	/* Get the color of the background and the text. */
	vsci.rgbBkgrnd = GetSysColor(COLOR_WINDOW);
	vsci.rgbText = GetSysColor(COLOR_WINDOWTEXT);
	vsci.rgbBorder = GetSysColor (COLOR_WINDOWFRAME);
#ifdef WIN23
	if (vsci.fWin3)
		{
		vsci.rgbButtonShadow = GetSysColor (COLOR_BTNSHADOW);
		vsci.rgbButtonFace = GetSysColor (COLOR_BTNFACE);
		vsci.rgbButtonShadow = GetSysColor (COLOR_BTNSHADOW);
		vsci.rgbButtonText = GetSysColor(COLOR_BTNTEXT);
		}
#endif /* WIN23 */

	Assert((vsci.rgbBkgrnd & 0xFF000000) == 0 && 
			(vsci.rgbText & 0xFF000000) == 0);

	/* FREE OLD OBJECTS AND SETUP NEW ONES */

	if (!vfInitializing)
		{
		SetAllDcAttrib( dccmUnlockBrushPen );
		UnlogGdiHandle(vsci.hbrText, 1015);
		DeleteObject (vsci.hbrText);
		UnlogGdiHandle(vsci.hbrDesktop, 1016);
		DeleteObject (vsci.hbrDesktop);
		UnlogGdiHandle(vsci.hbrBorder, 1017);
		DeleteObject (vsci.hbrBorder);
		UnlogGdiHandle(vsci.hbrScrollBar, 1018);
		DeleteObject (vsci.hbrScrollBar);
		UnlogGdiHandle(vsci.hpen, 1019);
		DeleteObject (vsci.hpen);
		UnlogGdiHandle(vsci.hpenBorder, 1020);
		DeleteObject (vsci.hpenBorder);
		UnlogGdiHandle(vsci.hbrBkgrnd, 1014);
		DeleteObject (vsci.hbrBkgrnd);
#ifdef WIN23
		if (vsci.fWin3Visuals )
			DeleteObject (vsci.hbrButtonText);
#endif /* WIN23 */
		}

	/* Set up the brush for the background. */
	if ((vsci.hbrBkgrnd = CreateSolidBrush(vsci.rgbBkgrnd)) == NULL)
		{
		/* Can't make the background brush; use the white brush. */
		vsci.hbrBkgrnd = GetStockObject(WHITE_BRUSH);
		vsci.rgbBkgrnd = RGB(0xff, 0xff, 0xff);
		}
	LogGdiHandle(vsci.hbrBkgrnd, 1014);

	/* Set up the text-color brush (for visi spaces) */
	if ((vsci.hbrText = CreateSolidBrush(vsci.rgbText)) == NULL)
		{
		/* Can't make the background brush; use the black brush. */
		vsci.hbrText = GetStockObject(BLACK_BRUSH);
		vsci.rgbText = RGB(0, 0, 0);
		}
	LogGdiHandle(vsci.hbrText, 1015);

	/* Set up the brush for the desktop. */
	if ((vsci.hbrDesktop = CreateSolidBrush(GetSysColor(COLOR_APPWORKSPACE))) == NULL)
		/* Can't make the desktop brush; use gray brush. */
		vsci.hbrDesktop = GetStockObject(GRAY_BRUSH);
	LogGdiHandle(vsci.hbrDesktop, 1016);

	/* Set up the brush for the scroll bar. */
	if ((vsci.hbrScrollBar = CreateSolidBrush(GetSysColor( COLOR_SCROLLBAR ))) == NULL)
		/* Can't make the scroll bar brush; use gray brush. */
		vsci.hbrScrollBar = GetStockObject(GRAY_BRUSH);
	LogGdiHandle(vsci.hbrScrollBar,1018);

	/* Set up the border-color brush */
	if ((vsci.hbrBorder = CreateSolidBrush(vsci.rgbBorder)) == NULL)
		/* cannot make border-color brush--use black */
		vsci.hbrBorder = GetStockObject(BLACK_BRUSH);
	LogGdiHandle(vsci.hbrBorder,1017);

#ifdef WIN23
	if (vsci.fWin3Visuals )
		{
		if ((vsci.hbrButtonText = CreateSolidBrush(vsci.rgbButtonText)) == NULL)
			vsci.hbrButtonText = GetStockObject(GRAY_BRUSH);
		/*REVIEW LOG ? */
		}
#endif /* WIN23 */
	/* Used for both normal pen and border pen */
	lp.lopnStyle = 0; /* solid */
	lp.lopnWidth.x = vsci.dxpBorder;
	lp.lopnWidth.y = vsci.dypBorder;

	/* Set up the normal black pen used in document and page info windows. */
	if (vsci.rgbText == RGB(0, 0, 0))
		{
		vsci.hpen = GetStockObject( BLACK_PEN );
		LogGdiHandle(vsci.hpen, 1019);
		}
	else  if (vsci.rgbText == RGB(0xFF, 0xFF, 0xFF))
		{
		vsci.hpen = GetStockObject( WHITE_PEN );
		LogGdiHandle(vsci.hpen, 1019);
		}
	else
		{
		lp.lopnColor = vsci.rgbText;
		if ((vsci.hpen = CreatePenIndirect ((LPLOGPEN) &lp)) == hNil)
			vsci.hpen = GetStockObject( BLACK_PEN );
		LogGdiHandle(vsci.hpen, 1019);
		}

	/* Set up the border pen */
	if (vsci.rgbBorder == RGB(0, 0, 0))
		{
		vsci.hpenBorder = GetStockObject( BLACK_PEN );
		LogGdiHandle(vsci.hpenBorder, 1020);
		}
	else  if (vsci.rgbBorder == RGB(0xFF, 0xFF, 0xFF))
		{
		vsci.hpenBorder = GetStockObject( WHITE_PEN );
		LogGdiHandle(vsci.hpenBorder, 1020);
		}
	else
		{
		lp.lopnColor = vsci.rgbBorder;
		if ((vsci.hpenBorder = CreatePenIndirect ((LPLOGPEN) &lp)) == hNil)
			vsci.hpenBorder = GetStockObject( BLACK_PEN );
		LogGdiHandle(vsci.hpenBorder, 1020);
		}


	Assert (vsci.hbrBkgrnd != NULL &&
			vsci.hbrText != NULL &&
			vsci.hbrDesktop != NULL &&
			vsci.hbrBorder != NULL &&

			vsci.hpen != NULL &&
			vsci.hpenBorder != NULL );

	/* Compute the raster op to erase the screen. */
	if (vsci.rgbBkgrnd == RGB(0xff, 0xff, 0xff))
		{
		/* WHITENESS is faster than copying a white brush. */
		vsci.ropErase = WHITENESS;
		}
	else  if (vsci.rgbBkgrnd == RGB(0, 0, 0))
		{
		/* BLACKNESS is faster than copying a black brush. */
		vsci.ropErase = BLACKNESS;
		}
	else
		{
		/* For everything else, we have to copy the brush. */
		vsci.ropErase = PATCOPY;
		}


	/* this flag indicates a special case for window colors exists:
		the screen is monochrome AND the text color is white.  When
		Blting to the screen we must use an inverted operation. */

	vsci.fInvertMonochrome = (vsci.fMonochrome && vsci.rgbText != 0L);

	/* Set up Memory DCIB */
	vsci.dcibMemDC.ropErase = WHITENESS;
	vsci.dcibMemDC.ropBitBlt = (vsci.fInvertMonochrome) ? ROP_Sn : SRCCOPY;

	/* Set up Screen DCIB */
	vsci.dcibScreen.ropErase = vsci.ropErase;
	vsci.dcibScreen.ropBitBlt = (vsci.fInvertMonochrome) ? ROP_Sn : SRCCOPY;

	SetAllDcAttrib( dccmColors );   /* select new colors into window dc's */
}


/* S e t  A l l  D c  A t t r i b */
/* %%Function:SetAllDcAttrib %%Owner:davidbo */
SetAllDcAttrib( dcc )
int dcc;
{
	int ww;

	for ( ww = wwDocMin; ww < wwMac; ww++ )
		{
		struct WWD **hwwd = mpwwhwwd [ww];
		HDC hdc;

/* test for hdc == NULL is for wwdtClipboard, whose DC is only
	present inside clipboard update messages. see clipdisp.c. */
		if (hwwd != hNil && (hdc = (*hwwd)->hdc) != NULL)
			if (!FSetDcAttribs( hdc, dcc ))
				SetErrorMat(matMem);
		}

	if (vsci.hdcScratch != NULL)
		if (!FSetDcAttribs(vsci.hdcScratch, (dcc &  (dccScratch + dccmUnlockBrushPen))))
			SetErrorMat(matMem);

	if (vsci.mdcdBmp.hdc != NULL)
		if (!FSetDcAttribs(vsci.mdcdBmp.hdc, (dcc &  (dccScratch + dccmUnlockBrushPen))))
			SetErrorMat(matMem);
}



/* F  G E T  P R I N T E R  F R O M  P R O F I L E */
/* Read the device name, driver name, and port name of the windows default
	printer into the hsz fields of vpri.  Return fFalse if there was no
	valid default printer, or if we ran out of memory allocating the strings

	Accepted format:

	[windows]
	device=Pr Name,Driver,Port

*/
/* %%Function:FGetPrinterFromProfile %%Owner:davidbo */
BOOL FGetPrinterFromProfile()
{
	CHAR szPrinter[ichMaxProfileSz];
	CHAR chNull = '\0';
	CHAR *pch, *pchT;
	CHAR *pchDriver;
	CHAR *pchPort;

	/* Read [windows] device = into szPrinter */

	GetProfileString((LPSTR)SzFrameKey("windows",WindowsWININI),
			(LPSTR)SzFrameKey("Device",DeviceWININI),
			(LPSTR)&chNull,
			(LPSTR)szPrinter, ichMaxProfileSz);

	if ((pch = index(szPrinter, ',')) == 0)
		goto LFail;

	/* Remove any trailing spaces from the printer name. */
	for ( pchT = pch;  *pchT == ' ' && pchT > &szPrinter[0];  pchT -- )
		;
	*pchT = '\0';

	/* Parse out the port and the driver names. */
	ParseDeviceSz(pch + 1, &pchPort, &pchDriver);
	if (*pchPort == '\0' || *pchDriver == '\0')
		{
		ReportSz("[windows] device= in WIN.INI has no port name");
		goto LFail;
		}

	if (FNoHeap( vpri.hszPrinter = HszCreateSYS(szPrinter) ) ||
			FNoHeap( vpri.hszPrDriver = HszCreateSYS(pchDriver) ) ||
			FNoHeap( vpri.hszPrPort = HszCreateSYS(pchPort) ))
		{
		goto LFail;
		}

	if (vulm == ulmNil)
		vulm = UlmFromDrvWinIni();

	return fTrue;

LFail:
	FreePh( &vpri.hszPrDriver );
	FreePh( &vpri.hszPrinter );
	FreePh( &vpri.hszPrPort );
	return fFalse;
}


/* %%Function:ParseDeviceSz %%Owner:davidbo */
int ParseDeviceSz(sz, ppchPort, ppchDriver)
CHAR sz[];
CHAR **ppchPort;
CHAR **ppchDriver;
{
	/* This routine takes a string that came from the "device" entry in the user
	profile and returns in *ppchPort and *ppchDriver pointers to the port and
	driver sutible for a CreateDC() call.  If no port is found in the string,
	*ppchPort will point to a string containing the name of the null device.
	This routine returns the number of ports for this printer (separated by null
	characters in the string pointed at by *ppchPort).  NOTE: sz may be modified
	by this routine, and the string at *ppchPort may not be a substring of sz
	and should not be modified by the caller. */

	register CHAR *pch;
	int cPort = 0;

	/* Remove any leading spaces from the string. */
	for (pch = &sz[0]; *pch == ' '; pch++);

	/* The string starts with the driver name. */
	*ppchDriver = pch;

	/* The next space or comma terminates the driver name. */
	for ( ; *pch != ' ' && *pch != ',' && *pch != '\0'; pch++);

	/* If the string does not have a port associated with it, then the port
	must be the null device. */
	if (*pch == '\0')
		{
		/* Set the port name to "None". */
		*ppchPort = &szNone[0];
		cPort = 1;
		}
	else
		{
		/* As far as we can tell, the port name is valid; parse it from the
		driver name. */
		if (*pch == ',')
			{
			*pch++ = '\0';
			}
		else
			{
			/* Find that comma separating the driver and the port. */
			*pch++ = '\0';
			for ( ; *pch != ',' && *pch != '\0'; pch++);
			if (*pch == ',')
				{
				pch++;
				}
			}

		/* Remove any leading spaces from the port name. */
		for ( ; *pch == ' '; pch++);

		/* Check to see if there is really a port name. */
		if (*pch == '\0')
			{
			/* Set the port name to "None". */
			*ppchPort = &szNone[0];
			cPort = 1;
			}
		else
			{
			/* Set the pointer to the port name. */
			*ppchPort = pch;

			while (*pch != '\0')
				{
				register CHAR *pchT = pch;

				/* Increment the number of ports found for this printer. */
				cPort++;

				/* Remove any trailing spaces from the port name. */
				for ( ; *pchT != ' ' && *pchT != ','; pchT++)
					{
					if (*pchT == '\0')
						{
						goto EndFound;
						}
					}
				*pchT++ = '\0';
				pch = pchT;

				/* Remove any leading spaces in the next port name. */
				for ( ; *pchT == ' '; pchT++);

				/* Throw out the leading spaces. */
				bltbyte(pchT, pch, CchSz(pchT));
				}
EndFound:
			;
			}
		}

	/* Parse the ".drv" out of the driver. */
	if ((pch = index(*ppchDriver, '.')) != 0 && 
			!FNeNcRgch(pch, SzFrame(".drv"), 4))
		{
		*pch = '\0';
		}

	return (cPort);
}




/* F i l l  H s t t b  P a f  F r o m  H d c */
/* This function is called when the printer changes or when the printer
	state changes such that the available font list might have changed.
	It enumerates the fonts available on the passed hdc device,
	adding them to *phsttbPaf, and to vhsttbFont if necessary */
/* %%Function:FillHsttbPaf %%Owner:davidbo */
FillHsttbPaf(phsttbPaf)
struct STTB ***phsttbPaf;
{
	extern FARPROC lpFontNameEnum;
	int tc;

	FreePhsttb( phsttbPaf );
	Assert( vpri.hdc != NULL );

	if ((*phsttbPaf = HsttbInit(20, fFalse/*fExt*/)) == hNil)
		return;

	tc = GetDeviceCaps(vpri.hdc, TEXTCAPS);

/* fEnumerationComplete = */
	EnumFonts( vpri.hdc, NULL /*lpFaceName */, lpFontNameEnum, 
			MAKELONG( phsttbPaf, 0 ) );

}


/* array of HALF-point sizes presented for enumeration for vector fonts
	or font-scaling printers (such as Postscript) */

csconst BYTE rghpsCanned [] = { 
	12, 16, 20, 24, 28, 32, 36, 40, 48, 60, 
			64, 72, 80, 96	};


#define ihpsCannedMax 	(sizeof (rghpsCanned))

/* F o n t  N a m e  E n u m */
/* This is the Windows callback function for font name (as opposed
	to size) enumeration.  This function is called once for each
	font available on the device. */
/* each font is added to vhsttbFont (if not already there); its ibst
	and available sizes is added to the passed sttbPaf. */
/* Return value is application-controlled; it is passed back to
	the caller of EnumFonts */

/* %%Function:FontNameEnum %%Owner:davidbo */
EXPORT BOOL FAR PASCAL FontNameEnum( lplf, lptm, fty, dw )
LPLOGFONT  lplf;
LPTEXTMETRIC lptm;
int fty;            /* font type:
	fty & RASTER_FONTTYPE == fRasterFont
	fty & DEVICE_FONTTYPE == fDeviceFont */
	struct {            /* This is passed through from the EnumFonts call */
	struct PL   ***phsttbPaf;
	struct PAF  *ppaf;
} dw;
{
	extern FARPROC lpFontSizeEnum;
	extern struct STTB **vhsttbFont;
	struct STTB **hsttbPaf = *dw.phsttbPaf;
	int ibstFont, ibstPaf;
	int tc;
	CHAR rgbPaf [cbPafLast];
	struct PAF *ppaf = rgbPaf;
	CHAR rgbFfn [cbFfnLast];
	struct FFN *pffn = rgbFfn;

/* fill out an FFN for this font */
/* SURPRISE! The Postscript symbol font lies, claiming
	that its char set is ANSI */

	bltbx( lplf->lfFaceName, (LPSTR)pffn->szFfn, LF_FACESIZE );
	pffn->ffid = lplf->lfPitchAndFamily & (maskFfFfid | maskPrqLF);
	pffn->fGraphics = !(fty & (DEVICE_FONTTYPE /* | RASTER_FONTTYPE */));
	pffn->fRaster = (fty & RASTER_FONTTYPE) != 0;
	pffn->cbFfnM1 = CbFfnFromCchSzFfn( CchSz( pffn->szFfn ) ) - 1;
	ChsPffn(pffn) = lplf->lfCharSet;

#ifdef BRYANL
		{
		int rgw [3];

		CommSzSz( SzFrame("FontNameEnum, name = "), pffn->szFfn );
		rgw [0] = (pffn->ffid & maskFfFfid) >> 4;
		rgw [1] = pffn->ffid & maskPrqLF;
		rgw [2] = ChsPffn(pffn);

		CommSzRgNum( SzFrame("  family, pitch, chs = "), rgw, 3 );
		}
#endif

/* add ffn to the master font table (vhsttbFont) if not already there */

	if ((ibstFont = IbstFindSzFfn(vhsttbFont, pffn )) == iNil)
		ibstFont = IbstAddStToSttb(vhsttbFont,pffn);
	else
		/* in case charset, pitch, or family changed */
		FChangeStInSttb(vhsttbFont, ibstFont, pffn);

	if (vmerr.fMemFail)
		return fFalse;
	Assert( ibstFont != ibstNil );

/* if we have seen this font name before (really happens!)
	add sizes to the existing PAF; else create a new one */

	if ((ibstPaf = IbstFindInSttbPaf( hsttbPaf, ibstFont)) != iNil)
		{
		struct PAF *ppafT;

		ppafT = PstFromSttb(hsttbPaf, ibstPaf);
		Assert( ppafT->cbPafM1 + 1 <= cbPafLast );
		bltbyte( ppafT, ppaf, ppafT->cbPafM1 + 1);
		}
	else 	
		{
		ppaf->cbPafM1 = offset(PAF,rghps) - 1;
		ppaf->ibst = ibstFont;
		}

/* Fill sizes in the PAF.  This is done from the canned list if we have 
	a scaling printer (e.g. PostScript) or a vector font, else from
	an enumeration of sizes */

	if ((tc = GetDeviceCaps( vpri.hdc, TEXTCAPS)) & (TC_SA_DOUBLE | TC_SA_INTEGER | TC_SA_CONTIN))
		{
		uns hpsBase;

#define hpsMacForIntDevice	128

		if (tc & (TC_SA_INTEGER | TC_SA_CONTIN))
			{
#ifdef BRYANL
			CommSzSz( SzFrame("** CONTIN/INT SCALING, ALLOW ALL SIZES **"),szEmpty);
#endif
			goto LNoLimits;
			}
	/* else fonts are limited to a power of two multiple of the base size */

		hpsBase = UMultDiv( lplf->lfHeight, 144 /*hpsInch*/, vfli.dyuInch );
#ifdef BRYANL
		CommSzNum( SzFrame("** DOUBLE SCALING, ALLOW ^2 MULTIPLES OF BASE hps = "), hpsBase );
#endif
		for ( ; hpsBase < hpsMacForIntDevice ; hpsBase <<= 1 )
			AddHpsToPaf( ppaf, hpsBase );
		}
	/* FUTURE - Danger Will Robinson (drm): This is a
		minor hack to deal with some printers that report
		"vector device" fonts */
	else  if (fty & DEVICE_FONTTYPE)
		{
		if (fty & RASTER_FONTTYPE)
			goto LNotVectorFont;
		else	
		/* vector device font - truetype font downloaded to printer is considered
		a device font, fake point sizes like postscript printers 
		*/
			goto LNoLimits;
		}
/* REVIEW BRYANL (bl): LaserJet claims not to be able to be "vector font
able or "raster font able" but Dan's App can print vector fonts
on the LaserJet. Is there something else we can test to make this
determination, or is this a bug in the LaserJet driver? In the short term,
the second half of the clause below is commented out so vector
fonts will always be enumerated. */

else  if (!(fty & RASTER_FONTTYPE) /* && (tc & TC_VA_ABLE) */ )  /* i.e. it is a vector font */
{
#ifdef BRYANL
	CommSzSz( SzFrame("** VECTOR FONT, ALL SIZES AVAILABLE **"),szEmpty);
#endif
LNoLimits:
	Assert( offset( PAF,rghps) + ihpsCannedMax <= cbPafLast );
	bltbx( (LPCH)rghpsCanned, (LPCH)ppaf->rghps, 
			ppaf->cbPafM1 = offset( PAF,rghps) + ihpsCannedMax - 1 );
}
else  if (tc & TC_RA_ABLE)
{
LNotVectorFont:
	dw.ppaf = ppaf;
	EnumFonts( vpri.hdc, (LPSTR) pffn->szFfn, lpFontSizeEnum, dw );
}
else
{
#ifdef BRYANL
	CommSzSz( SzFrame("** RASTER FONT w/ NON-RASTER PRINTER, NO SIZES **"),szEmpty );
#endif
	return fTrue;
}


/* add the PAF to the hsttb */
/* if it fails, no matter, just keep going */

if (ibstPaf == ibstNil)
IbstAddStToSttb( hsttbPaf, ppaf );          /* added new paf */
else
	FChangeStInSttb(hsttbPaf, ibstPaf, ppaf);/* added sizes to existing paf */

return fTrue;
}



/* F o n t  S i z e  E n u m */
/* This is the Windows callback function for font size
	enumeration.  This function is called once for each
	font size available on the device. */

/* %%Function:FontSizeEnum %%Owner:davidbo */
EXPORT BOOL FAR PASCAL FontSizeEnum( lplf, lptm, fty, dw )
LPLOGFONT  lplf;
LPTEXTMETRIC lptm;
int fty;            /* font type:
	fty & RASTER_FONTTYPE == fRasterFont
	fty & DEVICE_FONTTYPE == fDeviceFont */
	struct {            /* This is passed through from the EnumFonts call */
	struct PL   ***phsttbPaf;
	struct PAF  *ppaf;
} dw;
{
/* Add this size to the PAF */

#ifdef BRYANL
	CommSzNum( SzFrame("  FontSizeEnum, hps = "),UMultDiv(lptm->tmHeight - lptm->tmInternalLeading,
			144 /* hpsInch */,  vfli.dyuInch ));
#endif
	AddHpsToPaf( dw.ppaf, UMultDiv(lptm->tmHeight - lptm->tmInternalLeading,
			144 /* hpsInch */,  vfli.dyuInch )) ;
}


/* %%Function:AddHpsToPaf %%Owner:davidbo */
AddHpsToPaf( ppaf, hps )
struct PAF *ppaf;
int hps;
{
#define hpsMin		8
#define hpsMac		255

/* (BL) Bug fix: avoid proposing sizes that we'll just reject in the
	fonts dialog or combos */

	if ((uns)(hps - hpsMin) >= (hpsMac - hpsMin))
		return;

/* Add this size to the PAF */

	if (ppaf->cbPafM1 < cbPafLast - 1)
		{
		int ihps = ppaf->cbPafM1++ + (1 - offset(PAF,rghps));
		int ihpsMac = ihps + 1;

		Assert( ihps >= 0 );

/* size has been given in duplicate or out of order */

		if (ihps > 0 && hps <= ppaf->rghps [ihps-1])
			{
			while (ihps > 0 && hps <= ppaf->rghps [ihps - 1])
				ihps--;

			if (hps < ppaf->rghps [ihps])
				{                           /* out-of-order entry inserted */
				CHAR *phps = &ppaf->rghps [ihps];
				bltbyte( phps, phps+1, ihpsMac - 1 - ihps );
				}
			else
				{
				Assert(hps == ppaf->rghps[ihps]);
				--ppaf->cbPafM1;         /* duplicate entry ignored */
				}
			}

		ppaf->rghps [ihps] = hps;
		}
}



/* %%Function:IbstFindInSttbPaf %%Owner:davidbo */
IbstFindInSttbPaf( hsttbPaf, ibstFont )
struct STTB **hsttbPaf;
int ibstFont;
{
	int ibst;

	for ( ibst = 0; ibst < (*hsttbPaf)->ibstMac; ibst++)
		if (ibstFont == ((struct PAF *)PstFromSttb(hsttbPaf, ibst))->ibst)
			return ibst;

	return ibstNil;
}


/* P R O F I L E  S T R I N G S */
csconst CHAR rgstSection [][] =
{
	StSharedKey("intl",intl),
			stAppDef,
};


#define iszIntl     0
#define iszApp	    1

#define cchMaxProfileStr  50 /* for dumb bltbx */
#define chRespNo        'N'
#define chRespYes       'Y'

csconst struct PST
{  /* profile string table */
	BYTE istSection;
	CHAR stProfile [];
	CHAR stDefault [];
} 


rgpst [] =

{
	{ iszIntl, St("sDecimal"),		StKey(".",Decimal)	 },
	{ iszIntl, St("sCurrency"),		StKey("$",Currency)	 },
	{ iszIntl, St("sThousand"),		StKey(",",Thousand)	 },
	{ iszIntl, St("sList"), 		StKey(",",List)		 },
	{ iszIntl, St("iDate"), 		StKey("0",Date)		 },
	{ iszIntl, St("sDate"), 		StKey("/",DateSep)	 },
	{ iszIntl, St("sTime"), 		StKey(":",TimeSep)	 },
	{ iszIntl, St("s1159"), 		StKey("am",am)		 },
	{ iszIntl, St("s2359"), 		StKey("pm",pm)		 },
	{ iszIntl, St("iLZero"),		StKey("0",LZeroDefault) },
	{ iszIntl, St("iCurrency"),		StKey("0",PostfixDefault) },
	{ iszIntl, St("iDigits"),		StKey("2",IDigitsDefault) },
	{ iszApp,  St("DOC-Extension"), 	StShared(".DOC")	 },
	{ iszApp,  St("DOT-Extension"), 	StShared(".DOT")	 },
	{ iszApp,  St("BAK-Extension"), 	StShared(".BAK")	 },
#ifdef RPTLOWMEM /* no longer wanted */
	{ iszApp,  St("InitialMemoryWarning"),	""			 },
#else
	{ iszApp,  St(""),	""			 },
#endif /* RPTLOWMEM */
	{ iszApp,  St("LoadCore"),		""			 },
	{ iszApp,  St("Conversion"),		""			 },
	{ iszApp,  St("DrawAppName"),		StKey("EDITPIC.EXE",DrawNameDefault) },
	{ iszApp,  St("DrawAppClass"),		StKey("EDITPIC",DrawClassDefault) },
	{ iszIntl, St("iCountry"),		StKey("001",ICountryDefault) },
	{ iszIntl, St("sP"),			StKey("P",P)		 },
	{ iszIntl, St("sS"),			StKey("S",S)		 },
	{ iszIntl, St("iTime"), 		StKey("0",Time24)	 },
	{ iszApp,  St("NovellNet"),     ""           },
	{ iszApp,  St("As400"),		""		},
};


#define ipstDecimal     0
#define ipstCurSign     1
#define ipstThousand    2
#define ipstList        3
#define ipstIDate       4
#define ipstDateSep     5
#define ipstTimeSep     6
#define ipstMorn        7
#define ipstEve         8
#define ipstLZero       9
#define ipstCurrency   10
#define ipstIDigits    11
#define ipstDocExt     12
#define ipstDotExt     13
#define ipstBakExt     14

#ifdef INFOONLY
/* These appear in wordwin.h: */
#define ipstMemWarn     15
#define ipstCoreLoad	16
#define ipstConversion  17
#define ipstDrawName    18
#define ipstDrawClass   19
#define ipstNovellNet   24
#define ipstAs400			25
#endif /* INFOONLY */

#define ipstCountry     20
#define ipstPage        21
#define ipstSect        22
#define ipstITime       23

/* %%Function:GetSzProfileIpst %%Owner:davidbo */
int GetSzProfileIpst (ipst, sz, szDefault, cchMax, fForceDefault)
int ipst;
CHAR *sz, *szDefault; /* both return value, large enough for cchMax chars */
int cchMax;
BOOL fForceDefault;
{
	int	 cch;
	CHAR *pch;
	CHAR szSection [cchMaxProfileStr];
	CHAR szProfile [cchMaxProfileStr];
	CHAR far *lpst;

	lpst = rgstSection[rgpst[ipst].istSection];
	*(CHAR far *)bltbx(lpst + 1, (LPSTR)szSection, *lpst) = 0;
	lpst = rgpst[ipst].stProfile;
	*(CHAR far *)bltbx(lpst + 1, (LPSTR)szProfile, *lpst) = 0;
	lpst = rgpst[ipst].stDefault;
	*(CHAR far *)bltbx(lpst + 1, (LPSTR)szDefault, *lpst) = 0;

	Assert (CchSz (szSection) <= cchMaxProfileStr && 
			CchSz (szProfile) <= cchMaxProfileStr && 
			CchSz (szDefault) <= cchMax);

	if (fForceDefault || vhwndCBT)
		{
		cch = CchCopySz(szDefault, sz);
		}
	else
		{
		cch = GetProfileString ((LPSTR)szSection, (LPSTR)szProfile,
				(LPSTR)szDefault, (LPSTR)sz, cchMax);
		}

	return cch;
}


/* %%Function:FFromProfile %%Owner:davidbo */
FFromProfile(fDefault, ipst)
BOOL fDefault;
int ipst;
{
	CHAR szResp[2];
	CHAR szRespDef[2];
	GetSzProfileIpst(ipst, szResp, szRespDef, 2, fFalse);
	if (ChUpper(szResp[0]) == chRespYes || szResp[0] == '1')
		return fTrue;
	else  if (ChUpper(szResp[0]) == chRespNo || szResp[0] == '0')
		return fFalse;
	else
		return fDefault;
}


#define maskICurPostfix	  1
#define maskICurSepBlank  2

/* %%Function:ReadUserProfile %%Owner:davidbo */
ReadUserProfile(fForceDefault)
BOOL fForceDefault;
{
	int	 iCur;
	int	 iCountry;
	CHAR *pch, ch;
	CHAR szBuf [cchMaxSz];
	CHAR szDefault [cchMaxProfileStr];

	/* Get the decimal point, 1000th, and other changable characters from
		the user profile. */

	GetSzProfileIpst (ipstDecimal, szBuf, szDefault, 2, fForceDefault);
	vitr.chDecimal = FAlpha(*szBuf) ? szDefault[0] : szBuf[0];

	GetSzProfileIpst (ipstCurrency, szBuf, szDefault, 2, fForceDefault);
	iCur = *szBuf - '0';
	vitr.fCurPostfix = (iCur & maskICurPostfix) != 0;
	vitr.fCurSepBlank = (iCur & maskICurSepBlank) != 0;

	GetSzProfileIpst (ipstLZero, szBuf, szDefault, 2, fForceDefault);
	vitr.fLZero = (*szBuf == '1');

	/* + 1 for possible trailing or leading blanks. */
	GetSzProfileIpst (ipstCurSign, szBuf, szDefault, cchMaxCurrency + 1,
			fForceDefault);
	CchSYSStripString(szBuf, CchSz(szBuf) - 1);
	bltbyte(szBuf, vitr.szCurrency, cchMaxCurrency);

	GetSzProfileIpst (ipstThousand, szBuf, szDefault, 2, fForceDefault);
	vitr.chThousand = FAlpha(*szBuf) ? szDefault[0] : szBuf[0];

	GetSzProfileIpst (ipstList, szBuf, szDefault, 2, fForceDefault);
	vitr.chList = FAlpha(*szBuf) ? szDefault[0] : szBuf[0];

	vitr.fInvalidChSetting = FInvalidCharSetting();

	GetSzProfileIpst (ipstIDigits, szBuf, szDefault, 2, fForceDefault);
	if ((uns) (vitr.iDigits = *szBuf - '0') > 9)
		vitr.iDigits = szDefault[0] - '0';
	Assert((uns)(szDefault[0] - '0') <= 9);

	GetSzProfileIpst (ipstCountry, szBuf, szDefault, 4, fForceDefault);
	for (pch = szBuf, iCountry = 0; (ch = *pch) >= '0' && ch <= '9'; pch++)
		iCountry = (iCountry * 10) + (ch - '0');

	vitr.fMetric  = ((iCountry !=  US) && (iCountry != UK));
	vitr.fFrench = (iCountry == FRANCE) ||
		       (iCountry == NETHERLANDS);
	vitr.fScandanavian = (iCountry == DENMARK) ||
			(iCountry == FINLAND) ||
			(iCountry == NORWAY)  ||
			(iCountry == ICELAND) ||
			(iCountry == SWEDEN);

	switch (iCountry)
		{
	default:
		vitr.fUseA4Paper = fTrue;
		break;
/* following list courtesy of international via AdrianW */
	case US:
	case CANADA:
	case LATINAMERICA:
	case PERU:
	case MEXICO:
	case ARGENTINA:
	case BRAZIL:
	case CHILE:
	case VENEZUELA:
		vitr.fUseA4Paper = fFalse;
		break;
	case ITALY:
		vitr.fUseA4Paper = fTrue;
		vitr.iDigits = 2;
		break;
		}

	GetSzProfileIpst (ipstIDate, szBuf, szDefault, 2, fForceDefault);
	if ((uns)(vitr.iDate = *szBuf - '0') > 9)
		vitr.iDate = szDefault[0] - '0';
	Assert((uns)(szDefault[0] - '0') <= 9);

	GetSzProfileIpst (ipstITime, szBuf, szDefault, 2, fForceDefault);
	if ((uns)(vitr.iTime = *szBuf - '0') > 1)
		vitr.iTime = 0;
	Assert(vitr.iTime == 0 || vitr.iTime == 1);

	vitr.iLDate = iLDateDef;

	GetSzProfileIpst (ipstDateSep, szBuf, szDefault, 2, fForceDefault);
	vitr.chDate = *szBuf;

	GetSzProfileIpst (ipstTimeSep, szBuf, szDefault, 2, fForceDefault);
	vitr.chTime = *szBuf;

	GetSzProfileIpst (ipstPage, szBuf, szDefault, 2, fForceDefault);
	vitr.chPageNo = *szBuf;

	GetSzProfileIpst (ipstSect, szBuf, szDefault, 2, fForceDefault);
	vitr.chSectNo = *szBuf;

	GetAMPMFromProfile(fForceDefault);

	/* DEFAULT .DOC AND .DOT EXTENSIONS */

	GetExtFromProfile(ipstDocExt, szDoc, fForceDefault);
	GetExtFromProfile(ipstDotExt, szDot, fForceDefault);
	GetExtFromProfile(ipstBakExt, szBak, fForceDefault);

	/* reset bitmap cache size */
	cKBitmap = GetProfileInt( szApp, SzKey("BITMAPMEMORY",BitmapMemory),
		fProtectModeWindows ? 256 : 64);

}


/* %%Function:GetExtFromProfile %%Owner:davidbo */
GetExtFromProfile(ipst, szExt, fForceDefault)
int ipst;
CHAR *szExt;
BOOL fForceDefault;
{
	int cch;
	CHAR szUser[10], szDefault[10];

	GetSzProfileIpst (ipst, szUser, szDefault, 10, fForceDefault);
	cch = CchSYSStripString(szUser, CchSz(szUser)-1);
	Assert(szDefault[0] == '.');
	if (szUser[0] != '.')
		{
		bltb(szUser, szUser+1, max(3, cch));
		szUser[0] = '.';
		}
	szUser[4] = 0;/* truncate if too long */
	QszUpper(szUser);
	if (FValidExtSYS(szUser))
		CchCopySz(szUser, szExt);
	else
		CchCopySz(szDefault, szExt);
	Assert(FValidExtSYS(szExt));
}



/* G E T  A M  P M  F R O M  P R O F I L E */
/* Get and Check and see that AM/PM setting is not confusing to Field
	Picture processor. */

csconst CHAR szFldPicReserved[] = {
	chFldPicReqDigit, chFldPicOptDigit, chFldPicTruncDigit,
			chFldPicNegSign,  chFldPicSign,     chFldPicExpSeparate,
			chFldPicMonth,    chFldPicDay,      chFldPicYear,
			chFldPicHour,     chFldPicMinute,   '\0'};


#define cchMaxFldPicRes	sizeof(szFldPicReserved)

/* %%Function:GetAMPMFromProfile %%Owner:davidbo */
STATIC NEAR GetAMPMFromProfile(fForceDefault)
BOOL fForceDefault;
{
	CHAR	*pch;
	int	isz;
	CHAR	chMorn, chEve;
	CHAR	szFldPicRes[cchMaxFldPicRes];
	CHAR    szDefAM[cchMaxMornEve], szDefPM[cchMaxMornEve];

	GetSzProfileIpst (ipstMorn, vitr.rgszMornEve[iszMorn], szDefAM,
			cchMaxMornEve, fForceDefault);
	GetSzProfileIpst (ipstEve, vitr.rgszMornEve[iszEve], szDefPM, 
			cchMaxMornEve, fForceDefault);

	QszLower(vitr.rgszMornEve[iszMorn]);
	QszLower(vitr.rgszMornEve[iszEve]);

	chMorn = vitr.rgszMornEve[iszMorn][0];
	chEve = vitr.rgszMornEve[iszEve][0];

	if ((chMorn == '\0') || (chEve == '\0') || (chMorn == chEve))
		{
lblInvalid:
		bltb(szDefAM, vitr.rgszMornEve[iszMorn], cchMaxMornEve);
		bltb(szDefPM, vitr.rgszMornEve[iszEve], cchMaxMornEve);
		return;
		}

	CchCopyLpszCchMax((CHAR FAR *) szFldPicReserved, (CHAR FAR *) szFldPicRes,
			cchMaxFldPicRes);

	for (isz = iszMorn; isz <= iszEve; isz++)
		{
		pch = vitr.rgszMornEve[isz];
		if (index(szFldPicRes,*pch) != NULL ||
				(*pch == vitr.chDecimal) ||
				(*pch == vitr.chThousand))
			{
			goto lblInvalid;
			}
		}
}




/* F  I N V A L I D  C H A R  S E T T I N G */
/* return fTrue iff the current international character setting
	ambiguates the parsing of computed fields. */
/* %%Function:FInvalidCharSetting %%Owner:davidbo */
STATIC NEAR FInvalidCharSetting()
{
	CHAR        *szInvalSepCh = SzFrameKey("+-*/^%()[]<>=!_",InvalSepCh);
	BOOL	fCurCheck;
	CHAR	*pch;
	CHAR        szT[4];

	fCurCheck = fFalse;
	for (pch = &vitr.szCurrency[0]; !fCurCheck && *pch != '\0'; pch++)
		{
		fCurCheck = index(szInvalSepCh, *pch) != NULL;
		}

	if (FAlphaNum(vitr.chDecimal)  ||
			FAlphaNum(vitr.chThousand) || FAlphaNum(vitr.chList)        ||
			index(szInvalSepCh, vitr.chDecimal)  != NULL                ||
			fCurCheck						    ||
			index(szInvalSepCh, vitr.chThousand) != NULL                ||
			index(szInvalSepCh, vitr.chList)     != NULL)
		{
		return (fTrue);
		}
	/* Checking against the first letter of szCurrency is sufficient
		to disambiguate. */
	szT[0] = vitr.szCurrency[0];
	szT[1] = vitr.chThousand;
	szT[2] = vitr.chList;
	szT[3] = '\0';
	return (index(szT, vitr.chDecimal) != NULL ||
			index(&szT[1], vitr.szCurrency[0]) != NULL);
}


/* Creates a heap block containing sz */
/* %%Function:HszCreateSYS %%Owner:davidbo */
CHAR (**HszCreateSYS(sz))[]
CHAR sz[];
{
	CHAR (**hsz)[];
	int cch = CchSz(sz);
	hsz = (CHAR (**)[]) HAllocateCb(cch);
	if (hsz != hNil)
		bltb(sz, **hsz, cch);
	return hsz;
}


/* %%Function:CchSYSStringString %%Owner:davidbo */
int CchSYSStripString(pch, cch)
CHAR *pch;
int cch;
{
	CHAR *pchNonBlank = pch;

	if (!cch || (pch[cch - 1] != ' ' && *pch != ' '))
		return (cch);
	while (cch && pch[cch - 1] == ' ') /* strip trailing spaces */
		--cch;
	while (cch && *pchNonBlank == ' ') /* strip spaces in front */
		{
		--cch;
		pchNonBlank++;
		}
	if (pchNonBlank != pch && cch > 0)
		bltbyte(pchNonBlank, pch, cch);

	Assert(cch >= 0);
	pch[cch] = 0;
	return (cch);
}  /* CchStripString */


/* F  V A L I D  E X T */
/* Is it a valid file extension (".xxx")? */
/* %%Function:FValidExtSYS %%Owner:davidbo */
BOOL FValidExtSYS(szExt)
CHAR    *szExt;
{
	if (CchSz(szExt) > ichMaxExtension)
		{
		return fFalse;
		}
	else  if (*szExt++ != '.')
		{
		return fFalse;
		}

	while (FAlphaNum(*szExt++));

	return (*(szExt - 1) == '\0');
}



/* U L M  F R O M  D R V  W I N  I N I */
/* Get the ulm (underline mode) from the driver and win.ini */
int UlmFromDrvWinIni()
{
	int ulm;
	char *pch;
	char *pchDriver;
	char szDriver[ichMaxProfileSz];
	HANDLE hDriver = 0L;
	extern int vwWinVersion;

	/* Read the ulm from the file */
	/* Do this first since the win.ini can override the printer driver */
	ulm = GetProfileInt(szApp,
			(LPSTR) SzFrameKey("UnderlineMode", UlineModeWININI),
			ulmNil);

	if (ulm == ulmNil)
		{
		ulm = ulmNormal;

		if (vwWinVersion > 0x0203)
			{
			/* Get the name of the driver, complete with extension. */
			pchDriver = *vpri.hszPrDriver;
			pch = bltbyte(pchDriver, szDriver, CchSz(pchDriver));
			bltbyte(SzFrameKey(".DRV",DRV), pch - 1, 5);

			/* The driver is not resident; attempt to load it. */
			if ((hDriver = LoadLibrary((LPSTR)szDriver)) < 32)
				/* Cannot load library */
				return ulm;

			if (!GetProcAddress(hDriver, MAKEINTRESOURCE(idoExtTextOut)))
				ulm = ulmAlwaysSpace;

			Assert(hDriver);
			FreeLibrary(hDriver);
			}
		}

	return ulm;

}
