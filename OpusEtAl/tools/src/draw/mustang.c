/*
	Mustang	 -- Win App to test various windows activities to be used
			by WinWord.

	Danp

	Revisions: 1. Copy and Paste; Edit Menu.
			2. Resource menu; bitmaps, cursors and icons.
			3. Print Menu; Enumerate fonts.
			4.             Escape code testing.
			5.             Show font.
			6.             Test printing.
			7.             Display LogFont; better LogFont Selection
			8. Clean-up: better hungarian, code review. (31-Oct-88)
*/

/*  mustang.c --  Windows Initialization procs.
			Utility Procs.
			Global Variables.
*/

#define OEMRESOURCE /* for OBM_CLOSE */
#include <windows.h>
#include "ddall.h"
#include "resource.h"
#include "dddlg.h"

long FAR PASCAL MainWndProc(HWND, unsigned, WORD, LONG);
BOOL FAR PASCAL About();
BOOL FAR PASCAL MyMessage();

extern BOOL FAR PASCAL OptionsDlg();
extern BOOL FAR PASCAL FontDlg();
extern HDC  HDCPrinterFromProfile();


HWND 	vhwnd = NULL,	     		/* App. Window Handle */
vhDlgPrint;			/* Print Dialog Handle */
HANDLE  vhInst;				/* App. Instance */
FARPROC	lpfnAbort, lpfnPrintDlg;	/* Used for Printer Abort Proc */

BOOL 	vfLOption = fFalse,		/* T = started by another app */
vfOnePage = fFalse,		/* T = print one typeface per page*/
vfUserAbort = fFalse,		/* T = user has aborted print job */
vfLogFont = fFalse,		/* T = print the logfont structure */
vfTextMetric = fFalse,		/* T = print the textmetric struct */
vfSpecial = fFalse;
nUsePeekMessage = 0;

char 	vszApp [] = "Mustang",		/* Our Name */
vszOrigApp [ichMaxSz];		/* App that started us */

/* current measurements on device page. */
DUD	vdud = {
	0, 0, 0, 0, 0, 0, POINTS_INCH};


/* printer structure */
PRI	vpri = {
	NULL, 0, 0, 0};


/* Logical font set in Font Dlg */
UFD	vufd	= { 
	fFalse,
				{ 
		0, 0, 0, 0, 400, 0, 0, 0, 0, 0, 0, 0, 0, "Default"		}
};


/* Escape Codes, current selections */
ESC	vrgESC[idd_EscMax-idd_EscMin] = { 
	{ BANDINFO, 0 },
	{ GETPHYSPAGESIZE, 0 },
	{ GETPRINTINGOFFSET, 0 },
	{ GETSCALINGFACTOR, 0 },
	{ DRAFTMODE, 0 },
	{ GETEXTENDEDTEXTMETRICS, 0 },
	{ GETEXTENTTABLE, 0 },
	{ GETSETPAPERBINS, 0 }
	};					


/* Initialization items -- used in this module */
HMENU 	hMenuBar;
HCURSOR hCursor = NULL;
HBRUSH 	hbrBlack, hbrWhite, hbrBack;



/*  W I N   M A I N  */
/*	Windows main function.  Initializes main window and does forever loop
	processing.	 */
int PASCAL WinMain( hInstance, hPrevInstance, lpszCmdLine, cmdShow )
HANDLE hInstance, hPrevInstance;
LPSTR  lpszCmdLine;
int    cmdShow;
{
	MSG   msg;

	/* Create the App Window */
	if (!FInitApp(hInstance, hPrevInstance, lpszCmdLine, cmdShow))
		return fFalse;

	/* Get any command line arguments */
	ParseCmdLine(lpszCmdLine);

	/* Someone started us with info. in the clipboard.  Append their
		Name and get the clipboard info */
	if (vfLOption)
		{
		char szAppNew[ichMaxSz];
		szAppNew[0] = 0;
		SzSzAppend(szAppNew, vszApp);
		SzSzAppend(szAppNew, ": ");
		SzSzAppend(szAppNew, vszOrigApp);
		SetWindowText(vhwnd, (LPSTR) szAppNew);
		/* get clipboard contents */
		CmdPaste();
		}

	/* forever loop -- this is it! */
	for (;;)
		{
		if (nUsePeekMessage != 0)
			{
			while (!PeekMessage((LPMSG)&msg, NULL, NULL, NULL, PM_REMOVE ))
				{
				if (nUsePeekMessage == 2)
					Yield();
				}
			}
		else  if (!GetMessage((LPMSG)&msg, NULL, 0, 0))
			break;

		TranslateMessage((LPMSG)&msg);
		DispatchMessage((LPMSG)&msg);

		}

	ExitApp(0);
}


