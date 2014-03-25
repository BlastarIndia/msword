/* P R O F W I N . C */


#ifdef HYBRID

#define REALWINCALLS
#include "word.h"
#include "debug.h"
#include "lmem.h"
#include "sdmdefs.h"
#include "sdmver.h"
#include "sdm.h"

/* %%Function:FillWindowPR %%Owner:BRADV */
NATIVE void FillWindowPR(P1,P2,P3,P4) /* WINIGNORE - PROFILE only */
HWND P1;
HWND P2;
HDC P3;
HBRUSH P4;
{
	FillWindow(P1,P2,P3,P4);
}


/* %%Function:BeginPaintPR %%Owner:BRADV */
NATIVE HDC BeginPaintPR(P1,P2) /* WINIGNORE - PROFILE only */
HWND P1; 
LPPAINTSTRUCT P2;
{
	return BeginPaint(P1,P2);
}


/* %%Function:BitBltPR %%Owner:BRADV */
NATIVE BOOL BitBltPR(P1,P2,P3,P4,P5,P6,P7,P8,P9) /* WINIGNORE - PROFILE only */
HDC P1; 
short P2; 
short P3; 
short P4; 
short P5; 
HDC P6; 
short P7; 
short P8; 
DWORD P9;
{
	return BitBlt(P1,P2,P3,P4,P5,P6,P7,P8,P9);
}


/* %%Function:CallWindowProcPR %%Owner:BRADV */
NATIVE long CallWindowProcPR(P1,P2,P3,P4,P5) /* WINIGNORE - PROFILE only */
FARPROC P1; 
HWND P2;
unsigned P3; 
WORD P4; 
LONG P5;
{
	return CallWindowProc(P1,P2,P3,P4,P5);
}


/* %%Function:ChangeMenuPR %%Owner:BRADV */
NATIVE BOOL ChangeMenuPR(P1,P2,P3,P4,P5) /* WINIGNORE - PROFILE only */
HMENU P1; 
WORD P2; 
LPSTR P3; 
WORD P4; 
WORD P5;
{
	return ChangeMenu(P1,P2,P3,P4,P5);
}


/* %%Function:CheckDlgButtonPR %%Owner:BRADV */
NATIVE void CheckDlgButtonPR(P1,P2,P3) /* WINIGNORE - PROFILE only */
HWND P1; 
int P2; 
WORD P3;
{
	CheckDlgButton(P1,P2,P3);
}


/* %%Function:CheckMenuItemPR %%Owner:BRADV */
NATIVE BOOL CheckMenuItemPR(P1,P2,P3) /* WINIGNORE - PROFILE only */
HMENU P1; 
WORD P2; 
WORD P3;
{
	return CheckMenuItem(P1,P2,P3);
}


/* %%Function:CheckRadioButtonPR %%Owner:BRADV */
NATIVE void CheckRadioButtonPR(P1,P2,P3,P4) /* WINIGNORE - PROFILE only */
HWND P1; 
int P2; 
int P3; 
int P4;
{
	CheckRadioButton(P1,P2,P3,P4);
}


/* %%Function:CloseClipboardPR %%Owner:BRADV */
NATIVE BOOL CloseClipboardPR() /* WINIGNORE - PROFILE only */
{
	return CloseClipboard();
}


/* %%Function:CreateBitmapPR %%Owner:BRADV */
NATIVE HBITMAP CreateBitmapPR(P1,P2,P3,P4,P5) /* WINIGNORE - PROFILE only */
short P1; 
short P2; 
BYTE P3; 
BYTE P4; 
LPSTR P5;
{
	return CreateBitmap(P1,P2,P3,P4,P5);
}


/* %%Function:CreateBMIndirectPR %%Owner:BRADV */
NATIVE HBITMAP CreateBMIndirectPR(P1) /* WINIGNORE - PROFILE only */
BITMAP FAR * P1;
{
	return CreateBitmapIndirect(P1);
}


