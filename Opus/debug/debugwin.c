/* D E B U G W I N . C */
/*  Routines to cause windows calls to fail
*/

#define REALWINCALLS
#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "screen.h"
#include "debug.h"

extern struct DBS vdbs;
extern BOOL vfInCmd;

/* F  F A I L  W I N  O P */
/*  Determine if the next windows call should fail.  Return fTrue if it 
	SHOULD FAIL.
*/

/* %%Function:FFailWinOp %%Owner:BRADV */
BOOL FFailWinOp()
{
	if (vfInDebug)
		return fFalse;

	if (vdbs.fOutsideCmd || vfInCmd)
		{
		if (vdbs.fWinAct)
			CommSz(SzShared(" w"));
		vdbs.cWinCalls++;
		if (vdbs.cWinSucceed)
			{
			if (vdbs.fWinAct)
				CommSz(SzShared("s "));
			vdbs.cWinSucceed--;
			return fFalse;
			}
		if (vdbs.cWinFail)
			{
			if (vdbs.fWinAct)
				CommSz(SzShared("f "));
			vdbs.cWinFail--;
			if (vdbs.fBreakOnFail)
				DebugBreak(1);
			return fTrue;
			}
		if (vdbs.fWinAct)
			CommSz(SzShared(" "));
		}
	return fFalse;
}


/* %%Function:BitBltFP %%Owner:BRADV */
EXPORT BitBltFP(P1,P2,P3,P4,P5,P6,P7,P8,P9)
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
	return vdbs.fTraceGDI ? BitBltTR(P1,P2,P3,P4,P5,P6,P7,P8,P9) : 
			BitBlt(P1,P2,P3,P4,P5,P6,P7,P8,P9);
}


/* %%Function:ChangeMenuFP %%Owner:BRADV */
EXPORT ChangeMenuFP(P1,P2,P3,P4,P5)
HMENU P1; 
WORD P2; 
LPSTR P3; 
WORD P4; 
WORD P5;
{
	return !(P5 & MF_REMOVE) && FFailWinOp() ? NULL :
			ChangeMenu(P1,P2,P3,P4,P5);
}


/* %%Function:CombineRgnFP %%Owner:BRADV */
EXPORT CombineRgnFP(P1,P2,P3,P4)
HRGN P1; 
HRGN P2; 
HRGN P3; 
int P4;
{
	return FFailWinOp() ? ERROR : CombineRgn(P1,P2,P3,P4);
}


/* %%Function:CreateBitmapFP %%Owner:BRADV */
EXPORT CreateBitmapFP(P1,P2,P3,P4,P5)
short P1; 
short P2; 
BYTE P3; 
BYTE P4; 
LPSTR P5;
{
	return FFailWinOp() ? NULL : CreateBitmap(P1,P2,P3,P4,P5);
}


/* %%Function:CreateBMIndirectFP %%Owner:BRADV */
EXPORT CreateBMIndirectFP(P1)
BITMAP FAR * P1;
{
	return FFailWinOp() ? NULL : CreateBitmapIndirect(P1);
}


/* %%Function:CreateCompatBitmapFP %%Owner:BRADV */
EXPORT CreateCompatBitmapFP(P1,P2,P3)
HDC P1; 
short P2; 
short P3;
{
	return FFailWinOp() ? NULL : CreateCompatibleBitmap(P1,P2,P3);
}


/* %%Function:CreateCompatDCFP %%Owner:BRADV */
EXPORT CreateCompatDCFP(P1)
HDC P1;
{
	return FFailWinOp() ? NULL : CreateCompatibleDC(P1);
}


/* %%Function:CreateDCFP %%Owner:BRADV */
EXPORT CreateDCFP(P1,P2,P3,P4)
LPSTR P1; 
LPSTR P2; 
LPSTR P3; 
LPSTR P4;
{
	return FFailWinOp() ? NULL : CreateDC(P1,P2,P3,P4);
}


/* %%Function:CreateFontIndirectFP %%Owner:BRADV */
EXPORT CreateFontIndirectFP(P1)
LOGFONT FAR * P1;
{
	return FFailWinOp() ? NULL : CreateFontIndirect(P1);
}