/*  P A R S E   C M D   L I N E  */
/*  Parse the command line and set global switches to match command flags. */
ParseCmdLine( lpszCmdLine)
LPSTR  lpszCmdLine;
{
	LPSTR  lpch = lpszCmdLine;
	char   *pch = vszOrigApp;

	while ( *lpch != '\0' )
		{
		if (*lpch == '/')
			{
			lpch++;
		/* switch on command arg.  Only case so far 'L' */
			switch (*lpch)
				{
			case 'l':
			case 'L':
				/* skip past the space and first " */
				lpch += 3;
				vfLOption = fTrue;
				while ((*lpch != '"') && (*lpch != '\0') &&
						(*lpch != '/'))
					*pch++ = *lpch++;
				*pch = '\0';
				break;
			case 's' :
			case 'S' :
				vfSpecial = fTrue;
				break;
				}
			}
		(*lpch != '\0') ? lpch++ : 0;
		}
}


/*  Q U I T   A P P  */
/*  cleans up in preparation for exit from the app.  */
QuitApp()
{
	int	id;

	/* if we are closing but were opened by another app and have not
		pasted our info yet, then we ask if we should do it now. */
	if  (vfLOption)
		{
		id = MessageBox( vhwnd,
				(LPSTR)  "Save changes ?",
				(LPSTR)  vszOrigApp,
				MB_ICONQUESTION | MB_YESNOCANCEL );
		if (id == IDYES)
			{
			/* We need to copy our info and render it since we
				are closing */
			CmdCopy();
			RenderMetaFile();
			}
		else  if (id == IDCANCEL)
			return;		/* Cancel-we don't quit after all! */
		}

	/* write to win.ini so next time we come up looking the same */
	WriteProfileString((LPSTR) vszApp, (LPSTR) "AppMaximize",
			(LPSTR) (IsZoomed(vhwnd) ? "1" : "0"));

	PostQuitMessage(0);
}



/*  E X I T   A P P  */
/*  This is it, the end!  DOES NOT RETURN  */
ExitApp(ec)
WORD ec;
{
	/* REVIEW: clean up */
	exit(ec);
}


/*  F   I N I T   A P P  */
/*  Create the main window.  Set up menus.  */
FInitApp(hInstance, hPrevInstance, lpszCmdLine, cmdShow)
HANDLE hInstance, hPrevInstance;
LPSTR lpszCmdLine;
int cmdShow;
{
	HMENU hMenu;
	HWND  hWnd;
	BOOL  fMaxiApp;

	vhInst = hInstance;

	fMaxiApp = GetProfileInt((LPSTR) vszApp, 
			(LPSTR) "AppMaximize", fFalse);

	if (!hPrevInstance)
		if (!FInitInst( hInstance ))
			return FALSE;

	if (!FInitCommands())
		return FALSE;

	vhwnd = hWnd = CreateWindow((LPSTR) "MSWINDRAW",
			(LPSTR) vszApp,
			WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_VISIBLE,
			CW_USEDEFAULT,
			(fMaxiApp && cmdShow == SW_SHOW) ? SW_SHOWMAXIMIZED : cmdShow,
			CW_USEDEFAULT,
			0,    /* cy - ignored for tiled windows */
	(HWND)NULL,      /* parent */
			(HMENU)hMenuBar,  /* use class menu */
			(HANDLE)vhInst, /* handle to window instance */
			(LPSTR)NULL        /* no params to pass on */
			);

	hMenu = GetSystemMenu(hWnd, fFalse);
	ChangeMenu(hMenu, 0, NULL, 999, MF_APPEND | MF_SEPARATOR);
	ChangeMenu(hMenu, 0, "&About...", 1, MF_APPEND | MF_STRING);

	hCursor = LoadCursor( hInstance, (LPSTR)"MustangCUR" );

	hbrWhite = GetStockObject(WHITE_BRUSH);
	hbrBlack = GetStockObject(BLACK_BRUSH);
	hbrBack	 = CreateSolidBrush(GetSysColor(COLOR_BACKGROUND));

	ShowWindow(hWnd, cmdShow);
	UpdateWindow(hWnd);

	return fTrue;
}


