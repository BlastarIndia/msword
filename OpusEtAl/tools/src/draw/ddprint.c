#include <string.h>
#include <windows.h>
#include "ddall.h"
#include "dddlg.h"

extern	HWND	vhwnd,
vhDlgPrint;
extern	HANDLE  vhInst;
extern  HBRUSH  hbrBlack;

extern  FARPROC	lpfnAbort, lpfnPrintDlg;
extern	char	vszApp[];
extern  BOOL	vfOnePage,
vfUserAbort,
vfLogFont,
vfTextMetric,
vfSpecial;
extern  DUD	vdud;
extern  PRI	vpri;
extern  UFD	vufd;
extern	ESC	vrgesc[];


extern  LONG  	MultDiv();
FAR PASCAL EnumAllFaces();
FAR PASCAL EnumAllFonts();
BOOL FAR PASCAL AbortProc();
BOOL FAR PASCAL PrintDlg();
BOOL FAR PASCAL FontDlg();
BOOL FAR PASCAL OptionsDlg();
BOOL FAR PASCAL EscapeDlg();
HDC  HDCPrinterFromProfile();

/* C M D   P R I N T E R   S E T U P  */
/* Get the printer DC and prompt user to changed printer settings.
	Note: we could already have a dc */
CmdPrinterSetup()
{
	if (vpri.hdc)
		DeleteDC(vpri.hdc);

	if (HDCPrinterFromProfile())
		PrinterSetup();
	else
		ReportSz("Error getting printer dc");
}


/* C M D   O P T I O N S */
/* Bring up the options dialog box so user can set options. */
CmdOptions()
{
	FARPROC lpfnDlgProc;
	BOOL	fFont;

	/* Note: we gotta have the printer dc so that we can calc pts.
			correctly!! */
	if (!vpri.hdc)
		if (!HDCPrinterFromProfile())
			{
			ReportSz("Error getting printer dc");
			return;
			}

	lpfnDlgProc = MakeProcInstance(OptionsDlg, vhInst);
	fFont = DialogBox(vhInst, "OptionsDlg", vhwnd, lpfnDlgProc);
	FreeProcInstance(lpfnDlgProc);
	if (fFont == idd_FONT)
		ExecFontDlg(fTrue);
}


/* C M D   E N U M   F O N T S */
/*   Enumerate and print out all the fonts available for the current printer.
		For each font print it once with normal, expanded and compressed
		spacing and again with same underlined.
			
		This is a two step process.  First get all the facenames; then enumerate
		each facename to get all the sizes. */
CmdEnumFonts()
{
	FFL		ffl;	      	/* collects all the facenames */
	LPSTR		lpFace;		/* points to facename list */
	int		c;		/* count of facenames */
	FARPROC		lpfnEnumAllFaces,
			lpfnEnumAllFonts;

	/* get the printer dc, if we don't have it */
	if (!vpri.hdc)
		if (!HDCPrinterFromProfile())
			{
			ReportSz("Error getting printer dc");
			return;
			}

	if ((ffl.hGMem = GlobalAlloc(GHND, 1L)) == NULL)
		{
		ReportSz("Not enough memory to enumerate fonts.");
		return fFalse;
		}

	ffl.cFace = 0;

	/* first pass:  get all the facenames. */
	lpfnEnumAllFaces = MakeProcInstance( (FARPROC)EnumAllFaces,
			vhInst);
	WAssert(EnumFonts(vpri.hdc, NULL, lpfnEnumAllFaces, (LPSTR) &ffl),
			(LPSTR)"No Memory!");
	FreeProcInstance(lpfnEnumAllFaces);
	lpFace = GlobalLock(ffl.hGMem);

	lpfnEnumAllFonts = MakeProcInstance( (FARPROC)EnumAllFonts,
			vhInst);
	/* first set up the abort Proc */
	SetAbortProc("Enumerating Fonts","Typeface #: ","0");

	/* prepare to begin printing */
	WAssert(Escape(vpri.hdc,STARTDOC,15,(LPSTR) "ENUMERATEDFONTS",
			(LPSTR) NULL) > 0,  (LPSTR)"STARTDOC")

			/* enumerate each fontname and print the fonts */
	for (c = 0; !vfUserAbort && c < ffl.cFace; c++)
		{
		EnumFonts(vpri.hdc, lpFace+c*LF_FACESIZE,
				lpfnEnumAllFonts, (LPSTR) NULL);

		SetAbortProcNum(c+1);

		if (vfOnePage)
			NewPage(NULL);
		}

	if (!vfUserAbort)
		{
		SetDlgItemText(vhDlgPrint, idd_pdt3, (LPSTR)"DONE");
		if (!vfOnePage)
			NewPage(NULL);
		}

	/* End printing */
	WAssert(Escape(vpri.hdc,ENDDOC,NULL,(LPSTR)NULL,(LPSTR)NULL)>0,
			(LPSTR)"ENDDOC");

	FreeProcInstance(lpfnEnumAllFonts);
	GlobalUnlock(ffl.hGMem);
	GlobalFree(ffl.hGMem);

	UnsetAbortProc();
}


/* C M D   E S C A P E */
/*   Test escape codes.  First put up db for user to check which escape
codes to test.  Then test the codes.

		Notes:  the global  vrgesc.iSt field tells what the current state of
			each escape is.  There are three choices: checked, unchecked or
			not supported.
			the idd returned from the dialog tell how the user exited. There
			are three choices: cancel,  Ok,  or  all. */
CmdEscape()
{
	FARPROC lpfnDlgProc;
	int		idd;

	/* get printer dc. */
	if (!vpri.hdc)
		if (!HDCPrinterFromProfile())
			{
			ReportSz("Error getting printer dc");
			return;
			}

	/* do the escape code dialog box */
	lpfnDlgProc = MakeProcInstance(EscapeDlg, vhInst);
	idd = DialogBox(vhInst, "EscapeDlg", vhwnd, lpfnDlgProc);
	FreeProcInstance(lpfnDlgProc);

	/* Now test the escape codes */
	if (idd)
		ExecEscTests(idd);
}


