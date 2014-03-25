/*  WARNING --- If you modify this file provide updated version for
	inclusion in the SDM library!!
*/

#ifdef DEBUG
#ifndef REALWINCALLS

#undef BitBlt
#define BitBlt(P1,P2,P3,P4,P5,P6,P7,P8,P9) BitBltFP((HDC) (P1), (short) (P2), \
		(short) (P3), (short) (P4), (short) (P5), (HDC) (P6), (short) (P7), (short) (P8), (DWORD) (P9))
EXPORT BitBltFP();

#undef ChangeMenu
#define ChangeMenu(P1,P2,P3,P4,P5) ChangeMenuFP((HMENU) (P1), (WORD) (P2),\
		(LPSTR) (P3), (WORD) (P4), (WORD) (P5))
EXPORT ChangeMenuFP();

#undef CombineRgn
#define CombineRgn(P1,P2,P3,P4) CombineRgnFP((HRGN) (P1), (HRGN) (P2), (HRGN) (P3), (int) (P4))
EXPORT CombineRgnFP();

#undef CreateBitmap
#define CreateBitmap(P1,P2,P3,P4,P5) CreateBitmapFP((short) (P1), (short) (P2),\
		(BYTE) (P3), (BYTE) (P4), (LPSTR) (P5))
EXPORT CreateBitmapFP();

#undef CreateBitmapIndirect
#define CreateBitmapIndirect(P1) CreateBMIndirectFP((BITMAP FAR *) (P1))
EXPORT CreateBMIndirectFP();

#undef CreateCompatibleBitmap
#define CreateCompatibleBitmap(P1,P2,P3) CreateCompatBitmapFP((HDC) (P1), (short) (P2), (short) (P3))
EXPORT CreateCompatBitmapFP();

#undef CreateCompatibleDC
#define CreateCompatibleDC(P1) CreateCompatDCFP((HDC) (P1))
EXPORT CreateCompatDCFP();

#undef CreateDC
#define CreateDC(P1,P2,P3,P4) CreateDCFP((LPSTR) (P1), (LPSTR) (P2), (LPSTR) (P3), (LPSTR) (P4))
EXPORT CreateDCFP();

#undef CreateFontIndirect
#define CreateFontIndirect(P1) CreateFontIndirectFP((LOGFONT FAR *) (P1))
EXPORT CreateFontIndirectFP();

#undef CreateIC
#define CreateIC(P1,P2,P3,P4) CreateICFP((LPSTR) (P1), (LPSTR) (P2), (LPSTR) (P3), (LPSTR) (P4))
EXPORT CreateICFP();

#undef CreateMenu
#define CreateMenu() CreateMenuFP()
EXPORT CreateMenuFP();

#undef CreatePatternBrush
#define CreatePatternBrush(P1) CreatePatternBrushFP((HBITMAP) (P1))
EXPORT CreatePatternBrushFP();

#undef CreatePen
#define CreatePen(P1,P2,P3) CreatePenFP((short) (P1), (short) (P2), (DWORD) (P3))
EXPORT CreatePenFP();

#undef CreatePenIndirect
#define CreatePenIndirect(P1) CreatePenIndirectFP((LPLOGPEN) (P1))
EXPORT CreatePenIndirectFP();

#undef CreateRectRgn
#define CreateRectRgn(P1,P2,P3,P4) CreateRRgnFP((int) (P1), (int) (P2), (int) (P3), (int) (P4))
EXPORT CreateRRgnFP();

#undef CreateRectRgnIndirect
#define CreateRectRgnIndirect(P1) CreateRRgnIndirectFP((LPRECT) (P1))
EXPORT CreateRRgnIndirectFP();

#undef CreateSolidBrush
#define CreateSolidBrush(P1) CreateSolidBrushFP((DWORD) (P1))
EXPORT CreateSolidBrushFP();

#undef CreateWindow
#define CreateWindow(P1,P2,P3,P4,P5,P6,P7,P8,P9,P10,P11) CreateWindowFP((LPSTR)(P1),\
		(LPSTR)(P2),(DWORD)(P3),(int)(P4),(int)(P5),(int)(P6),(int)(P7),\
		(HWND)(P8),(HMENU)(P9),(HANDLE)(P10),(LPSTR)(P11))