/*  F   I N I T   I N S T  */
/*  Register the window class.  Only called on first instance. */
int FInitInst( hInstance )
HANDLE hInstance;
{
	PWNDCLASS   pClass;

	pClass = (PWNDCLASS)LocalAlloc( LPTR, sizeof(WNDCLASS) );

	pClass->hCursor        = NULL;
	pClass->hIcon          = LoadIcon( hInstance, (LPSTR)"Mustang" );
	pClass->lpszMenuName   = (HMENU) NULL;
	pClass->lpszClassName  = (LPSTR)"MSWINDRAW";
	pClass->hbrBackground  = GetStockObject (WHITE_BRUSH);
	pClass->hInstance      = hInstance;
	pClass->style          = 0;
	pClass->lpfnWndProc    = MainWndProc;

	if (!RegisterClass((LPWNDCLASS)pClass))
		return fFalse;   /* Initialization failed */

	LocalFree((HANDLE) pClass);

	return fTrue;    /* Initialization succeeded */
}


/*  F   I N I T   C O M M A N D S  */
/*  Create menu bars.  */
FInitCommands()
{
	HMENU hMenu;

	hMenuBar = CreateMenu();

	hMenu = CreateMenu();
	ChangeMenu(hMenuBar, 0, "&File", hMenu, MF_APPEND | MF_POPUP);
	ChangeMenu(hMenu, 0, "Use &PeekMessage", cmSwMsgMode, MF_APPEND);
	ChangeMenu(hMenu, 0, "E&xit...", cmFileExit, MF_APPEND);


	hMenu = CreateMenu();
	ChangeMenu(hMenuBar, 0, "&Edit", hMenu, MF_APPEND | MF_POPUP);
	ChangeMenu(hMenu, 0, "&Copy\tCtrl+Ins", cmCopy, MF_APPEND);
	ChangeMenu(hMenu, 0, "&Paste\tShift+Ins", cmPaste, MF_APPEND);
	ChangeMenu(hMenu, 0, "C&lear\tDel", cmClear, MF_APPEND);

	hMenu = CreateMenu();
	ChangeMenu(hMenuBar, 0, "&Resources", hMenu, MF_APPEND | MF_POPUP);
	ChangeMenu(hMenu, 0, "Next &Bitmap", cmBitmap, MF_APPEND);
	ChangeMenu(hMenu, 0, "Next &Cursor", cmCursor, MF_APPEND);
	ChangeMenu(hMenu, 0, "Next &Icon", cmIcon, MF_APPEND);

	hMenu = CreateMenu();
	ChangeMenu(hMenuBar, 0, "&Printers", hMenu, MF_APPEND | MF_POPUP);
	ChangeMenu(hMenu, 0, "&Printer Setup", cmPrSetup, MF_APPEND);
	ChangeMenu(hMenu, 0, "&Options...", cmOptions, MF_APPEND);
	ChangeMenu(hMenu, 0, NULL, 0, MF_SEPARATOR | MF_APPEND);
	ChangeMenu(hMenu, 0, "Enumerate &Fonts", cmEnum, MF_APPEND);
	ChangeMenu(hMenu, 0, "&Show Font", cmFont, MF_APPEND);
	ChangeMenu(hMenu, 0, "&Escapes Codes...", cmEscape, MF_APPEND);
	ChangeMenu(hMenu, 0, NULL, 0, MF_SEPARATOR | MF_APPEND);
	ChangeMenu(hMenu, 0, "&Test Print...", cmTestPr, MF_APPEND);
	ChangeMenu(hMenu, 0, "&User Function", cmUserPr, MF_APPEND);

	ChangeMenu(hMenuBar, 0, "\010&Help", 7, MF_APPEND);

	return fTrue;
}