/* C M D   F O N T */
/* print out each of the characters in a font.  */
CmdFont()
{
	HFONT		hFont,			/* font to be printed */
	hFontOld;
	char		szFont[ichMaxSz];
	BYTE		ch;    			/* character code */
	TEXTMETRIC	tm;
	short 		xduLoc,			/* x location [device units]*/
	xduCol;			/* x column offset */

	/* get the printer dc. */
	if (!vpri.hdc)
		if (!HDCPrinterFromProfile())
			{
			ReportSz("Error getting printer dc");
			return;
			}

	SetAbortProc("Printing Char Set.","Character :","0");

	/* prepare to print */
	Escape(vpri.hdc,STARTDOC,8,(LPSTR) "SHOWFONT",(LPSTR) NULL);

	/* if the user wants it, here is were we show them the LogFont */
	if (vfLogFont)
		{
		DisplayLogFont((LPLOGFONT)&vufd.lf, NULL);
		NewPage(NULL);
		}

	/* Now create the font */
	hFont = CreateFontIndirect(&(vufd.lf));
	hFontOld = SelectObject(vpri.hdc, hFont);


	GetTextMetrics(vpri.hdc, (LPTEXTMETRIC)&tm);
	if (vfTextMetric)
		{
		SelectObject(vpri.hdc, hFontOld);
		DisplayTextMetric((LPTEXTMETRIC) &tm);
		SelectObject(vpri.hdc, hFont);
		}

	/* get some measurements */
	vdud.yLin = tm.tmHeight + tm.tmExternalLeading;
	xduCol = 20 * tm.tmAveCharWidth;
	xduLoc = 0;

	/* print the font name */
	SzFontName(szFont, (LPTEXTMETRIC) &tm, -1);
	TextOut(vpri.hdc, 0, vdud.yLoc, (LPSTR) szFont, CchSz(szFont));
	NewLine(hFont);
	NewLine(hFont);

	/* print all the chars */
	for (ch = 0x20; ch < 0x80; ch++)
		WriteChar(ch, &xduLoc, xduCol, hFont);

	WriteChar(0x91, &xduLoc, xduCol, hFont);
	WriteChar(0x92, &xduLoc, xduCol, hFont);

	for (ch = 0xa0; ch > 0x00; ch++)
		WriteChar(ch, &xduLoc, xduCol, hFont);

	NewPage(NULL);
	Escape(vpri.hdc,ENDDOC,NULL,(LPSTR)NULL,(LPSTR)NULL);

	SelectObject(vpri.hdc, hFontOld);

	DeleteObject(hFont);

	UnsetAbortProc();
}


/* C M D   T E S T   P R */
/*  test printing a specific font.  This functions is to attempt to most
	closely duplicate the situation in which Win Word prints */
CmdTestPr()
{
	HFONT		hFont,
			hFontOld;
	char		sz[ichMaxSz],
	*pch;
	TEXTMETRIC	tm;

	/* get the printer dc. */
	if (!vpri.hdc)
		if (!HDCPrinterFromProfile())
			{
			ReportSz("Error getting printer dc");
			return;
			}

	SetAbortProc("Testing Printing", "Please Wait."," ");

	/* prepare to print */
	Escape(vpri.hdc,STARTDOC,13,(LPSTR) "FONTPRINTTEST",(LPSTR) NULL);

	/* Display the LogFont */
	if (vfLogFont)
		DisplayLogFont((LPLOGFONT) &vufd.lf);

	hFont = CreateFontIndirect((LPLOGFONT) &vufd.lf);
	hFontOld = SelectObject(vpri.hdc, hFont);

	GetTextMetrics(vpri.hdc, &tm);
	if (vfTextMetric)
		{
		SelectObject(vpri.hdc, hFontOld);
		DisplayTextMetric((LPTEXTMETRIC) &tm);
		SelectObject(vpri.hdc, hFont);
		}

	sz[0] = 0;
	SzSzAppend(sz, "The Current Font is:");
	for (pch = sz; *pch; pch++)
		;
	SzFontName(pch, (LPTEXTMETRIC) &tm, -1);
	ExtTextOut(vpri.hdc, 50, vdud.yLoc, 0, (LPRECT)0, sz,
			CchSz(sz), (LPINT)NULL);

	NewPage(NULL);
	Escape(vpri.hdc,ENDDOC,NULL,(LPSTR)NULL,(LPSTR)NULL);

	/* Clean Up Print Stuff *******/

	SelectObject(vpri.hdc, hFontOld);

	DeleteObject(hFont);

	UnsetAbortProc();
}


/* C M D   C U S T O M I Z E  P R */
/* This fuction has a printing shell that can be customized
	to test different things */
CmdCustomizePr()
{
	int dxa;
	int xaWidth;
	int ch;
	int yLoc;
	char rgch[32];
	int i;
	HFONT		hFont,
			hFontOld;
	char		sz[ichMaxSz],
	*pch;
	TEXTMETRIC	tm;
	RECT vrcBand;

	/* get the printer dc. */
	if (!vpri.hdc)
		if (!HDCPrinterFromProfile())
			{
			ReportSz("Error getting printer dc");
			return;
			}

	SetAbortProc("Testing Printing", "Please Wait."," ");

	/* prepare to print */
	Escape(vpri.hdc,STARTDOC,13,(LPSTR) "FONTPRINTTEST",(LPSTR) NULL);


	/* NewPage(NULL); */
	Escape(vpri.hdc,ENDDOC,NULL,(LPSTR)NULL,(LPSTR)NULL);

	/* Clean Up Print Stuff *******/

	UnsetAbortProc();
}


/* ----------------------------------------------------------------- */
/* --------------------- Dialog Procedures ------------------------- */
/* ----------------------------------------------------------------- */


/* P R I N T   D L G */
/*   Procedure for handling the abort print job dialog. */
BOOL FAR PASCAL PrintDlg(hDlg, iMessage, wParam, lParam)
HWND	hDlg;
unsigned iMessage;
WORD	wParam;
DWORD	lParam;
{
	switch (iMessage)
		{
	case WM_INITDIALOG:
		SetWindowText(hDlg, vszApp);
		EnableMenuItem(GetSystemMenu(hDlg, fFalse), SC_CLOSE, MF_GRAYED);
		break;

	case WM_COMMAND:
		vfUserAbort = fTrue;
		EnableWindow(GetParent(hDlg), fTrue);
		DestroyWindow(hDlg);
		vhDlgPrint = 0;
		break;

	default:
		return fFalse;
		}
	return fTrue;
}