EXPORT CreateWindowFPFP();

#undef DeleteDC
#define DeleteDC(P1) DeleteDCFP((HDC) (P1))
EXPORT DeleteDCFP();

#undef EmptyClipboard
#define EmptyClipboard() EmptyClipboardFP()
EXPORT EmptyClipboardFP();

#undef ExtTextOut
#define ExtTextOut(P1,P2,P3,P4,P5,P6,P7,P8) ExtTextOutFP((HDC) (P1), (int) (P2),\
		(int) (P3), (WORD) (P4), (LPRECT) (P5), (LPSTR) (P6), (int) (P7), (LPSTR) (P8))
EXPORT ExtTextOutFP();

#undef GetBitmapBits
#define GetBitmapBits(P1,P2,P3) GetBitmapBitsFP((HBITMAP) (P1), (long) (P2), (LPSTR) (P3))
EXPORT LONG GetBitmapBitsFP();

#undef GetDC
#define GetDC(P1) GetDCFP((HWND) (P1))
EXPORT GetDCFP();

#undef GetWindowDC
#define GetWindowDC(P1) GetWindowDCFP((HWND) (P1))
EXPORT GetWindowDCFP();

#undef GlobalAddAtom
#define GlobalAddAtom(P1) GlobalAddAtomFP((LPSTR) (P1))
EXPORT GlobalAddAtomFP();

#undef GlobalAlloc
#define GlobalAlloc(P1,P2) GlobalAllocFP((WORD) (P1), (DWORD) (P2))
EXPORT GlobalAllocFP();

#undef GlobalReAlloc
#define GlobalReAlloc(P1,P2,P3) GlobalReAllocFP((HANDLE) (P1), (DWORD) (P2), (WORD) (P3))
EXPORT GlobalReAllocFP();

#undef IntersectClipRect
#define IntersectClipRect(P1,P2,P3,P4,P5) IntersectClipRectFP((HDC) (P1), \
		(short) (P2), (short) (P3), (short) (P4), (short) (P5))
EXPORT IntersectClipRectFP();

#undef LoadBitmap
#define LoadBitmap(P1,P2) LoadBitmapFP((HANDLE) (P1), (LPSTR) (P2))
EXPORT LoadBitmapFP();

#undef LoadCursor
#define LoadCursor(P1,P2) LoadCursorFP((HANDLE) (P1), (LPSTR) (P2))
EXPORT LoadCursorFP();

#undef LoadLibrary
#define LoadLibrary(P1) LoadLibraryFP((LPSTR) (P1))
EXPORT LoadLibraryFP();

#undef LoadMenu
#define LoadMenu(P1,P2) LoadMenuFP((HANDLE) (P1), (LPSTR) (P2))
EXPORT LoadMenuFP();

#undef MakeProcInstance
#define MakeProcInstance(P1,P2) MakeProcInstanceFP((FARPROC) (P1), (HANDLE) (P2))
EXPORT FARPROC MakeProcInstanceFP();

#undef MessageBox
#define MessageBox(P1,P2,P3,P4) MessageBoxFP((HWND) (P1), (LPSTR) (P2), (LPSTR) (P3), (WORD) (P4))
EXPORT MessageBoxFP();

#undef OpenClipboard
#define OpenClipboard(P1) OpenClipboardFP((HWND) (P1))
EXPORT OpenClipboardFP();

#undef PatBlt
#define PatBlt(P1,P2,P3,P4,P5,P6) PatBltFP((HDC) (P1), (short) (P2), (short) (P3),\
		(short) (P4), (short) (P5), (DWORD) (P6))
EXPORT PatBltFP();

#undef PostMessage
#define PostMessage(P1,P2,P3,P4) PostMsgFP((HWND) (P1), (unsigned) (P2), (WORD) (P3), (LONG) (P4))
EXPORT PostMsgFP();

#undef RegisterClass
#define RegisterClass(P1) RegisterClassFP((LPWNDCLASS) (P1))
EXPORT RegisterClassFP();