/* %%Function:CreateCaretPR %%Owner:BRADV */
NATIVE void CreateCaretPR(P1,P2,P3,P4) /* WINIGNORE - PROFILE only */
HWND P1; 
HBITMAP P2; 
int P3; 
int P4;
{
	CreateCaret(P1,P2,P3,P4);
}


/* %%Function:CreateComBitmapPR %%Owner:BRADV */
NATIVE HBITMAP CreateComBitmapPR(P1,P2,P3) /* WINIGNORE - PROFILE only */
HDC P1; 
short P2; 
short P3;
{
	return CreateCompatibleBitmap(P1,P2,P3);
}


/* %%Function:CreateComDCPR %%Owner:BRADV */
NATIVE HDC CreateComDCPR(P1) /* WINIGNORE - PROFILE only */
HDC P1;
{
	return CreateCompatibleDC(P1);
}


/* %%Function:CreateDCPR %%Owner:BRADV */
NATIVE HDC CreateDCPR(P1,P2,P3,P4) /* WINIGNORE - PROFILE only */
LPSTR P1; 
LPSTR P2; 
LPSTR P3; 
LPSTR P4;
{
	return CreateDC(P1,P2,P3,P4);
}


/* %%Function:CreateFontIndirectPR %%Owner:BRADV */
NATIVE HFONT CreateFontIndirectPR(P1) /* WINIGNORE - PROFILE only */
LOGFONT FAR * P1;
{
	return CreateFontIndirect(P1);
}


/* %%Function:CreateICPR %%Owner:BRADV */
NATIVE HDC CreateICPR(P1,P2,P3,P4) /* WINIGNORE - PROFILE only */
LPSTR P1; 
LPSTR P2; 
LPSTR P3; 
LPSTR P4;
{
	return CreateIC(P1,P2,P3,P4);
}


/* %%Function:CreatePatternBrushPR %%Owner:BRADV */
NATIVE HBRUSH CreatePatternBrushPR(P1) /* WINIGNORE - PROFILE only */
HBITMAP P1;
{
	return CreatePatternBrush(P1);
}


/* %%Function:CreateSolidBrushPR %%Owner:BRADV */
NATIVE HBRUSH CreateSolidBrushPR(P1) /* WINIGNORE - PROFILE only */
DWORD P1;
{
	return CreateSolidBrush(P1);
}


/* %%Function:CreateWindowPR %%Owner:BRADV */
NATIVE HWND CreateWindowPR(P1,P2,P3,P4,P5,P6,P7,P8,P9,P10,P11) /* WINIGNORE - PROFILE only */
LPSTR P1; 
LPSTR P2;
DWORD P3; 
int P4; 
int P5; 
int P6; 
int P7; 
HWND P8; 
HMENU P9;
HANDLE P10; 
LPSTR P11;
{
	return CreateWindow(P1,P2,P3,P4,P5,P6,P7,P8,P9,P10,P11);
}


/* %%Function:DeleteDCPR %%Owner:BRADV */
NATIVE BOOL DeleteDCPR(P1) /* WINIGNORE - PROFILE only */
HDC P1;
{
	return DeleteDC(P1);
}


/* %%Function:DeleteObjectPR %%Owner:BRADV */
NATIVE BOOL DeleteObjectPR(P1) /* WINIGNORE - PROFILE only */
HANDLE P1;
{
	return DeleteObject(P1);
}


/* %%Function:DestroyCaretPR %%Owner:BRADV */
NATIVE void DestroyCaretPR() /* WINIGNORE - PROFILE only */
{
	DestroyCaret();
}


/* %%Function:DrawTextPR %%Owner:BRADV */
NATIVE void DrawTextPR(P1,P2,P3,P4,P5) /* WINIGNORE - PROFILE only */
HDC P1; 
LPSTR P2; 
int P3; 
LPRECT P4;
WORD P5;
{
	DrawText(P1,P2,P3,P4,P5);
}


/* %%Function:EmptyClipboardPR %%Owner:BRADV */
NATIVE BOOL EmptyClipboardPR() /* WINIGNORE - PROFILE only */
{
	return EmptyClipboard();
}


