#include <windows.h>
#include "ddall.h"
#include "resource.h"


HANDLE BuildMetaFile();
HANDLE HbmpGetBitmap();
HANDLE HGetRc();

HBITMAP hbmpBkgnd = NULL;
HBITMAP hbmpSystem = NULL;

extern HCURSOR hCursor;
extern HBRUSH hbrBlack, hbrWhite, hbrBack;
extern RCDS rgrcdsCur[], rgrcdsIcon[];
extern BMDS rgbmds[];
extern HWND	vhwnd;
extern char vszApp[ichMaxSz],
vszOrigApp[ichMaxSz];
extern BOOL vfLOption;


/*  C M D  P A S T E  */
/*  get data from the clipboard and draw it to the screen.  
*/
CMD CmdPaste()
{
	HANDLE hClipData;

	if (!OpenClipboard(vhwnd))
		{
		/* REVIEW: error! */
		MessageBox(vhwnd, (LPSTR) "Cannot open clipboard!",
				(LPSTR) vszApp,
				MB_OK | MB_ICONEXCLAMATION);
		return cmdError;
		}

	if ((hClipData = GetClipboardData(CF_METAFILEPICT)) != NULL)
		PasteMetaFile( hClipData);
	else if ((hClipData = GetClipboardData(CF_BITMAP)) != NULL)
		PasteBitmap( hClipData);
	else
		{
		/* REVIEW: error! */
		MessageBox(vhwnd, 
				(LPSTR) "The clipboard does not contain a metafile or bitmap!",
				(LPSTR) vszApp,
				MB_OK | MB_ICONEXCLAMATION);
		CloseClipboard();
		return cmdError;
		}

	CloseClipboard();

	return cmdOK;
}




/*  C M D  C L E A R  */
/*  clear the workspace area.   NOT YET IMPLEMENTED
*/
CMD CmdClear()
{
	WAssert(fFalse,(LPSTR)"NOT YET IMPLEMENTED");
}


/*  C M D  C O P Y  */
/*  copy a metafile to the clipboard.  Note the metafile is not rendered 
*  until it is requested.
*/
CMD CmdCopy()
{
	if (!OpenClipboard(vhwnd))
		{
		if (vfLOption)
			MessageBox(vhwnd, 
					(LPSTR) "Could Not Open Clipboard; Unable to Provide Picture for Title App.",
					(LPSTR) vszOrigApp,
					MB_OK | MB_ICONEXCLAMATION);
		return;
		}
	EmptyClipboard();
	SetClipboardData(CF_METAFILEPICT, NULL);
	CloseClipboard();
	return cmdOK;
}


/*  C M D  B I T M A P */
CmdBitmap()
{
	static int	iBitmap = 0;
	HANDLE	hBitmap;

	hBitmap = HbmpGetBitmap(iBitmap);

	PasteBitmap(hBitmap);

	DeleteObject(hBitmap);

	DrawLlcSzN("Bitmap # ", iBitmap);

	iBitmap = ++iBitmap % MAXBITMAP;
}


CmdIcon()
{
	static int	iIcon = 0;
	HANDLE hIcon;
	HDC hdc = GetDC(vhwnd);
	RECT	rect;

	hIcon = HGetRc(rgrcdsIcon, iIcon);

	SelectObject( hdc, hbrBack);

	GetClientRect(vhwnd, &rect);
	PatBlt(hdc, rect.left, rect.top, rect.right-rect.left, 
			rect.bottom-rect.top, PATCOPY);

	DrawIcon(hdc, 0, 0, hIcon);

	ReleaseDC(vhwnd, hdc);

	DrawLlcSzN("Icon # ", iIcon);

	iIcon = ++iIcon % MAXICON;
}


CmdCursor()
{
	static int	iCursor = 0;

	SetCursor(NULL);
	if (hCursor != NULL)
		GlobalFree(hCursor);

	hCursor = HGetRc(rgrcdsCur, iCursor);

	SetCursor(hCursor);

	DrawLlcSzN("Cursor # ", iCursor);

	iCursor = ++iCursor % MAXCURSOR;
}


/*	P A S T E B I T M A P  */
/*  date from the clipboard is a bitmap.  draw it to the screen. 
*/
PasteBitmap( hClipData)
HANDLE hClipData;
{
	HDC hdc, hdcDest, hdcSrc;
	HBITMAP hbmpSavDest, hbmpSavSrc;
	BITMAP bmp;


	GetObject(hClipData, sizeof (BITMAP), (LPSTR) &bmp);

	hdc = GetDC(vhwnd);
	hdcDest = CreateCompatibleDC(hdc);
	hdcSrc = CreateCompatibleDC(hdc);

	if (hbmpBkgnd != NULL)
		DeleteObject(hbmpBkgnd);
	hbmpBkgnd = CreateCompatibleBitmap(hdc, 
			bmp.bmWidth, bmp.bmHeight);

	hbmpSavDest = SelectObject(hdcDest, hbmpBkgnd);
	hbmpSavSrc = SelectObject(hdcSrc, hClipData);

	BitBlt(hdcDest, 0, 0, bmp.bmWidth, bmp.bmHeight,
			hdcSrc, 0, 0, SRCCOPY);

	SelectObject(hdcDest, hbmpSavDest);
	SelectObject(hdcSrc, hbmpSavSrc);
	DeleteDC(hdcDest);
	DeleteDC(hdcSrc);

	DrawBkgnd(hdc);
	ReleaseDC(vhwnd, hdc);
}