#undef ReleaseDC
#define ReleaseDC(P1,P2) ReleaseDCFP((HWND) (P1), (HDC) (P2))
EXPORT ReleaseDCFP();

#undef RestoreDC
#define RestoreDC(P1,P2) RestoreDCFP((HDC) (P1), (short) (P2))
EXPORT RestoreDCFP();

#undef SaveDC
#define SaveDC(P1) SaveDCFP((HDC) (P1))
EXPORT SaveDCFP();

#undef ScrollDC
#define ScrollDC(P1,P2,P3,P4,P5,P6,P7) ScrollDCFP((HDC) (P1), (int) (P2),\
		(int) (P3), (LPRECT) (P4), (LPRECT) (P5), (HRGN) (P6), (LPRECT) (P7))
EXPORT ScrollDCFP();

#undef SelectObject
#define SelectObject(P1,P2) SelectObjectFP((HDC) (P1), (HANDLE) (P2))
EXPORT SelectObjectFP();

#undef SetMenu
#define SetMenu(P1,P2) SetMenuFP((HWND) (P1), (HMENU) (P2))
EXPORT SetMenuFP();

#undef StretchBlt
#define StretchBlt(P1,P2,P3,P4,P5,P6,P7,P8,P9,P10,P11) StretchBltFP((HDC) (P1),\
		(short) (P2), (short) (P3), (short) (P4), (short) (P5), (HDC) (P6),\
		(short) (P7), (short) (P8), (short) (P9), (short) (P10), (DWORD) (P11))
EXPORT StretchBltFP();

#undef UnhookWindowsHook
#define UnhookWindowsHook(P1,P2) UnhookWindowsHookFP((int) (P1), (FARPROC) (P2))
EXPORT UnhookWindowsHookFP();

#define GlobalLockClip(P1) GlobalLockClipFP((HANDLE) (P1))
EXPORT LPSTR GlobalLockClipFP();

#endif /* REALWINCALLS */

#else /* !DEBUG */
#define GlobalLockClip GlobalLock
#endif /* DEBUG */


#ifdef HYBRID
#ifndef REALWINCALLS

#undef FillWindow
#define FillWindow(P1,P2,P3,P4) FillWindowPR((HWND) (P1), (HWND) (P2), (HDC) (P3), (HBRUSH) (P4))
NATIVE void FillWindowPR();

#undef BeginPaint
#define BeginPaint(P1,P2) BeginPaintPR((HWND) (P1), (LPPAINTSTRUCT) (P2))
NATIVE HDC BeginPaintPR();

#undef BitBlt
#define BitBlt(P1,P2,P3,P4,P5,P6,P7,P8,P9) BitBltPR((HDC) (P1), (short) (P2), (short) (P3),\
		(short) (P4), (short) (P5), (HDC) (P6), (short) (P7), (short) (P8), (DWORD) (P9))
NATIVE BOOL BitBltPR();

#undef CallWindowProc
#define CallWindowProc(P1,P2,P3,P4,P5) CallWindowProcPR((FARPROC) (P1), (HWND) (P2), \
		(unsigned) (P3), (WORD) (P4), (LONG) (P5))
NATIVE long CallWindowProcPR();

#undef ChangeMenu
#define ChangeMenu(P1,P2,P3,P4,P5) ChangeMenuPR((HMENU) (P1), (WORD) (P2), (LPSTR) (P3), \
		(WORD) (P4), (WORD) (P5))
NATIVE BOOL ChangeMenuPR();

#undef CheckDlgButton
#define CheckDlgButton(P1,P2,P3) CheckDlgButtonPR((HWND) (P1), (int) (P2), (WORD) (P3))
NATIVE void CheckDlgButtonPR();

#undef CheckMenuItem
#define CheckMenuItem(P1,P2,P3) CheckMenuItemPR((HMENU) (P1), (WORD) (P2), (WORD) (P3))
NATIVE BOOL CheckMenuItemPR();

#undef CheckRadioButton
#define CheckRadioButton(P1,P2,P3,P4) CheckRadioButtonPR((HWND) (P1), (int) (P2), (int) (P3), \
		(int) (P4))
NATIVE void CheckRadioButtonPR();