/* %%Function:CreateICFP %%Owner:BRADV */
EXPORT CreateICFP(P1,P2,P3,P4)
LPSTR P1; 
LPSTR P2; 
LPSTR P3; 
LPSTR P4;
{
	return FFailWinOp() ? NULL : CreateIC(P1,P2,P3,P4);
}


/* %%Function:CreateMenuFP %%Owner:BRADV */
EXPORT CreateMenuFP()
{
	return FFailWinOp() ? NULL : CreateMenu();
}


/* %%Function:CreatePatternBrushFP %%Owner:BRADV */
EXPORT CreatePatternBrushFP(P1)
HBITMAP P1;
{
	return FFailWinOp() ? NULL : CreatePatternBrush(P1);
}


/* %%Function:CreatePenFP %%Owner:BRADV */
EXPORT CreatePenFP(P1,P2,P3)
short P1; 
short P2; 
DWORD P3;
{
	return FFailWinOp() ? NULL : CreatePen(P1,P2,P3);
}


/* %%Function:CreatePenIndirectFP %%Owner:BRADV */
EXPORT CreatePenIndirectFP(P1)
LPLOGPEN P1;
{
	return FFailWinOp() ? NULL : CreatePenIndirect(P1);
}


/* %%Function:CreateRRgnFP %%Owner:BRADV */
EXPORT CreateRRgnFP(P1,P2,P3,P4)
int P1; 
int P2; 
int P3; 
int P4;
{
	return FFailWinOp() ? NULL : CreateRectRgn(P1,P2,P3,P4);
}


/* %%Function:CreateRRgnIndirectFP %%Owner:BRADV */
EXPORT CreateRRgnIndirectFP(P1)
LPRECT P1;
{
	return FFailWinOp() ? NULL : CreateRectRgnIndirect(P1);
}


/* %%Function:CreateSolidBrushFP %%Owner:BRADV */
EXPORT CreateSolidBrushFP(P1)
DWORD P1;
{
	return FFailWinOp() ? NULL : CreateSolidBrush(P1);
}


/* %%Function:CreateWindowFP %%Owner:BRADV */
EXPORT CreateWindowFP(P1,P2,P3,P4,P5,P6,P7,P8,P9,P10,P11)
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
	return FFailWinOp() ? NULL : 
			CreateWindow(P1,P2,P3,P4,P5,P6,P7,P8,P9,P10,P11);
}


/* %%Function:DeleteDCFP %%Owner:BRADV */
EXPORT DeleteDCFP(P1)
HDC P1;
{
	return vdbs.fTraceGDI ? DeleteDCTR(P1) : DeleteDC(P1);
}


/* %%Function:EmptyClipboardFP %%Owner:BRADV */
EXPORT EmptyClipboardFP()
{
	return FFailWinOp() ? NULL : EmptyClipboard();
}


/* %%Function:EnableWindowFP %%Owner:BRADV */
EXPORT EnableWindowFP(P1,P2)
HWND P1; 
BOOL P2;
{
/* this really shouldn't be able to fail after all!  (keep around
so that SDM will link) */
	return EnableWindow(P1,P2);
}


/* %%Function:ExtTextOutFP %%Owner:BRADV */
EXPORT ExtTextOutFP(P1,P2,P3,P4,P5,P6,P7,P8)
HDC P1; 
int P2; 
int P3; 
WORD P4; 
LPRECT P5; 
LPSTR P6; 
int P7; 
LPSTR P8;
{
	if (vdbs.fUseTextOut)
		return TextOut(P1, P2, P3, P6, P7);

	if (vdbs.fExtTextOutNull)
		{
		P4 = 0;
		P5 = (LPRECT) 0;
		P8 = (LPSTR) 0;
		}

	return vdbs.fTraceGDI ? ExtTextOutTR(P1,P2,P3,P4,P5,P6,P7,P8) :
			ExtTextOut(P1,P2,P3,P4,P5,P6,P7,P8);
}