/* %%Function:EnableWindowPR %%Owner:BRADV */
NATIVE BOOL EnableWindowPR(P1,P2) /* WINIGNORE - PROFILE only */
HWND P1; 
BOOL P2;
{
	return EnableWindow(P1,P2);
}


/* %%Function:EndPaintPR %%Owner:BRADV */
NATIVE void EndPaintPR(P1,P2) /* WINIGNORE - PROFILE only */
HWND P1; 
LPPAINTSTRUCT P2;
{
	EndPaint(P1,P2);
}


/* %%Function:EscapePR %%Owner:BRADV */
NATIVE short EscapePR(P1,P2,P3,P4,P5) /* WINIGNORE - PROFILE only */
HDC P1; 
short P2; 
short P3; 
LPSTR P4;
LPSTR P5;
{
	return Escape(P1,P2,P3,P4,P5);
}


/* %%Function:FillRectPR %%Owner:BRADV */
NATIVE int FillRectPR(P1,P2,P3) /* WINIGNORE - PROFILE only */
HDC P1; 
LPRECT P2; 
HBRUSH P3;
{
	return FillRect(P1,P2,P3);
}


/* %%Function:FillRgnPR %%Owner:BRADV */
NATIVE BOOL FillRgnPR(P1,P2,P3) /* WINIGNORE - PROFILE only */
HDC P1; 
HRGN P2; 
HBRUSH P3;
{
	return FillRgn(P1,P2,P3);
}


/* %%Function:GetClipboardDataPR %%Owner:BRADV */
NATIVE HANDLE GetClipboardDataPR(P1) /* WINIGNORE - PROFILE only */
WORD P1;
{
	return GetClipboardData(P1);
}


/* %%Function:GetDCPR %%Owner:BRADV */
NATIVE HDC GetDCPR(P1) /* WINIGNORE - PROFILE only */
HWND P1;
{
	return GetDC(P1);
}


/* %%Function:GetMenuPR %%Owner:BRADV */
NATIVE HMENU GetMenuPR(P1) /* WINIGNORE - PROFILE only */
HWND P1;
{
	return GetMenu(P1);
}


/* %%Function:GetMenuStringPR %%Owner:BRADV */
NATIVE int GetMenuStringPR(P1,P2,P3,P4,P5) /* WINIGNORE - PROFILE only */
HMENU P1; 
WORD P2; 
LPSTR P3;
int P4; 
WORD P5;
{
	return GetMenuString(P1,P2,P3,P4,P5);
}


/* %%Function:GetMessagePR %%Owner:BRADV */
NATIVE BOOL GetMessagePR(P1,P2,P3,P4) /* WINIGNORE - PROFILE only */
LPMSG P1; 
HWND P2; 
unsigned P3;
unsigned P4;
{
	return GetMessage(P1,P2,P3,P4);
}


/* %%Function:GetProfileIntPR %%Owner:BRADV */
NATIVE int GetProfileIntPR(P1,P2,P3) /* WINIGNORE - PROFILE only */
LPSTR P1; 
LPSTR P2; 
int P3;
{
	return GetProfileInt(P1,P2,P3);
}


/* %%Function:GetProfileStringPR %%Owner:BRADV */
NATIVE int GetProfileStringPR(P1,P2,P3,P4,P5) /* WINIGNORE - PROFILE only */
LPSTR P1; 
LPSTR P2; 
LPSTR P3; 
LPSTR P4; 
int P5;
{
	return GetProfileString(P1,P2,P3,P4,P5);
}


/* %%Function:GetStockObjectPR %%Owner:BRADV */
NATIVE HANDLE GetStockObjectPR(P1) /* WINIGNORE - PROFILE only */
short P1;
{
	return GetStockObject(P1);
}


/* %%Function:GetTempFileNamePR %%Owner:BRADV */
NATIVE int GetTempFileNamePR(P1,P2,P3,P4) /* WINIGNORE - PROFILE only */
BYTE P1; 
LPSTR P2; 
WORD P3;
LPSTR P4;
{
	return GetTempFileName(P1,P2,P3,P4);
}