#undef CloseClipboard
#define CloseClipboard() CloseClipboardPR()
NATIVE BOOL CloseClipboardPR();

#undef CreateBitmap
#define CreateBitmap(P1,P2,P3,P4,P5) CreateBitmapPR((short) (P1), (short) (P2), (BYTE) (P3),\
		(BYTE) (P4), (LPSTR) (P5))
NATIVE HBITMAP CreateBitmapPR();

#undef CreateBitmapIndirect
#define CreateBitmapIndirect(P1) CreateBMIndirectPR((BITMAP FAR *) (P1))
NATIVE HBITMAP CreateBMIndirectPR();

#undef CreateCaret
#define CreateCaret(P1,P2,P3,P4) CreateCaretPR((HWND) (P1), (HBITMAP) (P2), (int) (P3), (int) (P4))
NATIVE void CreateCaretPR();

#undef CreateCompatibleBitmap
#define CreateCompatibleBitmap(P1,P2,P3) CreateComBitmapPR((HDC) (P1), (short) (P2), \
		(short) (P3))
NATIVE HBITMAP CreateCompatibleBitmapPR();

#undef CreateCompatibleDC
#define CreateCompatibleDC(P1) CreateComDCPR((HDC) (P1))
NATIVE HDC CreateCompatibleDCPR();

#undef CreateDC
#define CreateDC(P1,P2,P3,P4) CreateDCPR((LPSTR) (P1), (LPSTR) (P2), (LPSTR) (P3), (LPSTR) (P4))
NATIVE HDC CreateDCPR();

#undef CreateFontIndirect
#define CreateFontIndirect(P1) CreateFontIndirectPR((LOGFONT FAR *) (P1))
NATIVE HFONT CreateFontIndirectPR();

#undef CreateIC
#define CreateIC(P1,P2,P3,P4) CreateICPR((LPSTR) (P1), (LPSTR) (P2), (LPSTR) (P3), (LPSTR) (P4))
NATIVE HDC CreateICPR();

#undef CreatePatternBrush
#define CreatePatternBrush(P1) CreatePatternBrushPR((HBITMAP) (P1))
NATIVE HBRUSH CreatePatternBrushPR();

#undef CreateSolidBrush
#define CreateSolidBrush(P1) CreateSolidBrushPR((DWORD) (P1))
NATIVE HBRUSH CreateSolidBrushPR();

#undef CreateWindow
#define CreateWindow(P1,P2,P3,P4,P5,P6,P7,P8,P9,P10,P11) CreateWindowPR((LPSTR) (P1), (LPSTR) (P2),\
		(DWORD) (P3), (int) (P4), (int) (P5), (int) (P6), (int) (P7), (HWND) (P8), (HMENU) (P9), \
		(HANDLE) (P10), (LPSTR) (P11))
NATIVE HWND CreateWindowPR();

#undef DeleteDC
#define DeleteDC(P1) DeleteDCPR((HDC) (P1))
NATIVE BOOL DeleteDCPR();

#undef DeleteObject
#define DeleteObject(P1) DeleteObjectPR((HANDLE) (P1))
NATIVE BOOL DeleteObjectPR();

#undef DestroyCaret
#define DestroyCaret() DestroyCaretPR()
NATIVE void DestroyCaretPR();

#undef DrawText
#define DrawText(P1,P2,P3,P4,P5) DrawTextPR((HDC) (P1), (LPSTR) (P2), (int) (P3), (LPRECT) (P4), \
		(WORD) (P5))
NATIVE void DrawTextPR();

#undef EmptyClipboard
#define EmptyClipboard() EmptyClipboardPR()
NATIVE BOOL EmptyClipboardPR();

#undef EnableWindow
#define EnableWindow(P1,P2) EnableWindowPR((HWND) (P1), (BOOL) (P2))
NATIVE BOOL EnableWindowPR();

#undef EndPaint
#define EndPaint(P1,P2) EndPaintPR((HWND) (P1), (LPPAINTSTRUCT) (P2))
NATIVE void EndPaintPR();

#undef Escape
#define Escape(P1,P2,P3,P4,P5) EscapePR((HDC) (P1), (short) (P2), (short) (P3), (LPSTR) (P4),\
		(LPSTR) (P5))