/* O P T I O N S   D L G */
/*   Procedure for handling the options dialog.
	Must set up the prompts based on Points per inch (hps vs. pts)
	Must fill in initial db values
	The FONT button Ok's the current settings and calls up fontdlg.
*/
BOOL FAR PASCAL OptionsDlg(hDlg, iMessage, wParam, lParam)
HWND		hDlg;
unsigned 	iMessage;
WORD		wParam;
LONG		lParam;
{
	BOOL   		Bool;

	switch (iMessage)
		{
	case WM_INITDIALOG:
		CheckDlgButton(hDlg, idd_Page, vfOnePage);
		CheckDlgButton(hDlg, idd_Half, (vdud.PtsInch == HALFPOINTS_INCH));
		CheckDlgButton(hDlg, idd_LogF, vfLogFont);
		CheckDlgButton(hDlg, idd_TexM, vfTextMetric);

		SetDlgItemInt(hDlg, idd_yOff,
				(int)MultDiv(vdud.yOff,vdud.PtsInch,vdud.yInch), fFalse);

		SetFocus(GetDlgItem(hDlg, idd_Page));
		return FALSE;

	case WM_COMMAND:
		switch (wParam)
			{
		case IDOK:
		case idd_FONT:
			vfOnePage = IsDlgButtonChecked(hDlg,idd_Page);
			vdud.PtsInch = IsDlgButtonChecked(hDlg,idd_Half) ?
					HALFPOINTS_INCH : POINTS_INCH;
			vfLogFont = IsDlgButtonChecked(hDlg,idd_LogF);
			vfTextMetric = IsDlgButtonChecked(hDlg,idd_TexM);
			vdud.yOff = (int) MultDiv(GetDlgItemInt(hDlg, idd_yOff,
					(BOOL FAR *) &Bool, fFalse), vdud.yInch, vdud.PtsInch);
			vdud.yLoc = vdud.yOff;
			WAssert(Bool, (LPSTR)"Error in Edit Box");
			if (wParam == idd_FONT)
				EndDialog(hDlg, idd_FONT);
			else
				EndDialog(hDlg, IDOK);
			break;

		case IDCANCEL:
			EndDialog(hDlg, FALSE);
			break;

		case idd_Page:
		case idd_Half:
		case idd_LogF:
		case idd_TexM:
			CheckDlgButton(hDlg, wParam,
					!IsDlgButtonChecked(hDlg,wParam));
			break;

		default:
			return FALSE;
			}
		break;

	default:
		return FALSE;
		}
	return TRUE;
}


/* E S C A P E   D L G */
BOOL FAR PASCAL EscapeDlg(hDlg, iMessage, wParam, lParam)
HWND		hDlg;
unsigned 	iMessage;
WORD		wParam;
LONG		lParam;
{
	int 	idd;
	char	sz[ichMaxSz];

	switch (iMessage)
		{
	case WM_INITDIALOG:
		sz[0] = 0;
		SzSzAppend(sz, vpri.szPrinter);
		SzSzAppend(sz, "; ");
		sz[19] =  '\0';
		SetDlgItemText(hDlg, idd_EscP1, (LPSTR)sz);

		sz[0] = 0;
		SzSzAppend(sz, vpri.szPrDriver);
		SzSzAppend(sz, "; ");
		sz[19] =  '\0';
		SetDlgItemText(hDlg, idd_EscP2, (LPSTR)sz);

		sz[0] = 0;
		SzSzAppend(sz, vpri.szPrPort);
		sz[19] = '\0';
		SetDlgItemText(hDlg, idd_EscP3, (LPSTR)sz);

		for (idd = idd_EscMin; idd < idd_EscMac; idd++)
			if (FEscSupport(idd))
				CheckDlgButton(hDlg, idd,
						(vrgesc[idd-idd_EscMin].iSt==iSt_ON));
			else
				EnableWindow(GetDlgItem(hDlg, idd), fFalse);
		for (; idd < idd_EscMax; idd++)
			{
			vrgesc[idd-idd_EscMin].iSt = iSt_NS;
			EnableWindow(GetDlgItem(hDlg,idd), fFalse);
			}
		SetFocus(GetDlgItem(hDlg, idd_EscMin));
		return FALSE;

	case WM_COMMAND:
		switch (wParam)
			{
		case IDOK:
			for (idd = idd_EscMin; idd < idd_EscMax; idd++)
				vrgesc[idd-idd_EscMin].iSt =
						IsDlgButtonChecked(hDlg, idd) ?
						iSt_ON : iSt_OFF;
			EndDialog(hDlg, idd_EscMin);
			break;

		case IDCANCEL:
			EndDialog(hDlg, FALSE);
			break;

		case idd_ALL:
			for (idd = idd_EscMin; idd < idd_EscMax; idd++)
				if (vrgesc[idd-idd_EscMin].iSt != iSt_NS)
					vrgesc[idd-idd_EscMin].iSt =
							IsDlgButtonChecked(hDlg, idd) ?
							iSt_ON : iSt_OFF;
			EndDialog(hDlg, idd_ALL);
			break;

		case idd_Esc1:
		case idd_Esc2:
		case idd_Esc3:
		case idd_Esc4:
		case idd_Esc5:
		case idd_Esc6:
		case idd_Esc7:
		case idd_Esc8:
		case idd_Esc9:
		case idd_Esc0:
		case idd_Esca:
		case idd_Escb:
		case idd_Escc:
		case idd_Escd:
		case idd_Esce:
		case idd_Escf:
		case idd_Escg:
		case idd_Esch:
		case idd_Esci:
		case idd_Escj:
			CheckDlgButton(hDlg, wParam,
					!IsDlgButtonChecked(hDlg,wParam));
			break;

		default:
			return FALSE;
			}
		break;

	default:
		return FALSE;
		}
	return TRUE;
}


