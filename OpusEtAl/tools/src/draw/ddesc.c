#include <windows.h>
#include "ddall.h"
#include "dddlg.h"

extern  BOOL FAR PASCAL AbortProc();
extern  BOOL FAR PASCAL PrintDlg();

extern  FARPROC	lpfnAbort, lpfnPrintDlg;
extern	HWND	vhwnd,
vhDlgPrint;
extern	HANDLE	vhInst;
extern	char	szApp[];
extern  BOOL	vfOnePage,
vfUserAbort;
extern  DUD	vdud;
extern	PRI	vpri;
extern	UFD	vufd;
extern  ESC     vrgesc[];

ExecEscTests(idd)
int	idd;
{
	TEXTMETRIC	tm;
	HFONT	hFont = NULL,
			hFontT;


	EnableWindow(vhwnd, fFalse);
	vfUserAbort = fFalse;
	lpfnPrintDlg = MakeProcInstance(PrintDlg, vhInst);
	vhDlgPrint = CreateDialog(vhInst, "PrintDlgBox", vhwnd, lpfnPrintDlg);
	SetDlgItemText(vhDlgPrint, idd_pdt1, (LPSTR)"Testing Escapes.");
	SetDlgItemText(vhDlgPrint, idd_pdt2, (LPSTR)"Escape Code ");
	SetDlgItemText(vhDlgPrint, idd_pdt3, (LPSTR)"None");

	lpfnAbort = MakeProcInstance(AbortProc, vhInst);
	Escape( vpri.hdc, SETABORTPROC, 0, (LPSTR) lpfnAbort, NULL);

	Escape(vpri.hdc,STARTDOC,15,(LPSTR) "ESCTEST",(LPSTR) NULL);

	if (vufd.fHaveFont)
		{
		hFont = CreateFontIndirect(&(vufd.lf));
		hFontT = SelectObject(vpri.hdc, hFont);
		}

	GetTextMetrics(vpri.hdc,(LPTEXTMETRIC)&tm);
	vdud.yLin = tm.tmHeight + tm.tmExternalLeading;

	switch (idd)
		{
	case idd_ALL:
		for (idd=idd_EscMin; idd < idd_EscMax; idd++)
			if (vrgesc[idd-idd_EscMin].iSt != iSt_NS)
				{
				SwitchEscTest(idd, hFont);
				if (vfOnePage)
					NewPage(hFont);
				}
		break;

	case idd_EscMin:
		for (idd=idd_EscMin; idd < idd_EscMax; idd++)
			if (vrgesc[idd-idd_EscMin].iSt == iSt_ON)
				{
				SwitchEscTest(idd, hFont);
				if (vfOnePage)
					NewPage(hFont);
				}
		break;

	default:
		break;
		}

	if (!vfOnePage)
		NewPage(hFont);
	Escape(vpri.hdc,ENDDOC,NULL,(LPSTR)NULL,(LPSTR)NULL);

	if (vufd.fHaveFont)
		{
		SelectObject(vpri.hdc, hFontT);
		DeleteObject(hFont);
		}

	DeleteDC(vpri.hdc);

	if (!vfUserAbort)
		{
		EnableWindow(vhwnd, fTrue);
		DestroyWindow(vhDlgPrint);
		}
	FreeProcInstance(lpfnPrintDlg);
	FreeProcInstance(lpfnAbort);
}


SwitchEscTest( idd, hFont)
int	idd;
HFONT	hFont;
{
	switch (idd)
		{
	case idd_Esc1:
		TestBANDINFO(hFont);
		break;

	case idd_Esc2:
		TestGETPHYSPAGESIZE(hFont);
		break;

	case idd_Esc3:
		TestGETPRINTINGOFFSET(hFont);
		break;

	case idd_Esc4:
		TestGETSCALINGFACTOR(hFont);
		break;

	case idd_Esc5:
		TestDRAFTMODE(hFont);
		break;

	case idd_Esc6:
		TestGETEXTENDEDTEXTMETRICS(hFont);
		break;

	case idd_Esc7:
		TestGETEXTENTTABLE(hFont);
		break;

	case idd_Esc8:
		TestGETSETPAPERBINS(hFont);
		break;

	case idd_Esc9:
		WAssert(fFalse, (LPSTR)"Escape Test 9");
		break;

	case idd_Esc0:
		WAssert(fFalse, (LPSTR)"Escape Test 0");
		break;

	case idd_Esca:
		WAssert(fFalse, (LPSTR)"Escape Test a");
		break;

	case idd_Escb:
		WAssert(fFalse, (LPSTR)"Escape Test b");
		break;

	case idd_Escc:
		WAssert(fFalse, (LPSTR)"Escape Test c");
		break;

	case idd_Escd:
		WAssert(fFalse, (LPSTR)"Escape Test d");
		break;

	case idd_Esce:
		WAssert(fFalse, (LPSTR)"Escape Test e");
		break;

	case idd_Escf:
		WAssert(fFalse, (LPSTR)"Escape Test f");
		break;

	case idd_Escg:
		WAssert(fFalse, (LPSTR)"Escape Test g");
		break;

	case idd_Esch:
		WAssert(fFalse, (LPSTR)"Escape Test h");
		break;

	case idd_Esci:
		WAssert(fFalse, (LPSTR)"Escape Test i");
		break;

	case idd_Escj:
		WAssert(fFalse, (LPSTR)"Escape Test j");
		break;

	default:
		WAssert(fFalse, (LPSTR) "Invalid Esc Test (ExecEscTest)");
		return fFalse;
		}
}