NATIVE short EscapePR();

#undef FillRect
#define FillRect(P1,P2,P3) FillRectPR((HDC) (P1), (LPRECT) (P2), (HBRUSH) (P3))
NATIVE int FillRectPR();

#undef FillRgn
#define FillRgn(P1,P2,P3) FillRgnPR((HDC) (P1), (HRGN) (P2), (HBRUSH) (P3))
NATIVE BOOL FillRgnPR();

#undef GetClipboardData
#define GetClipboardData(P1) GetClipboardDataPR((WORD) (P1))
NATIVE HANDLE GetClipboardDataPR();

#undef GetDC
#define GetDC(P1) GetDCPR((HWND) (P1))
NATIVE HDC GetDCPR();

#undef GetMenu
#define GetMenu(P1) GetMenuPR((HWND) (P1))
NATIVE HMENU GetMenuPR();

#undef GetMenuString
#define GetMenuString(P1,P2,P3,P4,P5) GetMenuStringPR((HMENU) (P1), (WORD) (P2), (LPSTR) (P3),\
		(int) (P4), (WORD) (P5))
NATIVE int GetMenuStringPR();

#undef GetMessage
#define GetMessage(P1,P2,P3,P4) GetMessagePR((LPMSG) (P1), (HWND) (P2), (unsigned) (P3),\
		(unsigned) (P4))
NATIVE BOOL GetMessagePR();

#undef GetProfileInt
#define GetProfileInt(P1,P2,P3) GetProfileIntPR((LPSTR) (P1), (LPSTR) (P2), (int) (P3))
NATIVE int GetProfileIntPR();

#undef GetProfileString
#define GetProfileString(P1,P2,P3,P4,P5) GetProfileStringPR((LPSTR) (P1), (LPSTR) (P2), (LPSTR) (P3), (LPSTR) (P4), (int) (P5))
NATIVE int GetProfileStringPR();

#undef GetStockObject
#define GetStockObject(P1) GetStockObjectPR((short) (P1))
NATIVE HANDLE GetStockObjectPR();

#undef GetTempFileName
#define GetTempFileName(P1,P2,P3,P4) GetTempFileNamePR((BYTE) (P1), (LPSTR) (P2), (WORD) (P3), \
		(LPSTR) (P4))
NATIVE int GetTempFileNamePR();

#undef GetTextExtent
#define GetTextExtent(P1,P2,P3) GetTextExtentPR((HDC) (P1), (LPSTR) (P2), (short) (P3))
NATIVE DWORD GetTextExtentPR();

#undef GetUpdateRect
#define GetUpdateRect(P1,P2,P3) GetUpdateRectPR((HWND) (P1), (LPRECT) (P2), (BOOL) (P3))
NATIVE BOOL GetUpdateRectPR();

#undef GlobalAlloc
#define GlobalAlloc(P1,P2) GlblAllocPR((WORD) (P1), (DWORD) (P2))
NATIVE HANDLE GlblAllocPR();

#undef GlobalFree
#define GlobalFree(P1) GlobalFreePR((HANDLE) (P1))
NATIVE HANDLE GlobalFreePR();

#undef GlobalLock
#define GlobalLock(P1) GlobalLockPR((HANDLE) (P1))
NATIVE LPSTR GlobalLockPR();

#undef GlobalReAlloc
#define GlobalReAlloc(P1,P2,P3) GlobalReAllocPR((HANDLE) (P1), (DWORD) (P2), (WORD) (P3))
NATIVE HANDLE GlobalReAllocPR();

#undef GlobalSize
#define GlobalSize(P1) GlobalSizePR((HANDLE) (P1))
NATIVE DWORD GlobalSizePR();

#undef GlobalUnlock
#define GlobalUnlock(P1) GlobalUnlockPR((HANDLE) (P1))
NATIVE BOOL GlobalUnlockPR();

#undef GrayString
#define GrayString(P1,P2,P3,P4,P5,P6,P7,P8,P9) GrayStringPR((HDC) (P1), (HBRUSH) (P2), \
		(FARPROC) (P3), (DWORD) (P4), (int) (P5), (int) (P6), (int) (P7), (int) (P8), (int) (P9))