/* F O N T   D L G */
/*  Procedure to handle the font dialog.  The purpose of the dialog is to
	allow the user to enter a customized LogFont.  
*/
BOOL FAR PASCAL FontDlg(hDlg, iMessage, wParam, lParam)
HWND		hDlg;
unsigned 	iMessage;
WORD		wParam;
LONG		lParam;
{
	short	idd,
	cch;

	BOOL	bool;

	/* Note: if the user picks Cancel we do not make any changes so
			we can not change the vufd structure till we get an OK.
			The next three bytes keep the current db selections. */
	static 	BYTE	bCharSet,	     	/* current CharSet choice in db */
	bQuality,		/* current Quality choice in db */
	bPitch;			/* current Pitch choice in db */

	char 	sz[ichMaxSz];


	switch (iMessage)
		{
	case WM_INITDIALOG:
		/* change promtps to reflect current units (pts or hps) */
		if (vdud.PtsInch == HALFPOINTS_INCH)
			{
			SetDlgItemText(hDlg, idd_FHPT, (LPSTR)"hps");
			SetDlgItemText(hDlg, idd_FWPT, (LPSTR)"hps");
			}

		/* Use as Default checkbox -- This is for escape testing. */
		CheckDlgButton(hDlg, idd_DEFF, vufd.fHaveFont);

		/* initialize db to current settings of LogFont */
		SetDlgItemInt(hDlg, idd_FHP, (int)
				MultDiv(ABS(vufd.lf.lfHeight),vdud.PtsInch,vdud.yInch), fFalse);
		SetDlgItemInt(hDlg, idd_FWP, (int)
				MultDiv(vufd.lf.lfWidth,vdud.PtsInch,vdud.xInch), fFalse);

		CheckDlgButton(hDlg, idd_Bold,
				(vufd.lf.lfWeight > (FW_BOLD+FW_NORMAL)/2));
		CheckDlgButton(hDlg, idd_Ital, vufd.lf.lfItalic);
		CheckDlgButton(hDlg, idd_Unde, vufd.lf.lfUnderline);
		CheckDlgButton(hDlg, idd_Stri, vufd.lf.lfStrikeOut);

		switch (vufd.lf.lfCharSet)
			{
		case ANSI_CHARSET:
			idd = idd_Ansi;
			break;
		case OEM_CHARSET:
			idd = idd_OEM;
			break;
		case SHIFTJIS_CHARSET:
			idd = idd_Kanj;
			break;
			}
		CheckRadioButton(hDlg, idd_Ansi, idd_OEM, idd);

		switch (vufd.lf.lfQuality)
			{
		case DEFAULT_QUALITY:
			idd = idd_DeQ;
			break;
		case DRAFT_QUALITY:
			idd = idd_Dra;
			break;
		case PROOF_QUALITY:
			idd = idd_Pro;
			break;
			}
		CheckRadioButton(hDlg, idd_Dra, idd_Pro, idd);

		switch (vufd.lf.lfPitchAndFamily)
			{
		case DEFAULT_PITCH:
			idd = idd_DeP;
			break;
		case FIXED_PITCH:
			idd = idd_Fix;
			break;
		case VARIABLE_PITCH:
			idd = idd_Var;
			break;
			}
		CheckRadioButton(hDlg, idd_DeP, idd_Var, idd);

		SetDlgItemText(hDlg, idd_FN, (LPSTR)(vufd.lf.lfFaceName));

		SetFocus(GetDlgItem(hDlg, idd_FN));
		return FALSE;

	case WM_COMMAND:
		switch (wParam)
			{
		case IDOK:
			LoadUFD(hDlg,bCharSet,bQuality,bPitch);
			EndDialog(hDlg, TRUE);
			break;

		case IDCANCEL:
			EndDialog(hDlg, FALSE);
			break;

			/* change the du meter to reflect the current entry in
			height edit box */
		case idd_FHP:
			sz[0] = 0;
			SzFromInt((int) MultDiv(
					GetDlgItemInt(hDlg,idd_FHP,(BOOL FAR *) &bool,
					fFalse),vdud.yInch,vdud.PtsInch), sz, &cch);
			SzSzAppend(sz," du");
			SetDlgItemText(hDlg, idd_FHPu, (LPSTR)sz);
			return FALSE;

			/* same as above but for width. */
		case idd_FWP:
			sz[0] = 0;
			SzFromInt((int) MultDiv(
					GetDlgItemInt(hDlg,idd_FWP,(BOOL FAR *) &bool,
					fFalse),vdud.xInch,vdud.PtsInch), sz, &cch);
			SzSzAppend(sz," du");
			SetDlgItemText(hDlg, idd_FWPu, (LPSTR)sz);
			return FALSE;

		case idd_Ansi:
			bCharSet = ANSI_CHARSET;
			CheckRadioButton(hDlg, idd_Ansi, idd_OEM, wParam );
			break;

		case idd_Kanj:
			bCharSet = SHIFTJIS_CHARSET;
			CheckRadioButton(hDlg, idd_Ansi, idd_OEM, wParam );
			break;

		case idd_OEM:
			bCharSet = OEM_CHARSET;
			CheckRadioButton(hDlg, idd_Ansi, idd_OEM, wParam );
			break;

		case idd_Dra:
			bQuality = DRAFT_QUALITY;
			CheckRadioButton(hDlg, idd_Dra, idd_Pro, wParam );
			break;

		case idd_DeQ:
			bQuality = DEFAULT_QUALITY;
			CheckRadioButton(hDlg, idd_Dra, idd_Pro, wParam );
			break;

		case idd_Pro:
			bQuality = PROOF_QUALITY;
			CheckRadioButton(hDlg, idd_Dra, idd_Pro, wParam );
			break;

		case idd_DeP:
			bPitch = DEFAULT_PITCH;
			CheckRadioButton(hDlg, idd_DeP, idd_Var, wParam );
			break;

		case idd_Fix:
			bPitch = FIXED_PITCH;
			CheckRadioButton(hDlg, idd_DeP, idd_Var, wParam );
			break;

		case idd_Var:
			bPitch = VARIABLE_PITCH;
			CheckRadioButton(hDlg, idd_DeP, idd_Var, wParam );
			break;

		case idd_DEFF:
		case idd_Ital:
		case idd_Bold:
		case idd_Unde:
		case idd_Stri:
			CheckDlgButton(hDlg, wParam,
					!IsDlgButtonChecked(hDlg,wParam));
			break;

		default:
			return FALSE;
			}
		break;

	default:
		return FALSE;
		}
	return TRUE;
}


/* ------------------------------------------------------------------ */
/* --------------------- Print  Utilities --------------------------- */
/* ------------------------------------------------------------------ */

