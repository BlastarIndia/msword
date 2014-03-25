/* debugfnt.c - debugging code for fonts */

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "props.h"
#include "disp.h"
#include "doc.h"
#include "format.h"
#include "print.h"
#include "screen.h"
#include "sel.h"
#include "ch.h"
#include "message.h"
#include "prompt.h"
#include "debug.h"


#ifdef DEBUG
extern HANDLE vhInstance;

extern struct STTB  **vhsttbFont;

extern struct MERR  vmerr;

extern int          wwMac;
extern struct WWD   **mpwwhwwd[];
extern struct DOD   **mpdochdod[];
extern struct MERR  vmerr;
extern struct PRI   vpri;
extern struct SCI   vsci;

extern struct FLI   vfli;
extern int          vifceMac;
extern struct FCE   rgfce[ifceMax];

extern struct SEL   selCur;

extern struct FTI   vfti;
extern struct FTI   vftiDxt;


extern struct DBS   vdbs;

EXPORT BOOL FAR PASCAL  FontNameDebugEnum();
EXPORT BOOL FAR PASCAL  FontSizeDebugEnum();

int cAll;
int cSize;
HDC hdcFontDebugEnum;
int dxpInch, dypInch;
FARPROC lpFontNameDebugEnum;
FARPROC lpFontSizeDebugEnum;

/* CmdEnumAllFonts -- dump information about all available fonts

Enumerates all available fonts in the system, creating a text
dump of the fonts' metrics into the clipboard in Opus document form

*/


CmdEnumAllFonts(pcmb)
CMB *pcmb;
{
	struct CA ca;
	extern int vflm;
	int flm;

	cAll = 0;

/* validate the procinstances for the callback functions */

	if (!lpFontNameDebugEnum)
		{
		lpFontNameDebugEnum = MakeProcInstance( FontNameDebugEnum, vhInstance );
		lpFontSizeDebugEnum = MakeProcInstance( FontSizeDebugEnum, vhInstance );
		Assert( lpFontNameDebugEnum && lpFontSizeDebugEnum );
		}

	StartLongOp();
/* clear the scrap */

	SetWholeDoc( docScrap, PcaSetNil(&ca));
	flm = vflm;
	SetFlm( flmPrint );

/* enumerate. the real work is done in the enumeration callback functions. */

	/* printer fonts */
	if (vpri.hdc)
		{
		ScrapSz(SzShared("----------  START OF PRINTER FONT ENUMERATION  ---------"));
		ScrapCRLF();
		dxpInch = vfli.dxuInch;
		dypInch = vfli.dyuInch;
		EnumFonts( (hdcFontDebugEnum = vpri.hdc),
				NULL /*lpFaceName */, lpFontNameDebugEnum, 0L );
		ScrapSz(SzShared("----------  END OF PRINTER FONT ENUMERATION  ---------"));
		ScrapCRLF();
		ScrapCRLF();
		}

	/* screen fonts */
	ScrapSz(SzShared("----------  START OF SCREEN FONT ENUMERATION  ---------"));
	ScrapCRLF();
	dxpInch = vfli.dxsInch;
	dypInch = vfli.dysInch;
	EnumFonts( (hdcFontDebugEnum = GetDC(NULL)),
			NULL /*lpFaceName */, lpFontNameDebugEnum, 0L );
	ReleaseDC( NULL, hdcFontDebugEnum );
	ScrapSz(SzShared("----------  END OF SCREEN FONT ENUMERATION  ---------"));
	ScrapCRLF();
	ScrapCRLF();
	SetFlm( flm );

	Beep();


/* make Opus the clipboard owner */

	ChangeClipboard();

	EndLongOp( fFalse );

/* set informative prompt */

	SetPromptWMst (mstDbgFont, cAll, pdcmPmt+pdcmCreate+pdcmRestOnInput);
	return cmdOK;
}



/* F o n t  N a m e  D e b u g  E n u m */
/* This is the Windows callback function for font name (as opposed
	to size) enumeration.  This function is called once for each
	font available on the device. */

EXPORT BOOL far PASCAL FontNameDebugEnum( lplf, lptm, fty, dw )
LPLOGFONT  lplf;
LPTEXTMETRIC lptm;
int fty;            /* font type:
	fty & RASTER_FONTTYPE == fRasterFont
	fty & DEVICE_FONTTYPE == fDeviceFont */
DWORD dw;
{
	CHAR rgch [LF_FACESIZE];
	­
			bltbx( lplf->lfFaceName, (LPSTR)rgch, LF_FACESIZE );

	cSize = 0;

	ScrapSz(SzShared("----------  "));
	ScrapSz(rgch);
	ScrapSz(SzShared("  ----------"));
	ScrapCRLF();
	ScrapCRLF();

	EnumFonts( hdcFontDebugEnum,
			(LPSTR)rgch,
			lpFontSizeDebugEnum, 0L );
}


/* F o n t  S i z e  D e b u g  E n u m */
/* This is the Windows callback function for font size
	enumeration.  This function is called once for each
	font size available on the device. */

EXPORT BOOL far PASCAL FontSizeDebugEnum( lplf, lptm, fty, dw )
LPLOGFONT  lplf;
LPTEXTMETRIC lptm;
int fty;            /* font type:
	fty & RASTER_FONTTYPE == fRasterFont
	fty & DEVICE_FONTTYPE == fDeviceFont */