/* %%Function:GetTextExtentPR %%Owner:BRADV */
NATIVE DWORD GetTextExtentPR(P1,P2,P3) /* WINIGNORE - PROFILE only */
HDC P1; 
LPSTR P2; 
short P3;
{
	return GetTextExtent(P1,P2,P3);
}


/* %%Function:GetUpdateRectPR %%Owner:BRADV */
NATIVE BOOL GetUpdateRectPR(P1,P2,P3) /* WINIGNORE - PROFILE only */
HWND P1; 
LPRECT P2; 
BOOL P3;
{
	return GetUpdateRect(P1,P2,P3);
}


/* %%Function:GlblAllocPR %%Owner:BRADV */
NATIVE HANDLE GlblAllocPR(P1,P2) /* WINIGNORE - PROFILE only */
WORD P1; 
DWORD P2;
{
	return GlobalAlloc(P1,P2);
}


/* %%Function:GlobalFreePR %%Owner:BRADV */
NATIVE HANDLE GlobalFreePR(P1) /* WINIGNORE - PROFILE only */
HANDLE P1;
{
	return GlobalFree(P1);
}


/* %%Function:GlobalLockPR %%Owner:BRADV */
NATIVE LPSTR GlobalLockPR(P1) /* WINIGNORE - PROFILE only */
HANDLE P1;
{
	return GlobalLock(P1);
}


/* %%Function:GlobalReAllocPR %%Owner:BRADV */
NATIVE HANDLE GlobalReAllocPR(P1,P2,P3) /* WINIGNORE - PROFILE only */
HANDLE P1; 
DWORD P2; 
WORD P3;
{
	return GlobalReAlloc(P1,P2,P3);
}


/* %%Function:GlobalSizePR %%Owner:BRADV */
NATIVE DWORD GlobalSizePR(P1) /* WINIGNORE - PROFILE only */
HANDLE P1;
{
	return GlobalSize(P1);
}


/* %%Function:GlobalUnlockPR %%Owner:BRADV */
NATIVE BOOL GlobalUnlockPR(P1) /* WINIGNORE - PROFILE only */
HANDLE P1;
{
	return GlobalUnlock(P1);
}


/* %%Function:GrayStringPR %%Owner:BRADV */
NATIVE BOOL GrayStringPR(P1,P2,P3,P4,P5,P6,P7,P8,P9) /* WINIGNORE - PROFILE only */
HDC P1; 
HBRUSH P2;
FARPROC P3; 
DWORD P4; 
int P5; 
int P6; 
int P7; 
int P8; 
int P9;
{
	return GrayString(P1,P2,P3,P4,P5,P6,P7,P8,P9);
}


/* %%Function:HideCaretPR %%Owner:BRADV */
NATIVE void HideCaretPR(P1) /* WINIGNORE - PROFILE only */
HWND P1;
{
	HideCaret(P1);
}


/* %%Function:InvalidateRectPR %%Owner:BRADV */
NATIVE void InvalidateRectPR(P1,P2,P3) /* WINIGNORE - PROFILE only */
HWND P1; 
LPRECT P2; 
BOOL P3;
{
	InvalidateRect(P1,P2,P3);
}


/* %%Function:InvertRectPR %%Owner:BRADV */
NATIVE int InvertRectPR(P1,P2) /* WINIGNORE - PROFILE only */
HDC P1; 
LPRECT P2;
{
	return InvertRect(P1,P2);
}


/* %%Function:LineToPR %%Owner:BRADV */
NATIVE BOOL LineToPR(P1,P2,P3) /* WINIGNORE - PROFILE only */
HDC P1; 
short P2; 
short P3;
{
	return LineTo(P1,P2,P3);
}


/* %%Function:MsgBeepPR %%Owner:BRADV */
NATIVE BOOL MsgBeepPR(P1) /* WINIGNORE - PROFILE only */
WORD P1;
{
	return MessageBeep(P1);
}


/* %%Function:MessageBoxPR %%Owner:BRADV */
NATIVE int MessageBoxPR(P1,P2,P3,P4) /* WINIGNORE - PROFILE only */
HWND P1; 
LPSTR P2; 
LPSTR P3; 
WORD P4;
{
	return MessageBox(P1,P2,P3,P4);
}