TestBANDINFO(hFont)
HFONT	hFont;
{
	BOOL 	fSuccess = fTrue;
	char	szLine[ichMaxSz], szT[ichMaxSz];
	short	c;

	struct	BIS 
		{
		BOOL	fGraphFlag;
		BOOL	fTextFlag;
		RECT	GraphicsRect;
		}	 bis;

	SetDlgItemText(vhDlgPrint, idd_pdt3, (LPSTR)"BANDINFO");

	if (Escape(vpri.hdc, BANDINFO, 0, (LPSTR)&bis, (LPSTR)NULL))
		{
		szLine[0] = '\0';
		SzSzAppend(szLine, "Graphics: ");
		SzBoolAppend(szLine, bis.fGraphFlag);

		SzSzAppend(szLine, "  Text: ");
		SzBoolAppend(szLine, bis.fTextFlag);

		TextOut(vpri.hdc, 0, vdud.yLoc, (LPSTR)szLine, CchSz(szLine));
		NewLine(hFont);

		szLine[0] = '\0';
		SzSzAppend(szLine, "Rect Top: ");
		SzFromInt(bis.GraphicsRect.top, szT, &c);
		SzSzAppend(szLine, szT);

		SzSzAppend(szLine, "  Left: ");
		SzFromInt(bis.GraphicsRect.left, szT, &c);
		SzSzAppend(szLine, szT);

		SzSzAppend(szLine, "  Bottom: ");
		SzFromInt(bis.GraphicsRect.bottom, szT, &c);
		SzSzAppend(szLine, szT);

		SzSzAppend(szLine, "  Right: ");
		SzFromInt(bis.GraphicsRect.right, szT, &c);
		SzSzAppend(szLine, szT);

		TextOut(vpri.hdc, 0, vdud.yLoc, (LPSTR)szLine, CchSz(szLine));
		NewLine(hFont);
		}
	else
		{
		TextOut(vpri.hdc, 0, vdud.yLoc, (LPSTR)"Escape failed!", 14);
		NewLine(hFont);
		}
}


TestGETPHYSPAGESIZE(hFont)
HFONT	hFont;
{
	BOOL 		fSuccess = fTrue;
	char		szLine[ichMaxSz],
	szT[ichMaxSz];
	short		c;
	TEXTMETRIC	tm;
	POINT		pt;

	SetDlgItemText(vhDlgPrint, idd_pdt3, (LPSTR)"GETPHYSPAGESIZE");

	TextOut(vpri.hdc, 0, vdud.yLoc, (LPSTR)"GETPHYSPAGESIZE escape", 22);
	NewLine(hFont);

	if (Escape(vpri.hdc, GETPHYSPAGESIZE, 0, (LPSTR)NULL, (LPSTR)&pt) > 0)
		{
		szLine[0] = '\0';
		SzSzAppend(szLine, "Size X: ");
		SzFromInt(pt.x, szT, &c);
		SzSzAppend(szLine, szT);

		SzSzAppend(szLine, "   Y: ");
		SzFromInt(pt.y, szT, &c);
		SzSzAppend(szLine, szT);

		TextOut(vpri.hdc, 0, vdud.yLoc, (LPSTR)szLine, CchSz(szLine));
		NewLine(hFont);

		GetTextMetrics(vpri.hdc, (LPTEXTMETRIC) &tm);

		NewPage(hFont);

		TextOut(vpri.hdc, 0, 0, (LPSTR) "1 2 3 4 5 6 7 8 9 0", 19);
		TextOut(vpri.hdc, pt.x - (17*tm.tmAveCharWidth), 0,
				(LPSTR) "1 2 3 4 5 6 7 8 9 0", 19);
		TextOut(vpri.hdc, 0, pt.y,
				(LPSTR) "1 2 3 4 5 6 7 8 9 0", 19);
		TextOut(vpri.hdc, pt.x - (17*tm.tmAveCharWidth), pt.y,
				(LPSTR) "1 2 3 4 5 6 7 8 9 0", 19);


		}
	else
		{
		TextOut(vpri.hdc, 0, vdud.yLoc, (LPSTR)"Escape failed!", 14);
		NewLine(hFont);
		}
	NewLine(hFont);

}