/*  M A I N   W N D   P R O C  */
/*  Procedure which makes up the main window class.   
*/
long FAR PASCAL MainWndProc(hWnd, message, wParam, lParam)
HWND hWnd;
unsigned message;
WORD wParam;
LONG lParam;
{
	PAINTSTRUCT  ps;
	HDC hDC;
	FARPROC lpprocAbout;

	switch (message)
		{
	case WM_ACTIVATEAPP:
		/* if we lose the focus and LOption is on we have to
			copy our data to the clipboard.  Note: do not
			actually render the data at this time. */
		if ((wParam == 0) && (vfLOption))
			CmdCopy();
		break;

		/* Now we have to render the data */
	case WM_RENDERFORMAT:
		RenderMetaFile();
		if (vfLOption)
			{
			/* so we don't ask to save changes later */
			vfLOption = fFalse;
			QuitApp();
			}
		break;

		/* we are being destroyed and the clipboard needs our data */
	case WM_RENDERALLFORMATS:
		RenderMetaFile();
		break;

	case WM_DESTROY:
		QuitApp();
		break;

	case WM_PAINT:
		BeginPaint(hWnd, (LPPAINTSTRUCT) &ps);
		DrawBkgnd(ps.hdc);
		EndPaint(hWnd, (LPPAINTSTRUCT) &ps);
		break;

	case WM_COMMAND:
		FExecCmd((CM) wParam);
		break;

	case WM_SYSCOMMAND:
		switch (wParam)
			{
		default:
			goto LDefWinProc;
		case 1:
			lpprocAbout =
					MakeProcInstance( (FARPROC)About, vhInst);
			DialogBox(vhInst, (LPSTR)"About",
					hWnd, lpprocAbout);
			FreeProcInstance(lpprocAbout);
			break;
			}
		break;

	case WM_MOUSEMOVE:
	case WM_SETCURSOR:
		SetCursor(hCursor);
		break;

	default:
LDefWinProc:
		return DefWindowProc(hWnd, message, wParam, lParam);
		}

	return 0L;
}


/*  F   E X E C   C M D  */
/*  execute menu commands.  
*/
FExecCmd(cm)
CM cm;
{
	FARPROC 	lpfnDlgProc;

	switch (cm)
		{
	default:
		return fFalse;
		break;

	case cmCopy:
		CmdCopy();
		break;

	case cmPaste:
		CmdPaste();
		break;

	case cmClear:
		CmdClear();
		break;

	case cmBitmap:
		CmdBitmap();
		break;

	case cmIcon:
		CmdIcon();
		break;

	case cmCursor:
		CmdCursor();
		break;

	case cmPrSetup:
		CmdPrinterSetup();
		break;

	case cmEnum:
		CmdEnumFonts();
		break;

	case cmEscape:
		CmdEscape();
		break;

	case cmTestPr:
		CmdTestPr();
		break;

	case cmFont:
		CmdFont();
		break;

	case cmOptions:
		CmdOptions();
		break;

	case cmFileExit:
		QuitApp();
		break;

	case cmSwMsgMode:
		nUsePeekMessage = (nUsePeekMessage+1)%3;
		DrawLlcSzN("nUsePeekMessage = ",nUsePeekMessage);
		break;

	case cmUserPr:
		CmdCustomizePr();
		break;
		}

	return fTrue;
}