NATIVE BOOL GrayStringPR();

#undef HideCaret
#define HideCaret(P1) HideCaretPR((HWND) (P1))
NATIVE void HideCaretPR();

#undef InvalidateRect
#define InvalidateRect(P1,P2,P3) InvalidateRectPR((HWND) (P1), (LPRECT) (P2), (BOOL) (P3))
NATIVE void InvalidateRectPR();

#undef InvertRect
#define InvertRect(P1,P2) InvertRectPR((HDC) (P1), (LPRECT) (P2))
NATIVE int InvertRectPR();

#undef LineTo
#define LineTo(P1,P2,P3) LineToPR((HDC) (P1), (short) (P2), (short) (P3))
NATIVE BOOL LineToPR();

#undef MessageBeep
#define MessageBeep(P1) MsgBeepPR((WORD) (P1))
NATIVE BOOL MsgBeepPR();

#undef MessageBox
#define MessageBox(P1,P2,P3,P4) MessageBoxPR((HWND) (P1), (LPSTR) (P2), (LPSTR) (P3), (WORD) (P4))
NATIVE int MessageBoxPR();

#undef MoveTo
#define MoveTo(P1,P2,P3) MoveToPR((HDC) (P1), (short) (P2), (short) (P3))
NATIVE DWORD MoveToPR();

#undef OpenClipboard
#define OpenClipboard(P1) OpenClipboardPR((HWND) (P1))
NATIVE BOOL OpenClipboardPR();

#undef OpenFile
#define OpenFile(P1,P2,P3) OpenFilePR((LPSTR) (P1), (LPOFSTRUCT) (P2), (WORD) (P3))
NATIVE int OpenFilePR();

#undef PatBlt
#define PatBlt(P1,P2,P3,P4,P5,P6) PatBltPR((HDC) (P1), (short) (P2), (short) (P3), (short) (P4), \
		(short) (P5), (DWORD) (P6))
NATIVE BOOL PatBltPR();

#undef PeekMessage
#define PeekMessage(P1,P2,P3,P4,P5) PeekMsgPR((LPMSG) (P1), (HWND) (P2), (unsigned) (P3), \
		(WORD) (P4), (BOOL) (P5))
NATIVE BOOL PeekMsgPR();

#undef PostMessage
#define PostMessage(P1,P2,P3,P4) PostMsgPR((HWND) (P1), (unsigned) (P2), (WORD) (P3), \
		(LONG) (P4))
NATIVE BOOL PostMessagePR();

#undef ReleaseCapture
#define ReleaseCapture() ReleaseCapturePR()
NATIVE void ReleaseCapturePR();

#undef RestoreDC
#define RestoreDC(P1,P2) RestoreDCPR((HDC) (P1), (short) (P2))
NATIVE int RestoreDCPR();

#undef SaveDC
#define SaveDC(P1) SaveDCPR((HDC) (P1))
NATIVE short SaveDCPR();

#undef ScrollWindow
#define ScrollWindow(P1,P2,P3,P4,P5) ScrollWindowPR((HWND) (P1), (int) (P2), (int) (P3), \
		(LPRECT) (P4), (LPRECT) (P5))
NATIVE void ScrollWindowPR();

#undef SelectObject
#define SelectObject(P1,P2) SelectObjectPR((HDC) (P1), (HANDLE) (P2))
NATIVE HANDLE SelectObjectPR();

#undef SendMessage
#define SendMessage(P1,P2,P3,P4) SendMsgPR((HWND) (P1), (unsigned) (P2), (WORD) (P3), \
		(LONG) (P4))
NATIVE long SendMsgPR();

#undef SetActiveWindow
#define SetActiveWindow(P1) SetActiveWindowPR((HWND) (P1))
NATIVE HWND SetActiveWindowPR();

#undef SetCapture
#define SetCapture(P1) SetCapturePR((HWND) (P1))
NATIVE HWND SetCapturePR();

#undef SetClipboardData
#define SetClipboardData(P1,P2) SetClipboardDataPR((WORD) (P1), (HANDLE) (P2))
NATIVE HANDLE SetClipboardDataPR();