/* H D C  P R I N T E R  F R O M  P R O F I L E */
/* Read the device name, driver name, and port name of the windows default
	printer sz fields of vpri.  Return fFalse if there was no
	valid default printer, or if we ran out of memory allocating the strings

	Accepted format:

	[windows]
	device=Pr Name,Driver,Port

*/
HDC HDCPrinterFromProfile()
{
	char szPrinter[ichMaxSz];
	char chNull = '\0';
	char *pch, *pchT;
	char *pchDriver;
	char *pchPort;
	char szDescPrinter[ichMaxSz];

	TEXTMETRIC tm;

	/* Read [windows] device = into szPrinter */

	GetProfileString("windows","device", "", (LPSTR)szPrinter, ichMaxSz);

	if ((pch = strchr(szPrinter, ',')) == NULL)
		return fFalse;

	/* Remove any trailing spaces from the printer name. */
	for ( pchT = pch;  *pchT == ' ' && pchT > &szPrinter[0];  pchT -- )
		;
	*pchT = '\0';

	/* Parse out the port and the driver names. */
	ParseDeviceSz(pch + 1, &pchPort, &pchDriver);

	WAssert((*pchPort != '\0' && *pchDriver != '\0'),(LPSTR)"Empty Port or Driver")

			bltbx((LPSTR)szPrinter, (LPSTR)vpri.szPrinter, CchSz(szPrinter)+1);
	bltbx((LPSTR)pchDriver, (LPSTR)vpri.szPrDriver, CchSz(pchDriver)+1);
	bltbx((LPSTR)pchPort, (LPSTR)vpri.szPrPort, CchSz(pchPort)+1);


	vpri.hdc = CreateDC((LPSTR)vpri.szPrDriver, (LPSTR)vpri.szPrinter,
			(LPSTR)vpri.szPrPort, NULL);

	WAssert(vpri.hdc,(LPSTR)"Could Not Create Printer DC!");
	GetTextMetrics(vpri.hdc, (LPTEXTMETRIC)&tm);
	vdud.yMaxPage = GetDeviceCaps(vpri.hdc, VERTRES);
	vdud.yInch    = GetDeviceCaps(vpri.hdc, LOGPIXELSY);
	vdud.xInch    = GetDeviceCaps(vpri.hdc, LOGPIXELSX);

	return vpri.hdc;
}


/* P R I N T E R   S E T U P */
/* Ask user if they want to change printer settings.  If so let the printer
	driver handle the actual work. */
PrinterSetup()
{
	char	szPrinterInfo[ichMaxSz];
	HANDLE  hLib;
	FARPROC	lpfnDM;

	szPrinterInfo[0] = 0;
	SzSzAppend(szPrinterInfo, "Current Printer: ");
	SzSzAppend(szPrinterInfo, vpri.szPrinter);
	SzSzAppend(szPrinterInfo, " on ");
	SzSzAppend(szPrinterInfo, vpri.szPrPort);
	SzSzAppend(szPrinterInfo, "  Change Printer Setup?");
	if (IDYES == MessageBox(vhwnd,
			(LPSTR) szPrinterInfo,
			(LPSTR)	vszApp,
			MB_YESNO | MB_DEFBUTTON2 | MB_ICONQUESTION))
		{
		szPrinterInfo[0] = 0;
		SzSzAppend(szPrinterInfo, vpri.szPrDriver);
		SzSzAppend(szPrinterInfo, ".DRV");
		hLib = LoadLibrary(szPrinterInfo);
		if (hLib >= 32)
			{
			lpfnDM = GetProcAddress(hLib, "DEVICEMODE");
			(*lpfnDM) (vhwnd, hLib, (LPSTR) vpri.szPrinter,
					(LPSTR) vpri.szPrPort);
			FreeLibrary(hLib);
			return fTrue;
			}
		else
			return fFalse;
		}
}


int ParseDeviceSz(sz, ppchPort, ppchDriver)
char sz[];
char **ppchPort;
char **ppchDriver;
{
	/* This routine takes a string that came from the "device" entry in the user
	profile and returns in *ppchPort and *ppchDriver pointers to the port and
	driver sutible for a CreateDC() call.  If no port is found in the string,
	*ppchPort will point to a string containing the name of the null device.
	This routine returns the number of ports for this printer (separated by null
	characters in the string pointed at by *ppchPort).  NOTE: sz may be modified
	by this routine, and the string at *ppchPort may not be a substring of sz
	and should not be modified by the caller. */

	register char *pch;
	int cPort = 0;
	char szNone[4];

	SzSzAppend(szNone, "None");

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
				register char *pchT = pch;

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
				bltbx(pchT, pch, CchSz(pchT));
				}
EndFound:
			;
			}
		}

	/* Parse the ".drv" out of the driver. */
	if ((pch = strchr(*ppchDriver, '.')) != 0 && !strncmp(pch, ".drv", 4))
		{
		*pch = '\0';
		}

	return (cPort);
}


int FAR PASCAL	EnumAllFaces(plf, ptm, nFontType, lpffl)
LPLOGFONT		plf;
LPTEXTMETRIC	ptm;
short			nFontType;
FFL	FAR			*lpffl;
{
	LPSTR		lpFace;
	short		c;

	if (GlobalReAlloc(lpffl->hGMem,(DWORD) LF_FACESIZE *
			(1 + lpffl->cFace), GMEM_MOVEABLE) == NULL)
		WAssert(fFalse,(LPSTR)"NOMEM");

	lpFace = GlobalLock(lpffl->hGMem);

	for (c=0; c < lpffl->cFace; c++)
		if (FCompLpsz((LPSTR)plf->lfFaceName, lpFace + c * LF_FACESIZE))
			{
			GlobalUnlock(lpffl->hGMem);
			return fTrue;
			}

	bltbx((LPSTR) plf->lfFaceName, lpFace + lpffl->cFace * LF_FACESIZE, LF_FACESIZE);

	GlobalUnlock(lpffl->hGMem);
	lpffl->cFace++;
	return fTrue;
}


int FAR PASCAL 	EnumAllFonts(lplf, lptm, nFontType, dummy)
LPLOGFONT	lplf;
LPTEXTMETRIC	lptm;
short		nFontType;
LPSTR		dummy;
{
	char	szFONT[ichMaxSz];
	HFONT	hfont, hfontUnd, hfontOld;
	TEXTMETRIC		tm;
	WORD	mpchdx[ichMaxSz];

	if (vfLogFont)
		DisplayLogFont(lplf,nFontType);
	if (vfTextMetric)
		DisplayTextMetric(lptm);

	WAssert(hfont = CreateFontIndirect(lplf), (LPSTR)"CreateFontIndirect");
	WAssert(hfontOld = SelectObject(vpri.hdc, hfont), (LPSTR)"SelectObject");

	GetTextMetrics(vpri.hdc, lptm);

	if (vfTextMetric && vfSpecial)
		{
		SelectObject(vpri.hdc, hfontOld);
		DisplayTextMetric((LPTEXTMETRIC) &tm);
		SelectObject(vpri.hdc, hfont);
		}

	SzFontName(szFONT, lptm, nFontType);

	if ((vdud.yLoc + lptm->tmHeight + lptm->tmExternalLeading) > vdud.yMaxPage)
		NewPage(hfont);
	ModifyText(szFONT, mpchdx);
	WriteText(szFONT, mpchdx);
	vdud.yLoc += (lptm->tmHeight + lptm->tmExternalLeading);

	if ((vdud.yLoc + lptm->tmHeight + lptm->tmExternalLeading) > vdud.yMaxPage)
		NewPage(hfont);
	if (GetDeviceCaps(vpri.hdc,TEXTCAPS) & TC_UA_ABLE)
		{
		lplf->lfUnderline = fTrue;
		hfontUnd = CreateFontIndirect(lplf);
		SelectObject(vpri.hdc, hfontUnd);

		WriteText(szFONT, mpchdx);

		SelectObject(vpri.hdc, hfont);
		DeleteObject(hfontUnd);
		}
	else
		{
		DxOfSz(mpchdx, "Can not underline preceeding font.", 0);
		WriteText("Can not underline preceeding font.", mpchdx);
		}

	vdud.yLoc += (lptm->tmHeight + lptm->tmExternalLeading);

	SelectObject(vpri.hdc, hfontOld);
	DeleteObject(hfont);
}