/*  A B O U T */
/*  the about box proc */
BOOL FAR PASCAL About( hDlg, message, wParam, lParam )
HWND            hDlg;
unsigned        message;
WORD            wParam;
LONG            lParam;
{
	if (message == WM_COMMAND) 
		{
		EndDialog( hDlg, TRUE );
		return TRUE;
		}
	else  if (message == WM_INITDIALOG)
		return TRUE;
	else  
		return FALSE;
}


/* --------------------------------------------------------------- */
/* --------------------  Utility Procedures ---------------------- */
/* --------------------------------------------------------------- */

/*  S Z   F R O M   I N T  */
/*  given an int fill an sz with the string representation of
*  the int.  Also return the count of chars in the int.
*/
SzFromInt( i, sz, pcch)
int 	i;
char	*sz;
int	*pcch;
{
	char	szT[ichMaxSz];
	char	*pch = szT;

	if (!i)
		{
		*pcch = 1;
		sz[0] = '0';
		sz[1] = 0;
		return;
		}
	*pcch = 0;
	while ( i )
		{
		(*pcch)++;
		*pch++ = '0' + i % 10;
		i = i / 10;
		}
	pch--;
	for ( ; i < *pcch; i++)
		*sz++ = *pch--;
	*sz = '\0';
}



/* S Z   S Z   A P P E N D */
/*  Append one sz onto the end of another sz */
SzSzAppend(szTo, szFrom)
char *szTo, *szFrom;
{
	WAssert( CchSz(szTo)+CchSz(szFrom) < ichMaxSz,
			(LPSTR)"Combined string to long");

	for (; *szTo; szTo++)
		;
	while (*szTo++ = *szFrom++)
		;
}


/* S Z   L P S Z   A P P E N D */
/*  append an lpsz onto the end of an sz */
SzLpszAppend(szTo, lpszFrom)
char 	*szTo;
LPSTR 	lpszFrom;
{
	for (; *szTo; szTo++)
		;
	while (*szTo++ = *lpszFrom++)
		;
}


/* S Z   B O O L   A P P E N D */
/*  append TRUE or FALSE to the end of an sz */
SzBoolAppend(sz, f)
char 	*sz;
BOOL	f;
{
	if (f)
		SzSzAppend(sz, "TRUE");
	else
		SzSzAppend(sz, "FALSE");
}


/* S Z   I N T   A P P E N D */
/*  append an int to the end of an sz */
SzIntAppend(sz, i)
char 	*sz;
int	i;
{
	int	cch;

	for (; *sz; sz++)
		;
	SzFromInt(i, sz, &cch);
}


/* F   C O M P   L P S Z */
/* compare two lpsz's to see if they are the same */
FCompLpsz(lpszOne, lpszTwo)
LPSTR lpszOne, lpszTwo;
{
	for (;*lpszOne && *lpszTwo;)
		if (*lpszOne++ != *lpszTwo++)
			return fFalse;

	while (*lpszOne++)
		if (*lpszOne != ' ')
			return fFalse;
	while (*lpszTwo++)
		if (*lpszTwo != ' ')
			return fFalse;

	return fTrue;
}


/* B L T   B X */
bltbx(lpchFrom, lpchTo, cb)
LPSTR	lpchFrom, lpchTo;
int	cb;
{
	LPSTR	lpchMac;
	lpchMac = &lpchFrom[cb];

	while (lpchFrom < lpchMac)
		*lpchTo++ = *lpchFrom++;
}