/*  P A S T E M E T A F I L E  */
/*  data from the clipboard is a metafile.  Draw it to the screen.
*/
PasteMetaFile( hClipData)
HANDLE hClipData;
{
	LPMETAFILEPICT lpmfp;
	HANDLE hMF;

	if ((lpmfp = (LPMETAFILEPICT)GlobalLock(hClipData)) == NULL)
		return;
	ShowPict(lpmfp->hMF, lpmfp->mm);
	GlobalUnlock(hClipData);
}


/*  D R A W B K G N D  */
/*  draw background bitmap  
*/
DrawBkgnd(hdc)
HDC hdc;
{
	HDC hdcT;
	BITMAP bmp;
	HBITMAP hbmpSav;
	RECT	rect;

	if (hbmpBkgnd == NULL)
		return;

	GetClientRect(vhwnd, &rect);

	hdcT = CreateCompatibleDC(hdc);
	hbmpSav = SelectObject(hdcT, hbmpBkgnd);

	PatBlt(hdc, rect.left, rect.top, rect.right-rect.left, 
			rect.bottom-rect.top, PATCOPY);
	GetObject(hbmpBkgnd, sizeof (BITMAP), (LPSTR) &bmp);
	BitBlt(hdc, 0, 0, bmp.bmWidth, bmp.bmHeight,
			hdcT, 0, 0, SRCCOPY);

	SelectObject(hdcT, hbmpSav);
	DeleteDC(hdcT);
}


/*  R E N D E R M E T A F I L E */
/*  render a metafile to the clipboard.
*/
RenderMetaFile()
{
	HANDLE 	hMF, hMem;
	LPMETAFILEPICT	lpmfp;
	short	mm;

	hMF = BuildMetaFile( &mm );

	hMem = GlobalAlloc( GHND, (DWORD) sizeof(METAFILEPICT));
	lpmfp = (LPMETAFILEPICT)GlobalLock( hMem);
	lpmfp->hMF = hMF;
	lpmfp->mm = mm;
	lpmfp->xExt = lpmfp->yExt = 0;
	GlobalUnlock( hMem);


	OpenClipboard(vhwnd);
	SetClipboardData(CF_METAFILEPICT, hMem);
	CloseClipboard();
}


char szMessage[] = "Next metafile: # ";

/*  B U I L D M E T A F I L E  */
/*  create a metafile.  return a handle to the metafile.  Also return the
*  mapping mode (pmm).
*/
HANDLE BuildMetaFile( pmm )
short	*pmm;
{
	HPEN	hPen;
	HBRUSH	hBrush;
	HDC	 	hDC, hDCT;
	HANDLE	hMF;
	char	szT[6];
	int		cch;

	hDCT = GetDC(vhwnd);
	SetMapMode(hDCT, MM_ANISOTROPIC);
	*pmm = MM_ANISOTROPIC;

	hDC = CreateMetaFile( NULL);
	SetWindowExt (hDC, 55, 55);

	hPen = CreatePen( 0, 0, (DWORD) 0x00FF00FF);
	SelectObject( hDC, hPen);

	hBrush = CreateSolidBrush( (DWORD) 0x0000FFFF);
	SelectObject( hDC, hBrush);

	Rectangle( hDC, 0, 0, 55, 55);

	TextOut(hDC, 4, 5,(LPSTR) "hello world", 11);

	SzFromInt( (int)((GetTickCount()/50)%1000), szT, &cch);

	TextOut(hDC, 5, 30, (LPSTR) szT, cch);

	hMF = CloseMetaFile( hDC);

	ReleaseDC(vhwnd, hDCT);

	DeleteObject (hBrush);
	DeleteObject (hPen);

	return hMF;
}


/*  S H O W P I C T  */
/*  given a handle to a metafile and the mapping mode draw the metafile to
*  the screen
*/
ShowPict( hmf, mm)
HANDLE	hmf;
short	mm;
{
	RECT	rect;
	HDC		hdc;

	hdc = GetDC(vhwnd);

	SetMapMode(hdc, mm);

	if (( mm == MM_ISOTROPIC) || ( mm == MM_ANISOTROPIC))
		{
		GetClientRect(vhwnd, &rect);
		SetViewportExt(hdc, rect.right - rect.left, 
				rect.bottom - rect.top);
		}

	PlayMetaFile(hdc, hmf);

	ReleaseDC(vhwnd, hdc);
	return cmdOK;
}


/* H B M P  G E T  B I T M A P */
HANDLE HbmpGetBitmap(i)
int	i;
{
	BMDS	*pbmds = rgbmds + i;

	return CreateBitmap( pbmds->bm.bmWidth, pbmds->bm.bmHeight,
			pbmds->bm.bmPlanes, pbmds->bm.bmBitsPixel,
			(LPSTR) pbmds->rgchBits);
}


/* H G E T R C */
HANDLE HGetRc( rgrcds, i)
RCDS rgrcds[];
int  i;
{
	RCDS *prcds = rgrcds + i;
	HANDLE	h;
	LPSTR	lpch;

	if ((h = GlobalAlloc(GMEM_MOVEABLE, (LONG) prcds->cb)) == NULL)
		return NULL;

	lpch = GlobalLock(h);
	bltbx((LPSTR) prcds->rgchBits, lpch, prcds->cb);
	GlobalUnlock(h);
	return(h);
}