SzFontName(pch, lptm, nFontType)
char	*pch;
LPTEXTMETRIC	lptm;
short	nFontType;
{
	char	sz[ichMaxSz];
	int	pt, ypu, ct;

	GetTextFace(vpri.hdc, LF_FACESIZE, pch);

	if (lptm->tmWeight == FW_BOLD)
		SzSzAppend(pch, " Bold");
	if (lptm->tmItalic)
		SzSzAppend(pch, " Italic");
	if (lptm->tmUnderlined)
		SzSzAppend(pch, " Underline");
	if (lptm->tmStruckOut)
		SzSzAppend(pch, " StrikeOut");

	SzSzAppend(pch, "  --  ");

	ypu = lptm->tmHeight - lptm->tmInternalLeading;
	pt = (int) MultDiv(ypu, vdud.PtsInch, vdud.yInch);

	SzFromInt(pt, sz, &ct);
	SzSzAppend(pch, sz);

	if (vdud.PtsInch == POINTS_INCH)
		SzSzAppend(pch, " pt.");
	else
		SzSzAppend(pch, " hps.");

	if (!(nFontType < 0 ))
		if (!(nFontType & DEVICE_FONTTYPE))
			if (nFontType & RASTER_FONTTYPE)
				SzSzAppend(pch, "Screen Raster Font");
			else
				SzSzAppend(pch, "Screen Vector Font");
}


ModifyText(pch, mpchdx)
char 	*pch;
WORD	mpchdx[ichMaxSz];
{
	short	cch;
	char	*pchT;

	SzSzAppend(pch,"    ");
	DxOfSz(mpchdx, pch, 0);

	cch = CchSz(pch);
	pchT = pch;
	for (;*pchT;pchT++)
		;

	SzSzAppend(pch,"Expanded Text   ");
	DxOfSz(mpchdx+cch, pchT, 3);

	cch = CchSz(pch);
	pchT = pch;
	for (;*pchT;pchT++)
		;

	SzSzAppend(pch,"Compressed Text");
	DxOfSz(mpchdx+cch, pchT, -3);
}



WriteText(pch, mpchdx)
char 	*pch;
WORD	mpchdx[ichMaxSz];
{
	BOOL	fError;

	fError = ExtTextOut(vpri.hdc, 5, vdud.yLoc, (WORD)0, (LPRECT)0,
			(LPSTR)pch, (short)CchSz(pch), (LPINT)mpchdx);
	WAssert(fError,(LPSTR)pch);

}


DxOfSz(rgdx, pch, b)
WORD	*rgdx;
char	*pch;
int		b;
{
	int	i = 0;
	int it;
	char szt[ichMaxSz];


/*  This Doesn't work !  ? Why!	
		{	
		for (;*pch;pch++)
			GetCharWidth(vpri.hdc,(WORD) *pch,(WORD) *pch,(LPINT)(rgdx + i++));
		}
*/

	for (;*pch;pch++)
		*(rgdx + i++) = GetTextExtent(vpri.hdc,(LPSTR) pch, 1);

	for (i--; i > -1; i--)
		rgdx[i] += (int)MultDiv(b,vdud.xInch,vdud.PtsInch);

}


NewLine( hFont)
HFONT 	hFont;
{
	WAssert(vdud.yMaxPage, (LPSTR)"No Valid vdud.yMaxPage");

	vdud.yLoc += vdud.yLin;
	if ( (vdud.yLoc + vdud.yLin) > vdud.yMaxPage)
		NewPage(hFont);
}


NewPage(hFont)
HFONT	hFont;
{
	short	iError;

	iError = Escape(vpri.hdc,NEWFRAME,NULL,(LPSTR)NULL,(LPSTR)NULL);
	switch (iError)
		{
	case SP_APPABORT:
		break;
	case SP_ERROR:
		MessageBox(vhwnd,
				(LPSTR) "General Printer Error.",
				(LPSTR)	vszApp,
				MB_OK | MB_ICONEXCLAMATION);
		break;
	case SP_OUTOFDISK:
		MessageBox(vhwnd,
				(LPSTR) "Not Enough Disk Space to Print.",
				(LPSTR)	vszApp,
				MB_OK | MB_ICONEXCLAMATION);
		break;
	case SP_OUTOFMEMORY:
		MessageBox(vhwnd,
				(LPSTR) "Not Enough Memory to Print.",
				(LPSTR)	vszApp,
				MB_OK | MB_ICONEXCLAMATION);
		break;
	case SP_USERABORT:
		MessageBox(vhwnd,
				(LPSTR) "User Aborted Print Job.",
				(LPSTR)	vszApp,
				MB_OK | MB_ICONEXCLAMATION);
		break;
		}
	if (hFont)
		SelectObject(vpri.hdc, hFont);
	vdud.yLoc = vdud.yOff;
}