/* %%Function:MoveToPR %%Owner:BRADV */
NATIVE DWORD MoveToPR(P1,P2,P3) /* WINIGNORE - PROFILE only */
HDC P1; 
short P2; 
short P3;
{
	return MoveTo(P1,P2,P3);
}


/* %%Function:OpenClipboardPR %%Owner:BRADV */
NATIVE BOOL OpenClipboardPR(P1) /* WINIGNORE - PROFILE only */
HWND P1;
{
	return OpenClipboard(P1);
}


/* %%Function:OpenFilePR %%Owner:BRADV */
NATIVE int OpenFilePR(P1,P2,P3) /* WINIGNORE - PROFILE only */
LPSTR P1; 
LPOFSTRUCT P2; 
WORD P3;
{
	return OpenFile(P1,P2,P3);
}


/* %%Function:PatBltPR %%Owner:BRADV */
NATIVE BOOL PatBltPR(P1,P2,P3,P4,P5,P6) /* WINIGNORE - PROFILE only */
HDC P1; 
short P2; 
short P3; 
short P4;
short P5; 
DWORD P6;
{
	return PatBlt(P1,P2,P3,P4,P5,P6);
}


/* %%Function:PeekMsgPR %%Owner:BRADV */
NATIVE BOOL PeekMsgPR(P1,P2,P3,P4,P5) /* WINIGNORE - PROFILE only */
LPMSG P1; 
HWND P2; 
unsigned P3;
WORD P4; 
BOOL P5;
{
	return PeekMessage(P1,P2,P3,P4,P5);
}


/* %%Function:PostMsgPR %%Owner:BRADV */
NATIVE BOOL PostMsgPR(P1,P2,P3,P4) /* WINIGNORE - PROFILE only */
HWND P1; 
unsigned P2; 
WORD P3;
LONG P4;
{
	return PostMessage(P1,P2,P3,P4);
}


/* %%Function:ReleaseCapturePR %%Owner:BRADV */
NATIVE void ReleaseCapturePR() /* WINIGNORE - PROFILE only */
{
	ReleaseCapture();
}


/* %%Function:RestoreDCPR %%Owner:BRADV */
NATIVE int RestoreDCPR(P1,P2) /* WINIGNORE - PROFILE only */
HDC P1; 
short P2;
{
	return RestoreDC(P1,P2);
}


/* %%Function:SaveDCPR %%Owner:BRADV */
NATIVE short SaveDCPR(P1) /* WINIGNORE - PROFILE only */
HDC P1;
{
	return SaveDC(P1);
}


/* %%Function:ScrollWindowPR %%Owner:BRADV */
NATIVE void ScrollWindowPR(P1,P2,P3,P4,P5) /* WINIGNORE - PROFILE only */
HWND P1; 
int P2; 
int P3;
LPRECT P4; 
LPRECT P5;
{
	ScrollWindow(P1,P2,P3,P4,P5);
}


/* %%Function:SelectObjectPR %%Owner:BRADV */
NATIVE HANDLE SelectObjectPR(P1,P2) /* WINIGNORE - PROFILE only */
HDC P1; 
HANDLE P2;
{
	return SelectObject(P1,P2);
}


/* %%Function:SendMsgPR %%Owner:BRADV */
NATIVE long SendMsgPR(P1,P2,P3,P4) /* WINIGNORE - PROFILE only */
HWND P1; 
unsigned P2; 
WORD P3;
LONG P4;
{
	return SendMessage(P1,P2,P3,P4);
}


/* %%Function:SetActiveWindowPR %%Owner:BRADV */
NATIVE HWND SetActiveWindowPR(P1) /* WINIGNORE - PROFILE only */
HWND P1;
{
	return SetActiveWindow(P1);
}


/* %%Function:SetCapturePR %%Owner:BRADV */
NATIVE HWND SetCapturePR(P1) /* WINIGNORE - PROFILE only */
HWND P1;
{
	return SetCapture(P1);
}