/* %%Function:GetBitmapBitsFP %%Owner:BRADV */
EXPORT LONG GetBitmapBitsFP(P1,P2,P3)
HBITMAP P1; 
long P2; 
LPSTR P3;
{
	return FFailWinOp() ? NULL : GetBitmapBits(P1,P2,P3);
}


/* %%Function:GetDCFP %%Owner:BRADV */
EXPORT GetDCFP(P1)
HWND P1;
{
	return FFailWinOp() ? NULL : GetDC(P1);
}


/* %%Function:GetWindowDCFP %%Owner:BRADV */
EXPORT GetWindowDCFP(P1)
HWND P1;
{
	return FFailWinOp() ? NULL : GetWindowDC(P1);
}


/* %%Function:GlobalAddAtomFP %%Owner:BRADV */
EXPORT GlobalAddAtomFP(P1)
LPSTR P1;
{
	return FFailWinOp() ? NULL : GlobalAddAtom(P1);
}


/* %%Function:GlobalAllocFP %%Owner:BRADV */
EXPORT GlobalAllocFP(P1,P2)
WORD P1; 
DWORD P2;
{
	return FFailWinOp() ? NULL : GlobalAlloc(P1,P2);
}


/* %%Function:GlobalLockClipFP %%Owner:BRADV */
EXPORT LPSTR GlobalLockClipFP(P1)
HANDLE P1;
{
	return FFailWinOp() ? NULL : GlobalLock(P1);
}


/* %%Function:GlobalReAllocFP %%Owner:BRADV */
EXPORT GlobalReAllocFP(P1,P2,P3)
HANDLE P1; 
DWORD P2; 
WORD P3;
{
	return FFailWinOp() ? NULL : GlobalReAlloc(P1,P2,P3);
}


/* %%Function:IntersectClipRectFP %%Owner:BRADV */
EXPORT IntersectClipRectFP(P1,P2,P3,P4,P5)
HDC P1; 
short P2; 
short P3; 
short P4; 
short P5;
{
	return FFailWinOp() ? ERROR : IntersectClipRect(P1,P2,P3,P4,P5);
}


/* %%Function:LoadBitmapFP %%Owner:BRADV */
EXPORT LoadBitmapFP(P1,P2)
HANDLE P1; 
LPSTR P2;
{
	return FFailWinOp() ? NULL : LoadBitmap(P1,P2);
}


/* %%Function:LoadCursorFP %%Owner:BRADV */
EXPORT LoadCursorFP(P1,P2)
HANDLE P1; 
LPSTR P2;
{
	return FFailWinOp() ? NULL : LoadCursor(P1,P2);
}


/* %%Function:LoadLibraryFP %%Owner:BRADV */
EXPORT LoadLibraryFP(P1)
LPSTR P1;
{
	return FFailWinOp() ? NULL : LoadLibrary(P1);
}


/* %%Function:LoadMenuFP %%Owner:BRADV */
EXPORT LoadMenuFP(P1,P2)
HANDLE P1; 
LPSTR P2;
{
	return FFailWinOp() ? NULL : LoadMenu(P1,P2);
}


/* %%Function:MakeProcInstanceFP %%Owner:BRADV */
EXPORT FARPROC MakeProcInstanceFP(P1,P2)
FARPROC P1; 
HANDLE P2;
{
	return FFailWinOp() ? NULL : MakeProcInstance(P1,P2);
}


/* %%Function:MessageBoxFP %%Owner:BRADV */
EXPORT MessageBoxFP(P1,P2,P3,P4)
HWND P1; 
LPSTR P2; 
LPSTR P3; 
WORD P4;
{
	return !(P4 & MB_SYSTEMMODAL) && FFailWinOp() ? NULL :
			MessageBox(P1,P2,P3,P4);
}


/* %%Function:OpenClipboardFP %%Owner:BRADV */
EXPORT OpenClipboardFP(P1)
HWND P1;
{
	return FFailWinOp() ? NULL : OpenClipboard(P1);
}