TestGETPRINTINGOFFSET(hFont)
HFONT	hFont;
{
	BOOL 	fSuccess = fTrue;
	char	szLine[ichMaxSz], szT[ichMaxSz];
	short	c;

	POINT	pt;

	SetDlgItemText(vhDlgPrint, idd_pdt3, (LPSTR)"GETPRINTINGOFFSET");

	TextOut(vpri.hdc, 0, vdud.yLoc, (LPSTR)"GETPRINTINGOFFSET escape", 24);
	NewLine(hFont);

	if (Escape(vpri.hdc, GETPRINTINGOFFSET, 0, (LPSTR)NULL, (LPSTR)&pt) > 0)
		{
		szLine[0] = '\0';
		SzSzAppend(szLine, "Offset X: ");
		SzFromInt(pt.x, szT, &c);
		SzSzAppend(szLine, szT);

		SzSzAppend(szLine, "   Y: ");
		SzFromInt(pt.y, szT, &c);
		SzSzAppend(szLine, szT);

		TextOut(vpri.hdc, 0, vdud.yLoc, (LPSTR)szLine, CchSz(szLine));
		NewLine(hFont);
		}
	else
		{
		TextOut(vpri.hdc, 0, vdud.yLoc, (LPSTR)"Escape failed!", 14);
		NewLine(hFont);
		}
	NewLine(hFont);
}


TestGETSCALINGFACTOR(hFont)
HFONT	hFont;
{
	BOOL 	fSuccess = fTrue;
	char	szLine[ichMaxSz], szT[ichMaxSz];
	short	c;

	POINT	pt;

	SetDlgItemText(vhDlgPrint, idd_pdt3, (LPSTR)"GETSCALINGFACTOR");

	TextOut(vpri.hdc, 0, vdud.yLoc, (LPSTR)"GETSCALINGFACTOR escape", 23);
	NewLine(hFont);

	if (Escape(vpri.hdc, GETSCALINGFACTOR, 0, (LPSTR)NULL, (LPSTR)&pt) > 0)
		{
		szLine[0] = '\0';
		SzSzAppend(szLine, "Scaling X: ");
		SzFromInt(pt.x, szT, &c);
		SzSzAppend(szLine, szT);

		SzSzAppend(szLine, "   Y: ");
		SzFromInt(pt.y, szT, &c);
		SzSzAppend(szLine, szT);

		TextOut(vpri.hdc, 0, vdud.yLoc, (LPSTR)szLine, CchSz(szLine));
		NewLine(hFont);
		}
	else
		{
		TextOut(vpri.hdc, 0, vdud.yLoc, (LPSTR)"Escape failed!", 14);
		NewLine(hFont);
		}
	NewLine(hFont);
}


TestDRAFTMODE(hFont)
HFONT	hFont;
{
	int	iMode;

	SetDlgItemText(vhDlgPrint, idd_pdt3, (LPSTR)"DRAFTMODE");


	if (!vfOnePage)
		NewPage(hFont);

	/* NOTE: Draftmode escape must be first thing on a new page so we have
		to wait to print the banner line till after we have tried the escape
	*/

	iMode = 1;
	if (Escape(vpri.hdc, DRAFTMODE, 2, (LPSTR) &iMode, (LPSTR)NULL) > 0)
		{
		TextOut(vpri.hdc, 0, vdud.yLoc, (LPSTR)"DRAFTMODE escape", 16);
		NewLine(hFont);
		WAssert(TextOut(vpri.hdc, 0, vdud.yLoc, (LPSTR)"This is in draft mode", 21),
				(LPINT)"TextOut");
		NewLine(hFont);

		NewPage(hFont);
		iMode = 0;
		Escape(vpri.hdc, DRAFTMODE, 2, (LPSTR) &iMode, (LPSTR)NULL);
		TextOut(vpri.hdc, 0, vdud.yLoc, (LPSTR)"DRAFTMODE escape", 16);
		NewLine(hFont);
		TextOut(vpri.hdc, 0, vdud.yLoc, (LPSTR)"This is not in draft mode", 25);
		NewLine(hFont);
		}
	else
		{
		TextOut(vpri.hdc, 0, vdud.yLoc, (LPSTR)"DRAFTMODE escape", 16);
		NewLine(hFont);
		TextOut(vpri.hdc, 0, vdud.yLoc, (LPSTR)"Escape failed!", 14);
		NewLine(hFont);
		}
	NewLine(hFont);
}


TestGETEXTENDEDTEXTMETRICS(hFont)
HFONT	hFont;
{
	SetDlgItemText(vhDlgPrint, idd_pdt3, (LPSTR)"GETEXTENDEDTEXTMETRICS");

	TextOut(vpri.hdc, 0, vdud.yLoc, (LPSTR)"GETEXTENDEDTEXTMETRICS escape", 29);
	NewLine(hFont);
}


TestGETEXTENTTABLE(hFont)
HFONT	hFont;
{
}


TestGETSETPAPERBINS(hFont)
HFONT	hFont;
{
}