/* D R A W   L L C   S Z   N */
/* Draw an sz and an int to the main app window lower left corner (llc). */
DrawLlcSzN(sz, n)
char *sz;
int n;
{
	RECT rect;
	HDC hdc = GetDC(vhwnd);
	char szInt[20];
	int cch;

	GetClientRect(vhwnd, (LPRECT)&rect);

	rect.top = rect.bottom - 15;
	PatBlt(hdc, rect.left, rect.top, rect.right-rect.left, 
			rect.bottom-rect.top, PATCOPY);

	cch = CchSz(sz);
	TextOut(hdc, rect.left+2, rect.top+2, (LPSTR)sz, CchSz(sz));
	if (n >= 0)
		{
		SzFromInt(n, szInt, &cch);
		TextOut(hdc, rect.left+2+LOWORD(GetTextExtent(hdc, (LPSTR)sz,
				CchSz(sz))), rect.top+2, (LPSTR)szInt, cch);
		}
	ReleaseDC(vhwnd, hdc);
}


/* C C H   S Z */
/* count the number of characters in an int */
CchSz(pch)
char *pch;
{
	int n = 0;
	while (*pch++)
		n++;
	return n;
}


/*	  REVIEW NEXT TWO !!!  Not currently Used!
/* M Y   M E S S A G E */
/* MyMessage is a dialog box with no controls that just sits on top of the
	app window and displays a message.  There are two lines in the db which
	can be changed independantly.  It is useful for providing a "please be
	patient" box during long ops. */
/*
BOOL FAR PASCAL MyMessage(hDlg, iMessage, wParam, lParam)
HWND	hDlg;
unsigned iMessage;
WORD	wParam;
DWORD	lParam;
{
	switch (iMessage)
		{
		default:
			return fFalse;
		}
	return fTrue;
}


/* C R E A T E   M Y   M E S S A G E */
/* control proc. for MyMessage box.  Five options are available: create,
	destroy, or change one or both of the messages in the box. */
/*
CreateMyMessage(mm, sz1, sz2)
MM		mm;
char	sz1[ichMaxSz], sz2[ichMaxSz];

{
	static HWND		hDlg;
	static FARPROC		lpfnMyMessage;

	switch (mm)
		{
		case MM_DESTROY:
			EnableWindow(vhwnd, fTrue);
			DestroyWindow(hDlg);
			FreeProcInstance(lpfnMyMessage);
			break;

		case MM_CREATE:
			EnableWindow(vhwnd, fFalse);
			lpfnMyMessage = MakeProcInstance(MyMessage, vhInst);
			hDlg = CreateDialog(vhInst, "MyMessageDlg",
					vhwnd, lpfnMyMessage);

			SetDlgItemText(hDlg, idd_Text1,(LPSTR)sz1);
			SetDlgItemText(hDlg, idd_Text2,(LPSTR)sz2);
			break;

		case MM_CHANGEBOTH:
			SetDlgItemText(hDlg, idd_Text1,(LPSTR)sz1);
			SetDlgItemText(hDlg, idd_Text2,(LPSTR)sz2);
			break;

		case MM_CHANGE1:
			SetDlgItemText(hDlg, idd_Text1,(LPSTR)sz1);
			break;

		case MM_CHANGE2:
			SetDlgItemText(hDlg, idd_Text2,(LPSTR)sz2);
			break;

		default:
			return fFalse;
		}
	return fTrue;
}
	Previous two proc's currently unused */


/* R E P O R T   N U M */
/* display a message box with the provided string and int. */
ReportNum(pch, n)
char *pch;
short	n;
{
	SzIntAppend(pch, n);
	MessageBox( vhwnd,
			(LPSTR) pch,
			(LPSTR) vszApp,
			MB_OK | MB_ICONEXCLAMATION);
}


/* R E P O R T   S Z */
/*  a message box with the given sz. */
ReportSz(sz)
char 	sz[];
{
	MessageBox( vhwnd,
			(LPSTR) sz,
			(LPSTR) vszApp,
			MB_OK | MB_ICONEXCLAMATION);
}


/*  M U L T   D I V */
/*  given three ints multiply the first two and divide the third.
	Use long's for better precision and adjust for rounding rather than
	truncation */
LONG MultDiv(a,b,c)
int a, b, c;
{
	LONG d;

	d = (LONG)a * (LONG)b;

	/* adding half the divisor will cause correct rounding!! */
	d += ((LONG)c/2);

	d /= (LONG)c;

	return  d;
}