/* %%Function:SetClipboardDataPR %%Owner:BRADV */
NATIVE HANDLE SetClipboardDataPR(P1,P2) /* WINIGNORE - PROFILE only */
WORD P1; 
HANDLE P2;
{
	return SetClipboardData(P1,P2);
}


/* %%Function:SetCursorPR %%Owner:BRADV */
NATIVE HCURSOR SetCursorPR(P1) /* WINIGNORE - PROFILE only */
HCURSOR P1;
{
	return SetCursor(P1);
}


/* %%Function:SetFocusPR %%Owner:BRADV */
NATIVE HWND SetFocusPR(P1) /* WINIGNORE - PROFILE only */
HWND P1;
{
	return SetFocus(P1);
}


/* %%Function:ShowCaretPR %%Owner:BRADV */
NATIVE void ShowCaretPR(P1) /* WINIGNORE - PROFILE only */
HWND P1;
{
	ShowCaret(P1);
}


/* %%Function:ShowCursorPR %%Owner:BRADV */
NATIVE int ShowCursorPR(P1) /* WINIGNORE - PROFILE only */
BOOL P1;
{
	return ShowCursor(P1);
}


/* %%Function:ShowWindowPR %%Owner:BRADV */
NATIVE BOOL ShowWindowPR(P1,P2) /* WINIGNORE - PROFILE only */
HWND P1; 
int P2;
{
	return ShowWindow(P1,P2);
}


/* %%Function:StretchBltPR %%Owner:BRADV */
NATIVE BOOL StretchBltPR(P1,P2,P3,P4,P5,P6,P7,P8,P9,P10,P11) /* WINIGNORE - PROFILE only */
HDC P1; 
short P2;
short P3; 
short P4; 
short P5; 
HDC P6; 
short P7; 
short P8;
short P9; 
short P10; 
DWORD P11;
{
	return StretchBlt(P1,P2,P3,P4,P5,P6,P7,P8,P9,P10,P11);
}


/* %%Function:TextOutPR %%Owner:BRADV */
NATIVE BOOL TextOutPR(P1,P2,P3,P4,P5) /* WINIGNORE - PROFILE only */
HDC P1; 
short P2; 
short P3; 
LPSTR P4;
short P5;
{
	return TextOut(P1,P2,P3,P4,P5);
}


/* %%Function:TranslateMessagePR %%Owner:BRADV */
NATIVE BOOL TranslateMessagePR(P1) /* WINIGNORE - PROFILE only */
LPMSG P1;
{
	return TranslateMessage(P1);
}


/* %%Function:UpdateWindowPR %%Owner:BRADV */
NATIVE void UpdateWindowPR(P1) /* WINIGNORE - PROFILE only */
HWND P1;
{
	UpdateWindow(P1);
}


/* %%Function:ValidateRectPR %%Owner:BRADV */
NATIVE void ValidateRectPR(P1,P2) /* WINIGNORE - PROFILE only */
HWND P1; 
LPRECT P2;
{
	ValidateRect(P1,P2);
}


/* %%Function:WriteProfileStringPR %%Owner:BRADV */
NATIVE BOOL WriteProfileStringPR(P1,P2,P3) /* WINIGNORE - PROFILE only */
LPSTR P1; 
LPSTR P2; 
LPSTR P3;
{
	return WriteProfileString(P1,P2,P3);
}


/* %%Function:SelectClipRgnPR %%Owner:BRADV */
NATIVE int SelectClipRgnPR(P1,P2) /* WINIGNORE - PROFILE only */
HDC P1; 
HRGN P2;
{
	return SelectClipRgn(P1,P2);
}


/* %%Function:CreateRectRgnIndirectPR %%Owner:BRADV */
NATIVE HRGN CreateRectRgnIndirectPR(P1) /* WINIGNORE - PROFILE only */
LPRECT P1;
{
	return CreateRectRgnIndirect(P1);
}


/* %%Function:BringWindowToTopPR %%Owner:BRADV */
NATIVE void BringWindowToTopPR(P1) /* WINIGNORE - PROFILE only */
HWND P1;
{
	BringWindowToTop(P1);
}