BOOL FAR PASCAL AbortProc(hdcPr, nCode)
HDC		hdcPr;
short	nCode;
{
	MSG	msg;

	while (!vfUserAbort && PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
		{
		if (!vhDlgPrint || !IsDialogMessage(vhDlgPrint, &msg))
			{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			}
		}
	return !vfUserAbort;
}


FEscSupport(idd)
int		idd;
{
	short	esc = vrgesc[idd-idd_EscMin].esc;
	BOOL	fTemp = fTrue;

	WAssert(vpri.hdc,(LPSTR)"No Valid DC");

	if (Escape(vpri.hdc, QUERYESCSUPPORT, 2, (LPSTR) &esc,(LPSTR) NULL))
		return fTrue;
	else
		{
		vrgesc[idd-idd_EscMin].iSt = iSt_NS;
		return fFalse;
		}
}


DisplayLogFont(lplf, nFontType)
LPLOGFONT	lplf;
short		nFontType;
{
	static LOGFONT lfD;
	static int	ft;

	static char *szYN [] = 
		{ 
		"No",         "Yes" 		};
	static char *szCS [] = 
		{ 
		"ANSI",       "?????",   "Kanji",    "OEM" 		};
	static char *szOP [] = 
		{ 
		"Default",    "String",  "Char",    "Stroke" 		};
	static char *szCP [] = 
		{ 
		"Default",    "Char",    "Stroke",   "?????" 		};
	static char *szQU [] = 
		{ 
		"Default",    "Draft", "Proof",    "?????" 		};
	static char *szP1 [] = 
		{ 
		"Default",    "Fixed",   "Variable", "?????" 		};
	static char *szFA [] = 
		{ 
		"Don't Care", "Roman",      "Swiss", "Modern",
		"Script",     "Decorative", "?????", "?????" 		};
	static char *szVR [] = 
		{ 
		"Vector",     "Raster" 		};
	static char *szGD [] = 
		{ 
		"GDI",        "Device" 		};

	static struct 
		{
		char  *szFmt ;
		short *pData ;
		} shorts [] = 
		{
		"LOGFONT",            NULL, 
		"-------",            NULL,
		"Height:      %10d",  &lfD.lfHeight,
		"Width:       %10d",  &lfD.lfWidth,
		"Escapement:  %10d",  &lfD.lfEscapement,
		"Orientation: %10d",  &lfD.lfOrientation,
		"Weight:      %10d",  &lfD.lfWeight,
		};

	static struct 
		{
		char  *szFmt ;
		BYTE  *pData ;
		char  **szArray ;
		short sAnd ;
		short sShift ;
		}  strings [] = 
		{
		"Italic:      %10s",  &lfD.lfItalic,         szYN, 1,    0,
		"Underline:   %10s",  &lfD.lfUnderline,      szYN, 1,    0,
		"Strike-Out:  %10s",  &lfD.lfStrikeOut,      szYN, 1,    0,
		"Char Set:    %10s",  &lfD.lfCharSet,        szCS, 0xC0, 6,
		"Out  Prec:   %10s",  &lfD.lfOutPrecision,   szOP, 3,    0,
		"Clip Prec:   %10s",  &lfD.lfClipPrecision,  szCP, 3,    0, 
		"Quality:     %10s",  &lfD.lfQuality,        szQU, 3,    0,
		"Pitch:       %10s",  &lfD.lfPitchAndFamily, szP1, 3,    0,
		"Family:      %10s",  &lfD.lfPitchAndFamily, szFA, 0x70, 4,
		"Font Type:  %6s",    (BYTE *) &ft,          szVR, 1,    0,
		"%s",                 (BYTE *) &ft,  	     szGD, 2,    1
		};

	char 	szBuffer [80] ;
	int  	i ;
	TEXTMETRIC  tm ;

	GetTextMetrics(vpri.hdc, &tm);
	vdud.yLin = tm.tmHeight + tm.tmExternalLeading ;

	lfD = *lplf;
	ft = nFontType;

	for (i = 0 ; i < sizeof shorts / sizeof shorts [0] ; i++)
		{
		Newline(NULL);
		TextOut (vpri.hdc, 5, vdud.yLoc, szBuffer,
				sprintf (szBuffer, shorts[i].szFmt, *shorts[i].pData)) ;
		}

	for (i = 0 ; i < sizeof strings / sizeof strings [0] ; i++)
		{
		Newline(NULL);
		TextOut (vpri.hdc, 5, vdud.yLoc, szBuffer,
				sprintf (szBuffer, strings[i].szFmt, (strings[i].szArray)
				[(*strings[i].pData & strings[i].sAnd) >>
												strings[i].sShift])) ;
		}
	Newline(NULL);
	TextOut (vpri.hdc, 5, vdud.yLoc, szBuffer,
			sprintf (szBuffer, "Face Name:   %10s", lfD.lfFaceName)) ;

	Newline(NULL);
}


DisplayTextMetric (lptm)
LPTEXTMETRIC	lptm;
{
	static TEXTMETRIC	tmD;

	static char *szYN [] = 
		{ 
		"No",         "Yes" 		};
	static char *szCS [] = 
		{ 
		"ANSI",       "?????",   "Kanji",    "OEM" 		};
	static char *szP2 [] = 
		{ 
		"Fixed",      "Variable" 		};
	static char *szFA [] = 
		{ 
		"Don't Care", "Roman",      "Swiss", "Modern",
		"Script",     "Decorative", "?????", "?????" 		};

	static struct 
		{
		char  *szFmt ;
		short *pData ;
		} shorts [] = 
		{
		"TEXTMETRIC",         NULL,
		"----------",         NULL,
		"Height:       %5d",  &tmD.tmHeight,
		"Ascent:       %5d",  &tmD.tmAscent,
		"Descent:      %5d",  &tmD.tmDescent,
		"Int. Leading: %5d",  &tmD.tmInternalLeading,
		"Ext. Leading: %5d",  &tmD.tmExternalLeading,
		"Ave. Width:   %5d",  &tmD.tmAveCharWidth,
		"Max. Width:   %5d",  &tmD.tmMaxCharWidth,
		"Weight:       %5d",  &tmD.tmWeight,
		"Overhang:     %10d", &tmD.tmOverhang,
		"Digitized X:  %10d", &tmD.tmDigitizedAspectX,
		"Digitized Y:  %10d", &tmD.tmDigitizedAspectY
		};

	static struct 
		{
		char  *szFmt ;
		BYTE  *pData ;
		} bytes [] = 
		{
		"First Char:   %10d", &tmD.tmFirstChar,
		"Last Char:    %10d", &tmD.tmLastChar,
		"Default Char: %10d", &tmD.tmDefaultChar,
		"Break Char:   %10d", &tmD.tmBreakChar
		};

	static struct 
		{
		char  *szFmt ;
		BYTE  *pData ;
		char  **szArray ;
		short sAnd ;
		short sShift ;
		} strings [] = 
		{
		"Italic:       %5s",  &tmD.tmItalic,         szYN, 1,    0,
		"Underline:    %5s",  &tmD.tmUnderlined,     szYN, 1,    0,
		"Strike-Out:   %5s",  &tmD.tmStruckOut,      szYN, 1,    0,
		"Pitch:        %10s", &tmD.tmPitchAndFamily, szP2, 1,    0,
		"Family:       %10s", &tmD.tmPitchAndFamily, szFA, 0x70, 4,
		"Char Set:     %10s", &tmD.tmCharSet,        szCS, 0xC0, 6,
		};

	char szBuffer [80];
	int  i;

	tmD = *lptm;

	vdud.yLin = tmD.tmHeight + tmD.tmExternalLeading ;

	for (i = 0 ; i < sizeof shorts / sizeof shorts [0] ; i++)
		{
		Newline(NULL);
		TextOut (vpri.hdc, 5, vdud.yLoc, szBuffer,
				sprintf (szBuffer, shorts[i].szFmt, *shorts[i].pData)) ;
		}

	for (i = 0 ; i < sizeof bytes / sizeof bytes [0] ; i++)
		{
		Newline(NULL);
		TextOut (vpri.hdc, 5, vdud.yLoc, szBuffer,
				sprintf (szBuffer, bytes[i].szFmt, *bytes[i].pData)) ;
		}

	for (i = 0 ; i < sizeof strings / sizeof strings [0] ; i++)
		{
		Newline(NULL);
		TextOut (vpri.hdc, 5, vdud.yLoc, szBuffer,
				sprintf (szBuffer, strings[i].szFmt, (strings[i].szArray)
				[(*strings[i].pData & strings[i].sAnd) >>
												strings[i].sShift])) ;
		}

	Newline(NULL);
}


WriteChar(ch1, pxduLoc, xduCol, hFont)
BYTE	ch1;
short	*pxduLoc, xduCol;
HFONT	hFont;
{
	char	szLine[ichMaxSz];
	char 	*pch;
	int		cch;

	SzFromInt((int)ch1, szLine, &cch);

	SetDlgItemText(vhDlgPrint, idd_pdt3, (LPSTR)szLine);

	SzSzAppend(szLine, " :: ");

	for (pch = szLine; *pch; pch++);
	*pch++ = ch1;
	*pch = 0;

	TextOut(vpri.hdc, *pxduLoc, vdud.yLoc, (LPSTR) szLine, CchSz(szLine));

	vdud.yLoc += vdud.yLin;
	if ( (vdud.yLoc + vdud.yLin) > vdud.yMaxPage)
		{
		*pxduLoc += xduCol;
		vdud.yLoc = vdud.yOff;
		}
}


SetAbortProc(szTitle, szHead, szTail)
char	szTitle[],
szHead[],
szTail[];
{
	EnableWindow(vhwnd, fFalse);
	vfUserAbort = fFalse;
	lpfnPrintDlg = MakeProcInstance(PrintDlg, vhInst);
	vhDlgPrint = CreateDialog(vhInst, "PrintDlgBox", vhwnd, lpfnPrintDlg);
	SetDlgItemText(vhDlgPrint, idd_pdt1, (LPSTR) szTitle);
	SetDlgItemText(vhDlgPrint, idd_pdt2, (LPSTR) szHead);
	SetDlgItemText(vhDlgPrint, idd_pdt3, (LPSTR) szTail);

	lpfnAbort = MakeProcInstance(AbortProc, vhInst);
	Escape( vpri.hdc, SETABORTPROC, 0, (LPSTR) lpfnAbort, NULL);
}


SetAbortProcNum(i)
int	i;
{
	char	sz[ichMaxSz];
	int	cch;

	SzFromInt(i, sz, &cch);
	SetDlgItemText(vhDlgPrint, idd_pdt3, (LPSTR)sz);
}


UnsetAbortProc()
{
	if (!vfUserAbort)
		{
		EnableWindow(vhwnd, fTrue);
		DestroyWindow(vhDlgPrint);
		}
	FreeProcInstance(lpfnPrintDlg);
	FreeProcInstance(lpfnAbort);
}


ExecFontDlg(fOptions)
BOOL	fOptions;
{
	FARPROC		lpfnFontDlg;

	/* get printer dc. */
	if (!vpri.hdc)
		if (!HDCPrinterFromProfile())
			{
			ReportSz("Error getting printer dc");
			return;
			}

	lpfnFontDlg = MakeProcInstance(FontDlg, vhInst);
	if (DialogBox(vhInst, "FontDlg", vhwnd, lpfnFontDlg))
		{
		FreeProcInstance(lpfnFontDlg);
		if (fOptions)
			CmdOptions();
		return fTrue;
		}

	FreeProcInstance(lpfnFontDlg);
	return fFalse;
}


LoadUFD(hDlg, bCharSet, bQuality, bPitch)
HWND	hDlg;
BYTE	bCharSet,
bQuality,
bPitch;
{
	BOOL	bool;

	/* if use as Default is check then we have a font for actions such
		as escape testing, were the default is normally used.  This does
		not apply to show font and test printing! */
	vufd.fHaveFont = IsDlgButtonChecked(hDlg, idd_DEFF);

	/* Height -- device units */
	vufd.lf.lfHeight = -(int) MultDiv(
			GetDlgItemInt(hDlg,idd_FHP,(BOOL FAR *)&bool,fFalse),
			vdud.yInch, vdud.PtsInch);

	/* Width  -- device units */
	vufd.lf.lfWidth = (int) MultDiv(
			GetDlgItemInt(hDlg,idd_FWP,(BOOL FAR *)&bool,fFalse),
			vdud.xInch, vdud.PtsInch );


	vufd.lf.lfWeight = IsDlgButtonChecked(hDlg,idd_Bold) ?
			FW_BOLD : FW_NORMAL;
	vufd.lf.lfItalic = IsDlgButtonChecked(hDlg,idd_Ital);
	vufd.lf.lfUnderline = IsDlgButtonChecked(hDlg,idd_Unde);
	vufd.lf.lfStrikeOut = IsDlgButtonChecked(hDlg,idd_Stri);

	vufd.lf.lfCharSet = bCharSet;
	vufd.lf.lfQuality = bQuality;
	vufd.lf.lfPitchAndFamily = bPitch;

	/* FaceName */
	GetDlgItemText(hDlg, idd_FN,
			(LPSTR)(vufd.lf.lfFaceName), 32);

}


/* O u r  G e t  C h a r  W i d t h */
int OurGetCharWidth( hdc, chFirst )
HDC hdc;
int chFirst;
{
	int retVal;
	Assert( hdc != NULL );
	Assert( chFirst >= 0 && chLast <= 255 );

	retVal = GetTextExtent( hdc, &chFirst, 1 );

	return (retVal);
}


SetBytes(pb, w, cb)
char *pb;
int w, cb;
{
	int i;

	for (i = 0; i < cb ; i++)
		*pb++ = w;


}