#undef SetCursor
#define SetCursor(P1) SetCursorPR((HCURSOR) (P1))
NATIVE HCURSOR SetCursorPR();

#undef SetFocus
#define SetFocus(P1) SetFocusPR((HWND) (P1))
NATIVE HWND SetFocusPR();

#undef ShowCaret
#define ShowCaret(P1) ShowCaretPR((HWND) (P1))
NATIVE void ShowCaretPR();

#undef ShowCursor
#define ShowCursor(P1) ShowCursorPR((BOOL) (P1))
NATIVE int ShowCursorPR();

#undef ShowWindow
#define ShowWindow(P1,P2) ShowWindowPR((HWND) (P1), (int) (P2))
NATIVE BOOL ShowWindowPR();

#undef StretchBlt
#define StretchBlt(P1,P2,P3,P4,P5,P6,P7,P8,P9,P10,P11) StretchBltPR((HDC) (P1), (short) (P2), \
		(short) (P3), (short) (P4), (short) (P5), (HDC) (P6), (short) (P7), (short) (P8), \
		(short) (P9), (short) (P10), (DWORD) (P11))
NATIVE BOOL StretchBltPR();

#undef TextOut
#define TextOut(P1,P2,P3,P4,P5) TextOutPR((HDC) (P1), (short) (P2), (short) (P3), (LPSTR) (P4), \
		(short) (P5))
NATIVE BOOL TextOutPR();

#undef TranslateMessage
#define TranslateMessage(P1) TranslateMessagePR((LPMSG) (P1))
NATIVE BOOL TranslateMessagePR();

#undef UpdateWindow
#define UpdateWindow(P1) UpdateWindowPR((HWND) (P1))
NATIVE void UpdateWindowPR();

#undef ValidateRect
#define ValidateRect(P1,P2) ValidateRectPR((HWND) (P1), (LPRECT) (P2))
NATIVE void ValidateRectPR();

#undef WriteProfileString
#define WriteProfileString(P1,P2,P3) WriteProfileStringPR((LPSTR) (P1), (LPSTR) (P2), (LPSTR) (P3))
NATIVE BOOL WriteProfileStringPR();

#undef SelectClipRgn
#define SelectClipRgn(P1,P2) SelectClipRgnPR((HDC) (P1), (HRGN) (P2))
NATIVE int SelectClipRgnPR();

#undef CreateRectRgnIndirect
#define CreateRectRgnIndirect(P1) CreateRectRgnIndirectPR((LPRECT) (P1))
NATIVE HRGN CreateRectRgnIndirectPR();

#undef BringWindowToTop
#define BringWindowToTop(P1) BringWindowToTopPR((HWND) (P1))
NATIVE void BringWindowToTopPR();

#undef LoadLibrary
#define LoadLibrary(P1) LoadLibraryPR((LPSTR) (P1))
NATIVE HANDLE LoadLibraryPR();

#undef ExcludeClipRect
#define ExcludeClipRect(P1,P2,P3,P4,P5) ExcludeClipRectPR((HDC) (P1), (short) (P2), (short) (P3), \
		(short) (P4), (short) (P5))
NATIVE short ExcludeClipRectPR();

#undef GlobalCompact
#define GlobalCompact(P1) GlobalCompactPR((DWORD) (P1))
NATIVE DWORD GlobalCompactPR();

#undef CreatePenIndirect
#define CreatePenIndirect(P1) CreatePenIndirectPR((LPLOGPEN) (P1))
NATIVE HPEN CreatePenIndirectPR();

#undef SetMenu
#define SetMenu(P1,P2) SetMenuPR((HWND) (P1), (HMENU) (P2))
NATIVE BOOL SetMenuPR();

#undef ScrollDC
#define ScrollDC(P1,P2,P3,P4,P5,P6,P7) ScrollDCPR((HDC) (P1), (int) (P2), (int) (P3), \
		(LPRECT) (P4), (LPRECT) (P5), (HRGN) (P6), (LPRECT) (P7))
NATIVE BOOL ScrollDCPR();