DWORD dw;
{
	int dza, ps;
	int dyp;

	cAll++;

	ScrapSzNum(SzShared("Font number: "), ++cSize );  
	ScrapCRLF();
	if (fty & DEVICE_FONTTYPE)
		ScrapSz(SzShared("Device Font"));
	else
		ScrapSz(SzShared("Raster Font"));
	ScrapCRLF();

	dyp = lptm->tmHeight - lptm->tmInternalLeading;
	ScrapSzNum(SzShared("Height (NOT including internal leading): "),dyp);
	dza = NMultDiv(dyp, czaInch, dypInch);
	ScrapSzNum(SzShared(" ("), dza);
	ScrapSz(" twips");
	ps = (dza+10) / 20;
	ScrapSzNum(SzShared(" / "),ps);
	if (ps * 20 > dza)
		ScrapSz(SzShared("-"));
	else  if (ps * 20 < dza)
		ScrapSz(SzShared("+"));
	ScrapSz(SzShared(" pts)"));
	ScrapCRLF();

	ScrapSzNum(SzShared("Width: "), lplf->lfWidth );
	ScrapSzNum(SzShared(" ("),NMultDiv(lplf->lfWidth, czaInch, dxpInch) );
	ScrapSz(" twips)");  
	ScrapCRLF();

	if (lptm->tmInternalLeading != 0)
		{
		ScrapSzNum(SzShared("Internal Leading: "),lptm->tmInternalLeading);
		ScrapCRLF();
		}

	if (lptm->tmExternalLeading != 0)
		{
		ScrapSzNum(SzShared("External Leading: "),lptm->tmExternalLeading);
		ScrapCRLF();
		}

	if (lplf->lfEscapement != 0)
		{
		ScrapSzNum(SzShared("Escapement: "),lplf->lfEscapement );
		ScrapCRLF();
		}
	if (lplf->lfOrientation != 0)
		{
		ScrapSzNum(SzShared("Orientation: "),lplf->lfOrientation );
		ScrapCRLF();
		}


	ScrapSz(SzShared("Other Attributes: "));
	if (lplf->lfWeight != 400)
		{
		if (lplf->lfWeight == 700)
			ScrapSz(SzShared("Bold "));
		else
			{
			ScrapSz(SzShared("Weight="));
			ScrapNum(lplf->lfWeight);
			ScrapSz(SzShared(" "));
			}
		}
	if (lplf->lfItalic)
		ScrapSz(SzShared("Italic "));
	if (lplf->lfUnderline)
		ScrapSz(SzShared("Underlined "));
	if (lplf->lfStrikeOut)
		ScrapSz(SzShared("Strikeout "));
	if (lplf->lfCharSet != ANSI_CHARSET)
		ScrapSz(SzShared("Non-Ansi "));
	ScrapCRLF();

/* Text metrics and logical font should match */

	if (lplf->lfWeight != lptm->tmWeight ||
			lplf->lfItalic != lptm->tmItalic ||
			lplf->lfStrikeOut != lptm->tmStruckOut ||
			lplf->lfUnderline != lptm->tmUnderlined)
		{ 
		ScrapSz(SzShared("LF != TM!!!!"));  
		ScrapCRLF();  
		}

/* pitch and family */

	ScrapSz(SzShared("Family (Pitch): "));
	switch (lplf->lfPitchAndFamily & 0xf0 ) 
		{
	case FF_ROMAN: 
		ScrapSz( SzShared( "Roman" ));  
		break;
	case FF_SWISS: 
		ScrapSz( SzShared( "Swiss" ));  
		break;
	case FF_MODERN: 
		ScrapSz( SzShared( "Modern" ));  
		break;
	case FF_SCRIPT: 
		ScrapSz( SzShared( "Script" ));  
		break;
	case FF_DECORATIVE: 
		ScrapSz( SzShared( "Decorative" ));  
		break;
	default: 
		ScrapSz( SzShared( "UNKNOWN????" ));  
		break;
		}
	switch (lplf->lfPitchAndFamily & 3 ) 
		{
	case FIXED_PITCH: 
		ScrapSz( SzShared( " (Fixed)" ));  
		break;
	case VARIABLE_PITCH: 
		ScrapSz( SzShared( " (Variable)" ));  
		break;
	default: 
		ScrapSz( SzShared( " (UNKNOWN????)" ));  
		break;
		}
	ScrapCRLF();

/* aspect ratio intended for */

	ScrapSz(SzShared("Target Device Aspect Ratio  X/Y: "));
	ScrapNum(lptm->tmDigitizedAspectX);
	ScrapSz(SzShared(" / "));
	ScrapNum(lptm->tmDigitizedAspectY);

	ScrapCRLF();
	ScrapCRLF();
}


ScrapSzNum(sz,num)
char *sz;
int num;
{
	ScrapSz(sz);
	ScrapNum(num);
}


ScrapNum(num)
int num;
{
	char rgch [20], *pch;

	pch = rgch;
	CchIntToPpch( num, &pch );
	*pch = '\0';
	ScrapSz(rgch);
}



ScrapSz(sz)
char *sz;
{
	FInsertRgch( docScrap, CpMacDoc(docScrap) - ccpEol,
			sz, CchSz(sz)-1, &selCur.chp, NULL );
}


ScrapCRLF()
{
	struct PAP pap;

	StandardPap(&pap);
	FInsertRgch( docScrap, CpMacDoc(docScrap) - ccpEol,
			SzShared("\r\n"), 2, &selCur.chp, &pap );
}


#endif  /* DEBUG */