/* %%Function:PatBltFP %%Owner:BRADV */
EXPORT PatBltFP(P1,P2,P3,P4,P5,P6)
HDC P1; 
short P2; 
short P3; 
short P4; 
short P5; 
DWORD P6;
{
	return vdbs.fTraceGDI ? PatBltTR(P1,P2,P3,P4,P5,P6) :
			PatBlt(P1,P2,P3,P4,P5,P6);
}


/* %%Function:PostMsgFP %%Owner:BRADV */
EXPORT PostMsgFP(P1,P2,P3,P4)
HWND P1; 
unsigned P2; 
WORD P3; 
LONG P4;
{
	return PostMessage(P1,P2,P3,P4);
}


/* %%Function:RegisterClassFP %%Owner:BRADV */
EXPORT RegisterClassFP(P1)
LPWNDCLASS P1;
{
	return FFailWinOp() ? NULL : RegisterClass(P1);
}


/* %%Function:ReleaseDCFP %%Owner:BRADV */
EXPORT ReleaseDCFP(P1,P2)
HWND P1; 
HDC P2;
{
	return vdbs.fTraceGDI ? ReleaseDCTR(P1,P2) : ReleaseDC(P1,P2);
}


/* %%Function:RestoreDCFP %%Owner:BRADV */
EXPORT RestoreDCFP(P1,P2)
HDC P1; 
short P2;
{
	return vdbs.fTraceGDI ? RestoreDCTR(P1,P2): RestoreDC(P1,P2);
}


/* %%Function:SaveDCFP %%Owner:BRADV */
EXPORT SaveDCFP(P1)
HDC P1;
{
	return FFailWinOp() ? NULL : SaveDC(P1);
}


/* %%Function:ScrollDCFP %%Owner:BRADV */
EXPORT ScrollDCFP(P1,P2,P3,P4,P5,P6,P7)
HDC P1; 
int P2; 
int P3; 
LPRECT P4; 
LPRECT P5; 
HRGN P6; 
LPRECT P7;
{
	return FFailWinOp() ? NULL : vdbs.fTraceGDI ? ScrollDCTR(P1,P2,P3,P4,P5,P6,P7)
			: ScrollDC(P1,P2,P3,P4,P5,P6,P7);
}


/* %%Function:FIsStockObj %%Owner:BRADV */
BOOL FIsStockObj(h)
HANDLE h;
{
	extern struct BMI vbmiEmpty;
	static HANDLE rghStock[15]; /* WHITE_BRUSH through DEVICEDEFAULT_FONT */
	int ih;

	if (rghStock[0]==0)
		for (ih=0; ih<15; ih++)
			rghStock[ih] = GetStockObject(ih);

	if (h == vbmiEmpty.hbm)
		return fTrue;

	for (ih=0; ih<15; ih++)
		if (h == rghStock[ih])
			return fTrue;

	return fFalse;
}


/* %%Function:SelectObjectFP %%Owner:BRADV */
EXPORT SelectObjectFP(P1,P2)
HDC P1; 
HANDLE P2;
{
	return !FIsStockObj(P2) && FFailWinOp() ? NULL : 
			vdbs.fTraceGDI ? SelectObjectTR(P1,P2) : SelectObject(P1,P2);
}


/* %%Function:SetMenuFP %%Owner:BRADV */
EXPORT SetMenuFP(P1,P2)
HWND P1; 
HMENU P2;
{
	return FFailWinOp() ? NULL : SetMenu(P1,P2);
}


/* %%Function:StretchBltFP %%Owner:BRADV */
EXPORT StretchBltFP(P1,P2,P3,P4,P5,P6,P7,P8,P9,P10,P11)
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
	return vdbs.fTraceGDI ? StretchBltTR(P1,P2,P3,P4,P5,P6,P7,P8,P9,P10,P11) :
			StretchBlt(P1,P2,P3,P4,P5,P6,P7,P8,P9,P10,P11);
}


/* %%Function:UnhookWindowsHookFP %%Owner:BRADV */
EXPORT UnhookWindowsHookFP(P1,P2)
int P1; 
FARPROC P2;
{
	return FFailWinOp() ? NULL : UnhookWindowsHook(P1,P2);
}