#undef CreatePen
#define CreatePen(P1,P2,P3) CreatePenPR((short) (P1), (short) (P2), (DWORD) (P3))
NATIVE HPEN CreatePenPR();

#undef CreateMenu
#define CreateMenu() CreateMenuPR()
NATIVE HMENU CreateMenuPR();

#undef GetMenuItemCount
#define GetMenuItemCount(P1) GetMenuItemCountPR((HMENU) (P1))
NATIVE WORD GetMenuItemCountPR();

#undef GetMenuItemId
#define GetMenuItemId(P1,P2) GetMenuItemIdPR((HMENU) (P1), (WORD) (P2))
NATIVE WORD GetMenuItemIdPR();

#undef GetInputState
#define GetInputState() GetInputStatePR()
NATIVE BOOL GetInputStatePR();

#undef ExtTextOut
#define ExtTextOut(P1,P2,P3,P4,P5,P6,P7,P8) ExtTextOutPR((HDC) (P1), (int) (P2), (int) (P3), \
		(WORD) (P4), (LPRECT) (P5), (LPSTR) (P6), (int) (P7), (LPSTR) (P8))
NATIVE BOOL ExtTextOutPR();

#undef GetEnvironment
#define GetEnvironment(P1,P2,P3) GetEnvironmentPR((LPSTR) (P1), (LPSTR) (P2), (int) (P3))
NATIVE short GetEnvironmentPR();

#undef GetMenuState
#define GetMenuState(P1,P2,P3) GetMenuStatePR((HMENU) (P1), (WORD) (P2), (WORD) (P3))
NATIVE WORD GetMenuStatePR();

#undef Yield
#define Yield() YieldPR()
NATIVE BOOL YieldPR();

#undef GlobalWire
#define GlobalWire(P1) GlobalWirePR((HANDLE) (P1))
NATIVE LPSTR GlobalWirePR();

#undef DestroyMenu
#define DestroyMenu(P1) DestroyMenuPR((HMENU) (P1))
NATIVE BOOL DestroyMenuPR();

#undef AddFontResource
#define AddFontResource(P1) AddFontResourcePR((LPSTR) (P1))
NATIVE short AddFontResourcePR();

#undef MoveWindow
#define MoveWindow(P1,P2,P3,P4,P5,P6) MoveWindowPR((HWND) (P1), (int) (P2), (int) (P3), (int) (P4),\
		(int) (P5), (BOOL) (P6))
NATIVE void MoveWindowPR();

#undef LoadBitmap
#define LoadBitmap(P1,P2) LoadBitmapPR((HANDLE) (P1), (LPSTR) (P2))
NATIVE HBITMAP LoadBitmapPR();

#undef DefWindowProc
#define DefWindowProc(P1,P2,P3,P4) DefWinProcPR((HWND) (P1), (unsigned) (P2), (WORD) (P3), (LONG) (P4))
NATIVE long DefWinProcPR();

#undef SetTextColor
#define SetTextColor(P1,P2) SetTextColorPR((HDC) (P1), (DWORD) (P2))
NATIVE DWORD SetTextColorPR();

#undef SetBkColor
#define SetBkColor(P1,P2) SetBkColorPR((HDC) (P1), (DWORD) (P2))
NATIVE DWORD SetBkColorPR();

#undef DestroyWindow
#define DestroyWindow(P1) DestroyWindowPR((HWND) (P1))
NATIVE BOOL DestroyWindowPR();

#undef SetWindowPos
#define SetWindowPos(P1,P2,P3,P4,P5,P6,P7) SetWindowPosPR((HWND) (P1), (HWND) (P2), (int) (P3),\
		(int) (P4), (int) (P5), (int) (P6), (WORD) (P7))
NATIVE void SetWindowPosPR();

#undef SetBkMode
#define SetBkMode(P1,P2) SetBkModePR((HDC) (P1), (short) (P2))
NATIVE SetBkModePR();

#undef SetMapperFlags
#define SetMapperFlags(P1,P2) SetMapperFlagsPR((HDC) (P1), (DWORD) (P2))
NATIVE DWORD SetMapperFlagsPR();




#endif /* REALWINCALLS */
#endif /* HYBRID */