/* %%Function:LoadLibraryPR %%Owner:BRADV */
NATIVE HANDLE LoadLibraryPR(P1) /* WINIGNORE - PROFILE only */
LPSTR P1;
{
	return LoadLibrary(P1);
}


/* %%Function:ExcludeClipRectPR %%Owner:BRADV */
NATIVE ExcludeClipRectPR(P1,P2,P3,P4,P5) /* WINIGNORE - PROFILE only */
HDC P1; 
short P2; 
short P3;
short P4; 
short P5;
{
	return ExcludeClipRect(P1,P2,P3,P4,P5);
}


/* %%Function:GlobalCompactPR %%Owner:BRADV */
NATIVE DWORD GlobalCompactPR(P1) /* WINIGNORE - PROFILE only */
DWORD P1;
{
	return GlobalCompact(P1);
}


/* %%Function:CreatePenIndirectPR %%Owner:BRADV */
NATIVE HPEN CreatePenIndirectPR(P1) /* WINIGNORE - PROFILE only */
LPLOGPEN P1;
{
	return CreatePenIndirect(P1);
}


/* %%Function:SetMenuPR %%Owner:BRADV */
NATIVE BOOL SetMenuPR(P1,P2) /* WINIGNORE - PROFILE only */
HWND P1; 
HMENU P2;
{
	return SetMenu(P1,P2);
}


/* %%Function:ScrollDCPR %%Owner:BRADV */
NATIVE BOOL ScrollDCPR(P1,P2,P3,P4,P5,P6,P7) /* WINIGNORE - PROFILE only */
HDC P1; 
int P2; 
int P3;
LPRECT P4; 
LPRECT P5; 
HRGN P6; 
LPRECT P7;
{
	return ScrollDC(P1,P2,P3,P4,P5,P6,P7);
}


/* %%Function:CreatePenPR %%Owner:BRADV */
NATIVE HPEN CreatePenPR(P1,P2,P3) /* WINIGNORE - PROFILE only */
short P1; 
short P2; 
DWORD P3;
{
	return CreatePen(P1,P2,P3);
}


/* %%Function:CreateMenuPR %%Owner:BRADV */
NATIVE HMENU CreateMenuPR() /* WINIGNORE - PROFILE only */
{
	return CreateMenu();
}


/* %%Function:GetMenuItemCountPR %%Owner:BRADV */
NATIVE WORD GetMenuItemCountPR(P1) /* WINIGNORE - PROFILE only */
HMENU P1;
{
	return GetMenuItemCount(P1);
}


/* %%Function:GetMenuItemIdPR %%Owner:BRADV */
NATIVE WORD GetMenuItemIdPR(P1,P2) /* WINIGNORE - PROFILE only */
HMENU P1; 
WORD P2;
{
	return GetMenuItemId(P1,P2);
}


/* %%Function:GetInputStatePR %%Owner:BRADV */
NATIVE BOOL GetInputStatePR() /* WINIGNORE - PROFILE only */
{
	return GetInputState();
}


/* %%Function:ExtTextOutPR %%Owner:BRADV */
NATIVE BOOL ExtTextOutPR(P1,P2,P3,P4,P5,P6,P7,P8) /* WINIGNORE - PROFILE only */
HDC P1; 
int P2; 
int P3;
WORD P4; 
LPRECT P5; 
LPSTR P6; 
int P7; 
LPSTR P8;
{
	return ExtTextOut(P1,P2,P3,P4,P5,P6,P7,P8);
}


/* %%Function:GetEnvironmentPR %%Owner:BRADV */
NATIVE short GetEnvironmentPR(P1,P2,P3) /* WINIGNORE - PROFILE only */
LPSTR P1; 
LPSTR P2; 
int P3;
{
	return GetEnvironment(P1,P2,P3);
}


/* %%Function:GetMenuStatePR %%Owner:BRADV */
NATIVE WORD GetMenuStatePR(P1,P2,P3) /* WINIGNORE - PROFILE only */
HMENU P1; 
WORD P2; 
WORD P3;
{
	return GetMenuState(P1,P2,P3);
}


#endif /* HYBRID */
